#include <Bluepad32.h>
#include <Servo.h>
#include <HardwareSerial.h>
#include "CRC8.h"

#define CONFIG_WEAPON_TYPE BRUSHLESS  //only BRUSHLESS or NONE supported
#define CONFIG_DRIVE_TYPE IMPULSE     //only IMPULSE supported
#define CONFIG_ACC_TYPE NONE          //not supported yet
#define CONFIG_I2C_TYPE NONE          //not supported yet
#define USER_WEAPON_RATE_LIMIT 5        //number of PPM the servo signal can change per ms. 1 = full speed to stop in 1 second.
#define USER_INVERT_MOTOR_1 false
#define USER_INVERT_MOTOR_2 false
#define USER_BRAKE_TIMEOUT 100
#define USER_DEBUG_UPDATE_INTERVAL 1000

#define USER_VERBOSE_LOGGING true

#define USER_IMPULSE_CURVE_LEVEL 100       //0 for off, 1-3 for lo-hi
#define USER_IMPULSE_BRAKE true     //0 for no brake, 1-3 for lo-hi
#define USER_IMPULSE_BRAKE_ON_STOP true     //0 for no brake, 1-3 for lo-hi
#define USER_IMPULSE_RESET_ON_BOOT true  //reset the impulse driver on every reciever bootup

#define USER_MOTOR_1_MAXIMUM_SPEED 255
#define USER_MOTOR_2_MAXIMUM_SPEED 255
#define USER_WEAPON_MAXIMUM_SPEED 2000

#define PIN_SNAP_1_RX 9
#define PIN_SNAP_1_TX 10
#define PIN_SNAP_2_RX 6
#define PIN_SNAP_2_TX 7
#define PIN_ESC 2
/*
#define INTERNAL_DEVICE_ADDRESS FIREFLY_DEVICE_ADDRESS
#define INTERNAL_DEVICE_EXPECTED_COMMAND_MAXIMUM FIREFLY_EXPECTED_COMMAND_MAXIMUM
*/
