#include "config.h"

//user vars
bool brake = CST_DFT_BRAKE_ENABLED;               //set this to false to disable braking after 100ms
bool brake_on_failsafe = CST_DFT_BRAKE_FAILSAFE;  //set this to false to disable braking on failsafe

//incoming data var
byte data;
int mapped_data;

//settings vars
int curve_factor = CST_DFT_CURVE_FACTOR;
int brake_timeout = CST_DFT_BRAKE_TIMEOUT;

//timer vars
unsigned long timer;
unsigned long last_rcv;
unsigned long last_motor_1_active;
unsigned long last_motor_2_active;

//activity vars
bool motor_1_active = false;
bool motor_2_active = false;

//motor data vars
int motor_1_pin_a_data = 0;
int motor_1_pin_b_data = 0;
int motor_2_pin_a_data = 0;
int motor_2_pin_b_data = 0;

//sensor vars
int battery_sense;

//settings enum
enum setting_entry {
  UNDEF,
  CURVE_SET,
  BRAKE_SET,
};

setting_entry setting = UNDEF;

void setup() {
  Serial.begin(9600);  //start serial comm
  Serial.write(CMD_DEBUG_SEND_HELLO_WORLD);
  Serial.write(CMD_DEBUG_SEND_INIT_COMPLETE);
}

void loop() {
  timer = millis();                            //enable timer
  mapped_data = map(data, 0, 200, -100, 100);  //calculate mapped data



  battery_sense = analogRead(PIN_BATTERY_SENSE);  //read battery voltage



  //handle motor 1 writing
  if (data < 201 && motor_1_active && !motor_2_active) {
    if (mapped_data > 5) {                                                                                  //fwd | the 5/-5 is a deadzone
      last_motor_1_active = timer;                                                                          //update activity timer
      motor_1_pin_a_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);  //we are mapping the throttle alone the equation |x(x/100)^n| where x is the mapped data and n is the curve factor
      motor_1_pin_b_data = 0;
    } else if (mapped_data < -5) {  //back
      last_motor_1_active = timer;
      motor_1_pin_a_data = 0;
      motor_1_pin_b_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
    } else if (!brake || (timer - motor_1_active < brake_timeout)) {  //we can't brake immediately, that would cause voltage spikes
      motor_1_pin_a_data = 0;
      motor_1_pin_b_data = 0;
    } else {
      motor_1_pin_a_data = 255;
      motor_1_pin_b_data = 255;
    }
  }
  //all that again but for the other motor
  if (data < 201 && motor_2_active && !motor_1_active) {
    if (mapped_data > 5) {
      last_motor_2_active = timer;
      motor_2_pin_a_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
      motor_2_pin_b_data = 0;
    } else if (mapped_data < -5) {
      last_motor_2_active = timer;
      motor_2_pin_a_data = 0;
      motor_2_pin_b_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
    } else if (!brake || (timer - motor_2_active < brake_timeout)) {
      motor_2_pin_a_data = 0;
      motor_2_pin_b_data = 0;
    } else {
      motor_2_pin_a_data = 255;
      motor_2_pin_b_data = 255;
    }
  }


  //handle settings
  switch (setting) {

    case CURVE_SET:
      if (data < 201) {
        curve_factor = data;
        Serial.write(CMD_DEBUG_SEND_CURVE_SET_APPLIED);
        Serial.write(data);
        setting = UNDEF;
      }
      break;

    case BRAKE_SET:
      if (data < 201) {
        brake_timeout = data;
        Serial.write(CMD_DEBUG_SEND_BRAKE_SET_APPLIED);
        Serial.write(data);
        setting = UNDEF;
      }
      break;
  }



  if (Serial.available()) {
    last_rcv = timer;  //refresh timer
    data = Serial.read();



    switch (data) {
      case CMD_WRITE_MOTOR_1:
        setting = UNDEF;  //disable settings mode to prevent conflicts
        motor_1_active = true;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_WRITING_MOTOR_1);
        break;  //only 1 motor or the settings mode can be enabled at any given time or there will be conflicts

      case CMD_WRITE_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = true;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_WRITING_MOTOR_2);
        break;

      case CMD_BRAKE_MOTOR_1:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        //direct command - write to motors directly
        motor_1_pin_a_data = 255;
        motor_1_pin_b_data = 255;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_BRAKING_MOTOR_1);
        break;

      case CMD_BRAKE_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        motor_2_pin_a_data = 255;
        motor_2_pin_b_data = 255;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_BRAKING_MOTOR_2);
        break;

      case CMD_COAST_MOTOR_1:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        motor_1_pin_a_data = 0;
        motor_1_pin_b_data = 0;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_COASTING_MOTOR_1);
        break;

      case CMD_COAST_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        motor_2_pin_a_data = 0;
        motor_2_pin_b_data = 0;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_COASTING_MOTOR_2);
        break;

      case CMD_CURVE_SET_INTENSITY:
        setting = CURVE_SET;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_CURVE_SET);
        break;

      case CMD_CURVE_OFF:
        curve_factor = 0;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_CURVE_OFF);
        break;

      case CMD_BRAKE_ON:
        brake = true;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_BRAKE_ON);
        break;

      case CMD_BRAKE_OFF:
        brake = false;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_BRAKE_OFF);
        break;

      case CMD_BRAKE_ON_FAILSAFE:
        brake_on_failsafe = true;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_FS_BRAKE_ON);
        break;

      case CMD_COAST_ON_FAILSAFE:
        brake_on_failsafe = false;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_FS_BRAKE_OFF);
        break;

      case CMD_BRAKE_SET_TIMEOUT:
        setting = BRAKE_SET;
        motor_1_active = false;
        motor_2_active = false;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(CMD_DEBUG_SEND_BRAKE_TIMEOUT_SET);
        break;

      case CMD_FAILSAFE:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        motor_1_pin_a_data = 255 * brake_on_failsafe;  //if brake_on_failsafe is 0, this will evaluate to 0, if it's 1 it will evaluate to 255
        motor_1_pin_b_data = 255 * brake_on_failsafe;
        motor_2_pin_a_data = 255 * brake_on_failsafe;
        motor_2_pin_b_data = 255 * brake_on_failsafe;

        Serial.write(CMD_SEND_DEBUG_MESSAGE);
        Serial.write(99);
        break;
    }
  }



  if (timer - last_rcv > 100) {
    motor_1_pin_a_data = 255 * brake_on_failsafe;  //if brake_on_failsafe is 0, this will evaluate to 0, if it's 1 it will evaluate to 255
    motor_1_pin_b_data = 255 * brake_on_failsafe;
    motor_2_pin_a_data = 255 * brake_on_failsafe;
    motor_2_pin_b_data = 255 * brake_on_failsafe;
  } else {
    analogWrite(PIN_MOTOR_1_A, motor_1_pin_a_data);
    analogWrite(PIN_MOTOR_1_B, motor_1_pin_b_data);
    analogWrite(PIN_MOTOR_2_A, motor_2_pin_a_data);
    analogWrite(PIN_MOTOR_2_B, motor_2_pin_b_data);
  }
}