//motor controller pins
#define PIN_CHANNEL_1_A PIN_PA4
#define PIN_CHANNEL_1_B PIN_PA5
#define PIN_CHANNEL_2_A PIN_PB1
#define PIN_CHANNEL_2_B PIN_PB0



//user settings
#define USR_MOTOR_1_MAXIMUM_SPEED 255
#define USR_MOTOR_2_MAXIMUM_SPEED 255

//serial safety timeout
#define USR_SERIAL_SAFETY_TIMEOUT 10000



//do not touch
#define INTERNAL_DEVICE_ADDRESS IMPULSE_DEVICE_ADDRESS
#define INTERNAL_DEVICE_EXPECTED_COMMAND_MAXIMUM IMPULSE_EXPECTED_COMMAND_MAXIMUM



struct MotorControls {
  int payload;
  int curve_exponent;
  int maximum_speed;
};



//impulse-specific PWM calculation code
int calculatePWM(MotorControls _MotorControl) {
  float _normalized = map(_MotorControl.payload, 0, 255, 0, 255) / 255.0;
  float _curve_value = _MotorControl.payload * pow(_normalized, _MotorControl.curve_exponent);
  return map(abs(_curve_value), 0, 255, 0, _MotorControl.maximum_speed);
}

#include "bfr_provisions.h"