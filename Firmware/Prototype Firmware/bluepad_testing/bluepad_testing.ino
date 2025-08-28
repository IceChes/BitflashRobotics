#include <Bluepad32.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define WEAPON_PIN 10
//#define SS_RX_1_PIN 0
//#define SS_TX_1_PIN 1
//#define SS_RX_2_PIN 2
//#define SS_TX_2_PIN 3

#define LED_1_PIN 9
#define LED_2_PIN 8

//Motor driver settings
#define MOTOR_1_MINIMUM_VALUE 0
#define MOTOR_1_MAXIMUM_VALUE 200
#define MOTOR_2_MINIMUM_VALUE 0
#define MOTOR_2_MAXIMUM_VALUE 200

#define MD_WRITE_MOTOR_1 0xC9  //writing motors: write motor command followed by data value between 0 and 200
#define MD_WRITE_MOTOR_2 0xCA
#define MD_BRAKE_MOTOR_1 0xCB  //stops motor
#define MD_BRAKE_MOTOR_2 0xCC
#define MD_COAST_MOTOR_1 0xCD  //disables output
#define MD_COAST_MOTOR_2 0xCE
#define MD_CURVE_SET_INTENSITY 0xCF  //curve set: curve set command followed by exponent value
#define MD_CURVE_OFF 0xD0            //disable curving
#define MD_BRAKE_ON 0xD1             //enables braking
#define MD_BRAKE_OFF 0xD2            //disables braking
#define MD_BRAKE_ON_FAILSAFE 0xD3    //brakes on failsafe
#define MD_COAST_ON_FAILSAFE 0xD4    //coasts on failsafe
#define MD_FAILSAFE 0xFF             //stops or brakes both motors depending on brake_on_failsafe
#define MD_CURVE 0                   //set to 0 to disable
#define MD_AUTO_BRAKE true
#define MD_FAILSAFE_BRAKE false

#define INVERT_MOTOR_1 false
#define INVERT_MOTOR_2 true

#define STOP_TIMEOUT 1000

byte curve = MD_CURVE;

Servo esc;
EspSoftwareSerial::UART snapSerial1;  //motor driver
EspSoftwareSerial::UART snapSerial2;  //other peripheral

//timer vars
unsigned long timer;
unsigned long last_button_press;
unsigned long last_esc_update_time;
unsigned long last_debug_message;

//control vars
int esc_value = 0;
int target_esc_value;
int motor_1_value;
int motor_2_value;
bool inverted = true;
bool weapon_locked = false;
bool data_updated;


//controller vars
int ls;   //left stick
int rs;   //right stick
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
  snapSerial1.begin(9600, SWSERIAL_8N1, 7, 8);
  snapSerial2.begin(9600, SWSERIAL_8N1, 9, 10);
  Serial.begin(115200);

  //Debug LED setup
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(WEAPON_PIN, OUTPUT);

  //Motor driver setup
  if (curve > 0) {
    snapSerial1.write(MD_CURVE_SET_INTENSITY);  //Set curve...
    snapSerial1.write(curve);                   //Set it to whatever the value is
  } else {
    snapSerial1.write(MD_CURVE_OFF);  //Disable curve
  }

  //Auto braking config
  if (MD_AUTO_BRAKE) {
    snapSerial1.write(MD_BRAKE_ON);
  } else {
    snapSerial1.write(MD_BRAKE_OFF);
  }

  //Failsafe brake config
  if (MD_FAILSAFE_BRAKE) {
    snapSerial1.write(MD_BRAKE_ON_FAILSAFE);
  } else {
    snapSerial1.write(MD_COAST_ON_FAILSAFE);
  }

  //Bluepad controller connection info.
  Serial.printf("BLUEPAD: Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BLUEPAD: BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  esc.attach(D0);
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
    motor_1_value = map(ls, -512, 512, 200, 0);
  } else {
    motor_1_value = map(rs, -512, 512, 0, 200);
  }

  if (INVERT_MOTOR_2) {
    motor_2_value = map(rs, -512, 512, 200, 0);
  } else {
    motor_2_value = map(ls, -512, 512, 0, 200);
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
    target_esc_value = map(lt, 0, 512, 0, 180);
  }

  if (!inverted && dpad_state == 2) {  //If down is pressed, invert the bot
    inverted = true;
    rumble(200, 0, 255, myControllers[0]);
    last_button_press = timer;
  } else if (inverted && dpad_state == 1) {  //If up is pressed, revert the bot
    inverted = false;
    rumble(200, 255, 0, myControllers[0]);
    last_button_press = timer;
  }

  if (inverted) {
    snapSerial1.write(MD_WRITE_MOTOR_1);
    snapSerial1.write(map(motor_2_value, 0, 200, 200, 0));
    snapSerial1.write(MD_WRITE_MOTOR_2);
    snapSerial1.write(map(motor_1_value, 0, 200, 200, 0));
  } else {
    snapSerial1.write(MD_WRITE_MOTOR_1);
    snapSerial1.write(motor_1_value);
    snapSerial1.write(MD_WRITE_MOTOR_2);
    snapSerial1.write(motor_2_value);
  }
  
  esc.write(D2, esc_value);

  //If it's been 10ms since the last debug message, print a new one.
  if (timer - last_debug_message > 10) {
    Serial.println("INFO: CONTROLLER DUMP: DP: " + String(dpad_state) + ", BT: " + String(buttons_state) + ", LS: " + String(ls) + ", RS: " + String(rs) + ", LT: " + String(lt) + ", RT: " + String(rt));
    Serial.println("INFO: VARIABLE DUMP  : IV: " + String(inverted) + ", WP: " + String(esc_value));
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

void rumble(int _dur, int _pwr_weak, int _pwr_strng, ControllerPtr ctl){
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
