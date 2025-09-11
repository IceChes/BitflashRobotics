#include <Bluepad32.h>
#include <Servo.h>
#include <SoftwareSerial.h>

<<<<<<< Updated upstream
//Motor settings
#define MOTORS_MINIMUM_VALUE -100  //0-100
#define MOTORS_MAXIMUM_VALUE 100
#define INVERT_MOTOR_1 false  //whether the motors are wired backwards
#define INVERT_MOTOR_2 true

#define DRIVE_ESC SERVO          //SERVO or SNAP
#define HAS_WEAPON true          //does the robot have a weapon
#define WEAPON_PROFILE VERTICAL  //VERTICAL or HORIZONTAL. ignored if HAS_WEAPON is false

//peripheral boards like sensors
#define PERIPHERAL_TYPE_I2C NO_I2C    //not supported right now
#define PERIPHERAL_TYPE_SNAP NO_SNAP  //not supported right now
#define COMPENSATE false              //not supported right now

#define VERBOSE_LOGGING true  //whether to send INFO messages to serial

#define SNAP_CFG_AUTO_BRAKE false
#define SNAP_CFG_FAIL_BRAKE false
#define SNAP_CFG_ENABLE_CURVE false
#define SNAP_CFG_CURVE_VALUE 1
  == == == =
>>>>>>> Stashed changes

//hard defines, do NOT touch
#define MOTOR_1_PIN D2
#define MOTOR_2_PIN D1
#define WEAPON_PIN D3

#define LED_1_PIN D9
#define LED_2_PIN D8

#define SS_RX_1_PIN 0
#define SS_TX_1_PIN 1
#define SS_RX_2_PIN 2
#define SS_TX_2_PIN 3

#define SNAP_CMD_WRITE_MOTOR_1 0xC9  //writing motors: write motor command followed by data value between 0 and 200
#define SNAP_CMD_WRITE_MOTOR_2 0xCA
#define SNAP_CMD_CURVE_SET_INTENSITY 0xCF  //curve set: curve set command followed by exponent value
#define SNAP_CMD_CURVE_OFF 0xD0            //disable curving
#define SNAP_CMD_BRAKE_ON 0xD1             //enables braking
#define SNAP_CMD_BRAKE_OFF 0xD2            //disables braking
#define SNAP_CMD_BRAKE_ON_FAILSAFE 0xD3    //brakes on failsafe
#define SNAP_CMD_COAST_ON_FAILSAFE 0xD4    //coasts on failsafe
#define SNAP_CMD_FAILSAFE 0xFF             //stops or brakes both motors depending on brake_on_failsafe



  //MAC address of your controller
  uint8_t controller_mac[6] = { 0xF2, 0x8D, 0x95, 0xD5, 0x01, 0xE6 };

EspSoftwareSerial::UART snapSerial1;  //motor driver ONLY
EspSoftwareSerial::UART snapSerial2;  //other peripheral ONLY

Servo motor_control;  //servo object. it's easiest to define this regardless of if it is used or not

//controller vars
int ls;       //left stick
int rs;       //right stick
int used_ls;  //the stick value the code will actually use
int used_rs;
bool lb;  //left bumper
bool rb;  //right bumper
int lt;   //left trigger
int rt;   //right trigger
bool a;
bool b;
bool x;
bool y;
int dpad_state;  //1=up, 2=down, 4=right, 8=left
int buttons_state;
bool super;  //xbox/stadia button

//timer vars
unsigned long timer;
unsigned long last_button_press;
unsigned long last_esc_update_time;
unsigned long last_debug_message;

//control vars
bool has_weapon = HAS_WEAPON;
int esc_value = 1000;
int target_esc_value;
int motor_1_value;
int motor_2_value;
bool inverted = false;
bool weapon_locked = false;
bool data_updated;
int motors_minimum_value;
int motors_maximum_value;
int battery_level;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];





//ENUMS FOR CONTROL SYSTEMS
//esc types
enum esc {
  SNAP,  //snapserial
  SERVO  //servo desc
};
esc current_esc = DRIVE_ESC;

//weapon profiles
enum weapon_profile {
  VERTICAL,
  HORIZONTAL
};
weapon_profile current_weapon_profile = WEAPON_PROFILE;

//i2c peripherals
enum i2c_peripheral {
  NO_I2C
};
i2c_peripheral current_i2c_peripheral = PERIPHERAL_TYPE_I2C;

//snap peripherals
enum snap_peripheral {
  NO_SNAP
};
snap_peripheral current_snap_peripheral = PERIPHERAL_TYPE_SNAP;





void setup() {
  //serial bus setup
  snapSerial1.begin(9600, SWSERIAL_8N1, SS_RX_1_PIN, SS_TX_1_PIN);  //snapserial 1 is always the motor driver
  snapSerial2.begin(9600, SWSERIAL_8N1, SS_RX_2_PIN, SS_RX_2_PIN);  //snapserial 2 is the snap peripheral, if there is any
  Serial.begin(115200);

  //activate bluepad
  BP32.setup(&onConnectedController, &onDisconnectedController);



  //setup needed for the drive ESC
  switch (current_esc) {
    case SNAP:

      motor_1_minimum_value = map(MOTOR_1_MINIMUM_VALUE, -100, 100, 0, 200);
      motor_1_maximum_value = map(MOTOR_1_MAXIMUM_VALUE, -100, 100, 0, 200);
      motor_2_minimum_value = map(MOTOR_2_MINIMUM_VALUE, -100, 100, 0, 200);
      motor_2_maximum_value = map(MOTOR_1_MAXIMUM_VALUE, -100, 100, 0, 200);


      //auto brake
      if (SNAP_CFG_AUTO_BRAKE) {
        snapSerial1.write(SNAP_CMD_BRAKE_ON);
      } else {
        snapSerial1.write(SNAP_CMD_BRAKE_OFF);
      }

      //fail brake
      if (SNAP_CFG_FAIL_BRAKE) {
        snapSerial1.write(SNAP_CMD_BRAKE_ON_FAILSAFE);
      } else {
        snapSerial1.write(SNAP_CMD_COAST_ON_FAILSAFE);
      }

      //curve
      if (SNAP_CFG_ENABLE_CURVE) {
        snapSerial1.write(SNAP_CMD_CURVE_SET_INTENSITY);
        snapSerial1.write(SNAP_CFG_CURVE_VALUE);
      } else {
        snapSerial1.write(SNAP_CMD_CURVE_OFF);
      }
      break;

    case SERVO:
      motor_1_minimum_value = map(MOTOR_1_MINIMUM_VALUE, -100, 100, 1000, 2000);
      motor_1_maximum_value = map(MOTOR_1_MAXIMUM_VALUE, -100, 100, 1000, 2000);
      motor_2_minimum_value = map(MOTOR_2_MINIMUM_VALUE, -100, 100, 1000, 2000);
      motor_2_maximum_value = map(MOTOR_1_MAXIMUM_VALUE, -100, 100, 1000, 2000);
      break;
  }



  //setup needed for the weapon
  if (has_weapon) {
    switch (current_weapon_profile) {
      case HORIZONTAL:
        break;

      case VERTICAL:
        break;
    }
  }



  //setup needed for the i2c peripheral
  switch (current_i2c_peripheral) {
    case NO_I2C:
      break;
  }



  //setup needed for the snap peripheral
  switch (current_snap_peripheral) {
    case NO_SNAP:
      break;
  }

  //LED 1 is an OK light, it's on as long as everything is OK
  digitalWrite(LED_1_PIN, 1);
}





void loop() {

  timer = millis();





  //process the controller data if there is new controller data.
  data_updated = BP32.update();
  if (data_updated) {
    processControllers();
  }





  if (INVERT_MOTOR_1) {
    used_ls = map(ls, 512, -512, -512, 512);
  } else {
    used_ls = ls;
  }
  if (INVERT_MOTOR_2) {
    used_rs = map(rs, 512, -512, -512, 512);
  } else {
    used_rs = rs;
  }





  //inversion handling. this logic will work regardless of what esc is selected
  if (inverted) {
    motor_1_value = map(used_rs, -512, 512, motor_1_maximum_value, motor_1_minimum_value);
    motor_2_value = map(used_ls, -512, 512, motor_2_maximum_value, motor_2_minimum_value);
    if (dpad_state == 1) {
      inverted = false;
      rumble(200, 255, 0, myControllers[0]);
      last_button_press = timer;
      if (VERBOSE_LOGGING) {
        Serial.println("INFO: Now inverted!");
      }
    }
  } else if (!inverted) {
    motor_1_value = map(used_ls, -512, 512, motor_1_minimum_value, motor_1_maximum_value);
    motor_2_value = map(used_rs, -512, 512, motor_2_minimum_value, motor_2_maximum_value);
    if (dpad_state == 2) {
      inverted = true;
      rumble(200, 0, 255, myControllers[0]);
      last_button_press = timer;
      if (VERBOSE_LOGGING) {
        Serial.println("INFO: Now reverted!");
      }
    }
  }




  //esc handler switch
  switch (current_esc) {
    case SNAP:

      //read battery level
      if (snapSerial1.available()) {
        battery_level = snapSerial1.read();
      }

      //write motor stuff
      snapSerial1.write(SNAP_CMD_WRITE_MOTOR_1);
      snapSerial1.write(motor_1_value);
      snapSerial1.write(SNAP_CMD_WRITE_MOTOR_2);
      snapSerial1.write(motor_2_value);

      break;

    case SERVO:
      motor_control.write(MOTOR_1_PIN, motor_1_value);
      motor_control.write(MOTOR_2_PIN, motor_2_value);

      break;
  }





  //handler for weapon stuff
  if (has_weapon) {
    //Toggle the throttle lock if A is pressed.
    if (rb && (timer - last_button_press > 200)) {
      weapon_locked = !weapon_locked;
      rumble(200, 255, 0, myControllers[0]);  //buzz the controller to let the user know the lock is tripped
      last_button_press = timer;
      if (VERBOSE_LOGGING) {
        Serial.println("INFO: Throttle lock tripped.");
      }
    }

    //If the throttle is locked, just make the target ESC value the current ESC value
    if (weapon_locked) {
      target_esc_value = esc_value;
    } else {  //Otherwise, map RT as usual.
      target_esc_value = map(rt, 0, 1020, 1000, 2000);
    }





    switch (current_weapon_profile) {
      case VERTICAL:
        //Rate limit the BLDC motor by about 1.5% every 1ms.
        if (target_esc_value > esc_value && (timer - last_esc_update_time > 1)) {
          esc_value += 3;
          last_esc_update_time = timer;
        } else if (target_esc_value < esc_value && (timer - last_esc_update_time > 1)) {
          esc_value -= 3;
          last_esc_update_time = timer;
        }
        motor_control.write(WEAPON_PIN, esc_value);
        break;

      case HORIZONTAL:
        //Rate limit the BLDC motor by about 1% every 3ms.
        if (target_esc_value > esc_value && (timer - last_esc_update_time > 3)) {
          esc_value += 2;
          last_esc_update_time = timer;
        } else if (target_esc_value < esc_value && (timer - last_esc_update_time > 3)) {
          esc_value -= 2;
          last_esc_update_time = timer;
        }
        motor_control.write(WEAPON_PIN, esc_value);
        break;
    }
  }





  //handler for i2c peripherals
  switch (current_i2c_peripheral) {
    case NO_I2C:
      break;
  }





  //handler for i2c peripherals
  switch (current_snap_peripheral) {
    case NO_SNAP:
      break;
  }



  if (VERBOSE_LOGGING && timer - last_debug_message > 100) {
    Serial.println("INFO: RAW CONTROLLER INFO DUMP: LS: " + String(used_ls) + " RS: " + String(used_rs));
    Serial.println("INFO: MODESET INFO DUMP       : DRIVE_ESC: " + String(current_esc) + " WEAPON_ENABLED: " + String(has_weapon) + " WEAPON_PROFILE: " + String(current_weapon_profile));
    Serial.println("INFO: CONTROL SYSTEM INFO DUMP: MOTOR_1: " + String(motor_1_value) + " MOTOR_2: " + String(motor_2_value) + " WEAPON: " + String(esc_value));
    last_debug_message = timer;
  }
}





//FUNCTION ZONE
void triggerFailsafe() {
  //Failsafe by setting all the controller variables to their rest states.
  //The rate limit code for the weapon will handle the throttling-down.
  Serial.println("ERROR: Controller disconnected! Failsafing...");
  ls = 0;
  rs = 0;
  lb = false;
  rb = false;
  lt = 0;
  rt = 0;
  a = false;
  b = false;
  x = false;
  y = false;
  buttons_state = 0;
  dpad_state = 0;
  super = false;

  //Reset misc variables...
  inverted = false;
  weapon_locked = false;
}

//Bluepad32 connection callback. I don't know how this works, but I am going to add some failsafe code in there.
void onConnectedController(ControllerPtr ctl) {
  Serial.println("BLUEPAD: Controller is connected!");
  myControllers[0] = ctl;
  ControllerProperties properties = ctl->getProperties();
  bool valid_controller;

  for (int m = 0; m < 6; m++) {
    if (controller_mac[m] != properties.btaddr[m]) {
      valid_controller = false;
    } else {
      valid_controller = true;
    }
  }
  if (valid_controller) {
    Serial.println("INFO: Controller validated and bound.");
    rumble(200, 255, 100, myControllers[0]);
  } else {
    ctl->disconnect();
    BP32.enableNewBluetoothConnections(true);
    Serial.println("ERROR: MAC section invalid. Rejecting pairing attempt.");
  }
}

//ctl->playDualRumble(0 /* delayedStartMs */, 500 /* durationMs */, 127 /* weakMagnitude */, 127 /* strongMagnitude */);

//Bluepad32 disconnect callback
void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;
  Serial.println("ERROR: Lost controller connection! Failsafing...");
  BP32.enableNewBluetoothConnections(true);
  triggerFailsafe();
}

void processGamepad(ControllerPtr ctl) {

  ls = ctl->axisY();
  rs = ctl->axisRY();
  lb = ctl->l1();
  rb = ctl->r1();
  lt = ctl->brake();
  rt = ctl->throttle();
  a = ctl->a();
  b = ctl->b();
  x = ctl->x();
  y = ctl->y();
  buttons_state = ctl->buttons();
  dpad_state = ctl->dpad();
  super = ctl->miscSystem();
  //This was in the example code. Saving it for later:
  //ctl->playDualRumble(0 /* delayedStartMs */, 2000 /* durationMs */, 255 /* weakMagnitude */, 255 /* strongMagnitude */);
}

void rumble(int _dur, int _pwr_weak, int _pwr_strng, ControllerPtr ctl) {
  ctl->playDualRumble(0, _dur, _pwr_weak, _pwr_strng);
}

//I have no idea what this does. This is a great library, if only they would document it.
void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      processGamepad(myController);
    }
  }
}
