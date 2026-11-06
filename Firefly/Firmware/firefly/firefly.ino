#include <bfr_provisions.h>
#include <Bluepad32.h>
#include <Servo.h>
#include <CRC8.h>
#include <HardwareSerial.h>

#include "config.h"
#include "functions.h"


//timer vars


//control vars




//data checksum item
CRC8 crc_1;
CRC8 crc_2;



HardwareSerial DriverSerial(1);     //motor drivers are to be connected to the driver port
HardwareSerial AccessorySerial(0);  //accessory modules are to be connected to the accessory port

Firefly firefly;

Servo weapon_control;



//MAC addresses
//uint8_t controller_mac[6] = { 0xF2, 0x8D, 0x95, 0xD5, 0x01, 0xE6 };  //stadia
//uint8_t controller_mac[6] = { 0xC8, 0x3F, 0x26, 0x8D, 0xE6, 0x28 }; //xbox



//data controls
DataStream driver(IMPULSE_DEVICE_ADDRESS, &DriverSerial);
DataStream accessory(IMPULSE_DEVICE_ADDRESS, &AccessorySerial);

uint8_t driver_return_code;
uint8_t accessory_return_code;



//input data structs
PacketData d_driver;
PacketData d_accessory;



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

  assign(FIREFLY_DEVICE_ADDRESS, FIREFLY_EXPECTED_COMMAND_MAXIMUM);

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
  DriverSerial.begin(115200, SERIAL_8N1, 9, 10);
  AccessorySerial.begin(115200, SERIAL_8N1, PIN_SNAP_2_RX, PIN_SNAP_2_TX);


  //impulse setup
  if (current_drive_mode == IMPULSE) {
    if (USER_IMPULSE_RESET_ON_BOOT) {
      driver.sendPacket(IMPULSE_COMMAND_RESET, 0);
    }

    driver.sendPacket(IMPULSE_COMMAND_SET_CURVE, USER_IMPULSE_CURVE_LEVEL);

    if (USER_IMPULSE_BRAKE) {
      driver.sendPacket(IMPULSE_COMMAND_SET_BRAKE, true);
      driver.sendPacket(IMPULSE_COMMAND_SET_BRAKE_TIMEOUT, USER_BRAKE_TIMEOUT);
    } else {
      driver.sendPacket(IMPULSE_COMMAND_SET_BRAKE, false);
    }

    if (USER_IMPULSE_BRAKE_ON_STOP) {
      driver.sendPacket(IMPULSE_COMMAND_SET_FAIL_BRAKE, true);
    } else {
      driver.sendPacket(IMPULSE_COMMAND_SET_FAIL_BRAKE, false);
    }
  }
}





void loop() {



  //update timer
  firefly.main_timer = millis();



  //update snap1 packet
  while (DriverSerial.available()) {
    if (driver.receiveAndAssign(&d_driver, &driver_return_code)) {
      if (driver_return_code == GLOBAL_PACKET_ERROR_NONE) {
        handleInboundData(d_driver);
      } else if (driver_return_code != GLOBAL_PACKET_ERROR_NONE && USER_VERBOSE_LOGGING) {
        Serial.println("WARN: got garbled packet from driver with error code " + String(driver_return_code));
      }
    }
  }

  while (AccessorySerial.available()) {
    if (accessory.receiveAndAssign(&d_accessory, &accessory_return_code)) {
      if (accessory_return_code == GLOBAL_PACKET_ERROR_NONE) {
        handleInboundData(d_accessory);
      } else if (accessory_return_code != GLOBAL_PACKET_ERROR_NONE && USER_VERBOSE_LOGGING) {
        Serial.println("WARN: got garbled packet from accessory with error code " + String(accessory_return_code));
      }
    }
  }



  //process the controller data if there is new controller data.
  stadia.data_updated = BP32.update();
  if (stadia.data_updated) {
    processControllers();
  }



  //Motor inverts
  //eventually want to optimize this to integrate with the invert buttn
  if (USER_INVERT_MOTOR_1) {
    stadia.inv_ls = map(stadia.ls, 509, -509, -509, 509);
  } else {
    stadia.inv_ls = stadia.ls;
  }

  if (USER_INVERT_MOTOR_2) {
    stadia.inv_rs = map(stadia.rs, 509, -509, -509, 509);
  } else {
    stadia.inv_rs = stadia.rs;
  }


  //Impulse driver handling
  if (current_drive_mode == IMPULSE) {
    if (stadia.used_ls > 100) {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_1_FORWARDS, map(stadia.used_ls, 100, 509, 0, USER_MOTOR_1_MAXIMUM_SPEED));
    } else if (stadia.used_ls < -100) {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_1_BACKWARDS, map(stadia.used_ls, -100, -509, 0, USER_MOTOR_1_MAXIMUM_SPEED));
    } else {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_1_STOP, 0);
    }

    if (stadia.used_rs > 100) {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_2_FORWARDS, map(stadia.used_rs, 100, 509, 0, USER_MOTOR_2_MAXIMUM_SPEED));
    } else if (stadia.used_rs < -100) {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_2_BACKWARDS, map(stadia.used_rs, -100, -509, 0, USER_MOTOR_2_MAXIMUM_SPEED));
    } else {
      driver.sendPacket(IMPULSE_COMMAND_CHANNEL_2_STOP, 0);
    }
  }


  //inversion handling. this logic will work regardless of what esc is selected
  if (firefly.inverted) {
    stadia.used_ls = stadia.inv_rs;
    stadia.used_rs = stadia.inv_ls;
    if (stadia.dpad_state == 1) {
      firefly.inverted = false;
      rumble(200, 255, 0, myControllers[0]);
      firefly.last_button_press = firefly.main_timer;
    }
  } else if (!firefly.inverted) {
    stadia.used_ls = map(stadia.inv_ls, -509, 509, 509, -509);
    stadia.used_rs = map(stadia.inv_rs, -509, 509, 509, -509);
    if (stadia.dpad_state == 2) {
      firefly.inverted = true;
      rumble(200, 0, 255, myControllers[0]);
      firefly.last_button_press = firefly.main_timer;
    }
  }



  if (current_weapon_mode == BRUSHLESS) {
    if (firefly.target_esc_value > firefly.esc_value && (firefly.main_timer - firefly.last_esc_update_time > 1)) {
      firefly.esc_value += USER_WEAPON_RATE_LIMIT;
      firefly.last_esc_update_time = firefly.main_timer;
    } else if (firefly.target_esc_value < firefly.esc_value && (firefly.main_timer - firefly.last_esc_update_time > 1)) {
      firefly.esc_value -= USER_WEAPON_RATE_LIMIT;
      firefly.last_esc_update_time = firefly.main_timer;
    }

    //Toggle the throttle lock if A is pressed.
    if (stadia.rb && (firefly.main_timer - firefly.last_button_press > 200)) {
      firefly.weapon_locked = !firefly.weapon_locked;
      rumble(200, 255, 0, myControllers[0]);
      firefly.last_button_press = firefly.main_timer;
    }

    //If the throttle is locked, just make the target ESC value the current ESC value
    if (firefly.weapon_locked) {
      firefly.target_esc_value = firefly.esc_value;
    } else {  //Otherwise, map RT as usual.
      firefly.target_esc_value = map(stadia.rt, 0, 1020, 1000, USER_WEAPON_MAXIMUM_SPEED);
    }

    weapon_control.write(PIN_ESC, firefly.esc_value);
  }



  if ((firefly.main_timer - firefly.last_debug_update > USER_DEBUG_UPDATE_INTERVAL) && USER_VERBOSE_LOGGING) {
    if (stadia.controller_connected) {
      Serial.println("INFO: Controller connected.");
      Serial.println("INFO: Inversion status: " + String(firefly.inverted));
      Serial.println("INFO: Weapon interlock status: " + String(firefly.weapon_locked));
    }
    else{
      Serial.println("WARN: Controller not connected!");
    }
    firefly.last_debug_update = firefly.main_timer;
  }
}