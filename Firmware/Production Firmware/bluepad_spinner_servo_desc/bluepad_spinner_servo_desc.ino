#include <Bluepad32.h>
#include <Servo.h>
#include <SoftwareSerial.h>

//Motor settings
#define MOTORS_MINIMUM_VALUE 1000
#define MOTORS_MAXIMUM_VALUE 2000

#define MOTOR_1_PIN D2
#define MOTOR_2_PIN D1
#define WEAPON_PIN D3

#define INVERT_MOTOR_1 false
#define INVERT_MOTOR_2 true

Servo motor_control;

//timer vars
unsigned long timer;
unsigned long last_button_press;
unsigned long last_esc_update_time;
unsigned long last_debug_message;

//control vars
int esc_value = 1000;
int target_esc_value;
int motor_1_value;
int motor_2_value;
bool inverted = true;
bool weapon_locked = false;
bool data_updated;


//controller vars
int ls;   //left stick
int rs;   //right stick
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

//controller pointer for bluepad
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_1_PIN, OUTPUT);
  pinMode(MOTOR_2_PIN, OUTPUT);
  pinMode(WEAPON_PIN, OUTPUT);

  //Bluepad controller connection info.
  Serial.printf("BLUEPAD: Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BLUEPAD: BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {
  timer = millis();

  //process the controller data if there is new controller data.
  data_updated = BP32.update();
  if (data_updated) {
    processControllers();
  }

  //Motor inverts
  if (INVERT_MOTOR_1) {
    used_ls = map(ls, -512, 512, 512, -512);
  }
  else{
    used_ls = ls;
  }

  if (INVERT_MOTOR_2) {
    used_rs = map(rs, -512, 512, 512, -512);
  }
  else{
    used_rs = rs;
  }

  //Rate limit the BLDC motor by about 0.5% every 3ms.
  if (target_esc_value > esc_value && (timer - last_esc_update_time > 1)) {
    esc_value += 3;
    last_esc_update_time = timer;
  } else if (target_esc_value < esc_value && (timer - last_esc_update_time > 1)) {
    esc_value -= 3;
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
    target_esc_value = map(rt, 0, 1020, 1000, 2000);
  }

  if (inverted) {
    motor_1_value = map(used_rs, -512, 512, MOTORS_MAXIMUM_VALUE, MOTORS_MINIMUM_VALUE);
    motor_2_value = map(used_ls, -512, 512, MOTORS_MAXIMUM_VALUE, MOTORS_MINIMUM_VALUE);
    if (dpad_state == 1) {
      inverted = false;
      rumble(200, 255, 0, myControllers[0]);
      last_button_press = timer;
    }
  } else if (!inverted) {
    motor_1_value = map(used_ls, -512, 512, MOTORS_MINIMUM_VALUE, MOTORS_MAXIMUM_VALUE);
    motor_2_value = map(used_rs, -512, 512, MOTORS_MINIMUM_VALUE, MOTORS_MAXIMUM_VALUE);
    if (dpad_state == 2) {
      inverted = true;
      rumble(200, 0, 255, myControllers[0]);
      last_button_press = timer;
    }
  }



  motor_control.write(MOTOR_1_PIN, motor_1_value);
  motor_control.write(MOTOR_2_PIN, motor_2_value);
  motor_control.write(WEAPON_PIN, esc_value);

  //If it's been 10ms since the last debug message, print a new one.
  if (timer - last_debug_message > 10) {
    Serial.println("INFO: CONTROLLER DUMP: DP: " + String(dpad_state) + ", BT: " + String(buttons_state) + ", LS: " + String(ls) + ", RS: " + String(rs) + ", LT: " + String(lt) + ", RT: " + String(rt));
    Serial.println("INFO: VARIABLE DUMP  : IV: " + String(inverted) + ", WP: " + String(esc_value) + ", LM: " + String(motor_1_value) + ", RM: " + String(motor_2_value));
  }
  /*Reminder to myself of what variables we have
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
  dpad_state = ctl->dpad();
  super = ctl->miscSystem();
*/
  delay(1);
}

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
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.println("BLUEPAD: Controller is connected!");
      myControllers[i] = ctl;
      foundEmptySlot = true;
      ctl->playDualRumble(0 /* delayedStartMs */, 500 /* durationMs */, 127 /* weakMagnitude */, 127 /* strongMagnitude */);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("BLUEPAD: Controller connected, but could not found empty slot");
  }
}

//Bluepad32 disconnect callback
void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.println("BLUEPAD: Lost controller connection");
      triggerFailsafe();
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("BLUEPAD: Controller disconnected, but not found in myControllers");
  }
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
