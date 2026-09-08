#include <Bluepad32.h>
#include <Servo.h>
#include "CRC8.h"
#include "bfr_provisions.h"
#include "config.h"
#include "functions.h"
#include "HardwareSerial.h"

config.address = 0;
config.command_maximum = 0;

//timer vars
unsigned long timer;
unsigned long last_motor_1_active;
unsigned long last_motor_2_active;
unsigned long last_button_press;
unsigned long last_esc_update_time;
unsigned long last_debug_update;



//control vars
int esc_value = 1000;
int target_esc_value;
int motor_1_value;
int motor_2_value;
bool motor_1_active;
bool motor_2_active;
bool inverted = false;
bool weapon_locked = false;



//data controls
int driver_return_code;
int accessory_return_code;
PacketControls pi_driver;
PacketControls pi_accessory;
PacketData d_driver;
PacketData d_accessory;

//data checksum item
CRC8 crc_1;
CRC8 crc_2;



HardwareSerial driver(0);     //motor drivers are to be connected to the driver port
HardwareSerial accessory(1);  //accessory modules are to be connected to the accessory port

Servo motor_control;



//MAC address of your controller
//uint8_t controller_mac[6] = { 0xF2, 0x8D, 0x95, 0xD5, 0x01, 0xE6 };  //stadia
//uint8_t controller_mac[6] = { 0xC8, 0x3F, 0x26, 0x8D, 0xE6, 0x28 }; //xbox










enum drive_mode {
  IMPULSE
};
drive_mode current_drive_mode = CONFIG_DRIVE_TYPE;

enum weapon_mode {
  NONE,
  BRUSHLESS
};
weapon_mode current_weapon_mode = CONFIG_WEAPON_TYPE;

void setup() {
  Serial.begin(115200);
  Serial.println("INFO: Serial initialized.");

  //serial setup
  digitalWrite(PIN_SNAP_1_RX, 0);
  digitalWrite(PIN_SNAP_1_TX, 0);
  digitalWrite(PIN_SNAP_2_RX, 0);
  digitalWrite(PIN_SNAP_2_TX, 0);
  Serial.begin(115200);
  Serial.println("BLUEPAD: Activating bluepad...");
  BP32.setup(&onConnectedController, &onDisconnectedController);
  Serial.println("BLUEPAD: Controller should be connected.");

  delay(3000);
  driver.begin(115200, SERIAL_8N1, PIN_SNAP_1_RX, PIN_SNAP_1_TX);
  accessory.begin(115200, SERIAL_8N1, PIN_SNAP_2_RX, PIN_SNAP_2_TX);


  //impulse setup
  if (current_drive_mode = IMPULSE) {
    if (USER_IMPULSE_RESET_ON_BOOT) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_RESET, 0, &driver);
    }

    sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_CURVE, USER_IMPULSE_CURVE_LEVEL, &driver);

    if (USER_IMPULSE_BRAKE) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_BRAKE, true, &driver);
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_BRAKE_TIMEOUT, USER_BRAKE_TIMEOUT, &driver);
    } else {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_BRAKE, false, &driver);
    }

    if (USER_IMPULSE_BRAKE_ON_STOP) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_FAIL_BRAKE, true, &driver);
    } else {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_SET_FAIL_BRAKE, false, &driver);
    }
  }
}





void loop() {



  //update timer
  timer = millis();



  //update snap1 packet
  while (driver.available()) {
    d_driver = recieveAndAssign(pi_driver, &driver, &driver_return_code);
    if (driver_return_code != GLOBAL_PACKET_ERROR_NONE && USER_VERBOSE_LOGGING) {
      Serial.println("WARN: got garbled packet from driver with error code " + String(driver_return_code));
    }
    handleInboundData(d_driver);
  }

  while (accessory.available()) {
    d_driver = recieveAndAssign(pi_accessory, &accessory, &accessory_return_code);
    if (driver_return_code != GLOBAL_PACKET_ERROR_NONE && USER_VERBOSE_LOGGING) { 
      Serial.println("WARN: got garbled packet from accessory with error code " + String(accessory_return_code));
    }
    handleInboundData(d_accessory);
  }



  //process the controller data if there is new controller data.
  data_updated = BP32.update();
  if (data_updated) {
    processControllers();
  }



  //Motor inverts
  if (USER_INVERT_MOTOR_1) {
    inv_ls = map(ls, 509, -509, -509, 509);
  } else {
    inv_ls = ls;
  }

  if (USER_INVERT_MOTOR_2) {
    inv_rs = map(rs, 509, -509, -509, 509);
  } else {
    inv_rs = rs;
  }


  //Impulse driver handling
  if (current_drive_mode == IMPULSE) {
    if (used_ls > 100) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_1_FORWARDS, map(used_ls, 100, 509, 0, USER_MOTOR_1_MAXIMUM_SPEED), &driver);
    } else if (used_ls < -100) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_1_BACKWARDS, map(used_ls, -100, -509, 0, USER_MOTOR_1_MAXIMUM_SPEED), &driver);
    } else {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_1_STOP, 0, &driver);
    }

    if (used_rs > 100) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_2_FORWARDS, map(used_rs, 100, 509, 0, USER_MOTOR_2_MAXIMUM_SPEED), &driver);
    } else if (used_rs < -100) {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_2_BACKWARDS, map(used_rs, -100, -509, 0, USER_MOTOR_2_MAXIMUM_SPEED), &driver);
    } else {
      sendPacket(IMPULSE_DEVICE_ADDRESS, IMPULSE_COMMAND_CHANNEL_2_STOP, 0, &driver);
    }
  }


  //inversion handling. this logic will work regardless of what esc is selected
  if (inverted) {
    used_ls = inv_rs;
    used_rs = inv_ls;
    if (dpad_state == 1) {
      inverted = false;
      rumble(200, 255, 0, myControllers[0]);
      last_button_press = timer;
    }
    Serial.println("Inverted! LS: " + String(used_ls) + "|| RS: " + String(used_rs));
  } else if (!inverted) {
    used_ls = map(inv_ls, -509, 509, 509, -509);
    used_rs = map(inv_rs, -509, 509, 509, -509);
    if (dpad_state == 2) {
      inverted = true;
      rumble(200, 0, 255, myControllers[0]);
      last_button_press = timer;
    }
    Serial.println("Normal! LS: " + String(used_ls) + " || RS: " + String(used_rs));
  }



  if (current_weapon_mode == BRUSHLESS) {
    if (target_esc_value > esc_value && (timer - last_esc_update_time > 1)) {
      esc_value += USER_WEAPON_RATE_LIMIT;
      last_esc_update_time = timer;
    } else if (target_esc_value < esc_value && (timer - last_esc_update_time > 1)) {
      esc_value -= USER_WEAPON_RATE_LIMIT;
      last_esc_update_time = timer;
    }

    //Toggle the throttle lock if A is pressed.
    if (rb && (timer - last_button_press > 200)) {
      weapon_locked = !weapon_locked;
      rumble(200, 255, 0, myControllers[0]);
      last_button_press = timer;
    }

    //If the throttle is locked, just make the target ESC value the current ESC value
    if (weapon_locked) {
      target_esc_value = esc_value;
    } else {  //Otherwise, map RT as usual.
      target_esc_value = map(rt, 0, 1020, 1000, USER_WEAPON_MAXIMUM_SPEED);
    }

    motor_control.write(PIN_ESC, esc_value);
  }



  if (timer - last_debug_update > USER_DEBUG_UPDATE_INTERVAL) {
    Serial.println("");
  }
}