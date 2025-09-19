//Do NOT touch ANYTHING unless you know EXACTLY what you are doing!
//Seriously, messing things up in here can lead to a runaway robot.

#define CST_DFT_CURVE_FACTOR 0       //default curve factor
#define CST_DFT_BRAKE_TIMEOUT 100    //default auto-brake timeout
#define CST_DFT_BRAKE_ENABLED true   //is brake enabled?
#define CST_DFT_BRAKE_FAILSAFE true  //is failsafe brake enabled?

#define PIN_MOTOR_1_A 0
#define PIN_MOTOR_1_B 1
#define PIN_MOTOR_2_A 10
#define PIN_MOTOR_2_B 11
#define PIN_BATTERY_SENSE 7

//recieve commands
#define CMD_WRITE_MOTOR_1 201  //writing motors: write motor command followed by data value between 0 and 200
#define CMD_WRITE_MOTOR_2 202
#define CMD_BRAKE_MOTOR_1 203  //stops motor
#define CMD_BRAKE_MOTOR_2 204
#define CMD_COAST_MOTOR_1 205  //disables output
#define CMD_COAST_MOTOR_2 206
#define CMD_CURVE_SET_INTENSITY 207  //curve set: curve set command followed by exponent value
#define CMD_CURVE_OFF 208            //disable curving
#define CMD_BRAKE_ON 209             //enables braking
#define CMD_BRAKE_OFF 210            //disables braking
#define CMD_BRAKE_ON_FAILSAFE 211    //brakes on failsafe
#define CMD_COAST_ON_FAILSAFE 212    //coasts on failsafe
#define CMD_BRAKE_SET_TIMEOUT 213    //set brake timeout
#define CMD_FAILSAFE 214             //stops or brakes both motors depending on brake_on_failsafe



//Send codes section
//Fault codes/other
#define CMD_SEND_INIT_FAIL 96
#define CMD_SEND_GENERAL_FAIL 97
#define CMD_SEND_DEBUG_MESSAGE 98
#define CMD_SEND_FAILSAFE 99

//debug codes. Debug codes, by default, are ALWAYS sent over snap.
#define CMD_DEBUG_SEND_HELLO_WORLD 0
#define CMD_DEBUG_SEND_WRITING_MOTOR_1 1
#define CMD_DEBUG_SEND_WRITING_MOTOR_2 2
#define CMD_DEBUG_SEND_BRAKING_MOTOR_1 3
#define CMD_DEBUG_SEND_BRAKING_MOTOR_2 4
#define CMD_DEBUG_SEND_COASTING_MOTOR_1 5
#define CMD_DEBUG_SEND_COASTING_MOTOR_2 6
#define CMD_DEBUG_SEND_CURVE_SET 7
#define CMD_DEBUG_SEND_CURVE_OFF 8
#define CMD_DEBUG_SEND_BRAKE_ON 9
#define CMD_DEBUG_SEND_BRAKE_OFF 10
#define CMD_DEBUG_SEND_FS_BRAKE_ON 11
#define CMD_DEBUG_SEND_FS_BRAKE_OFF 12
#define CMD_DEBUG_SEND_BRAKE_TIMEOUT_SET 13
#define CMD_DEBUG_SEND_CURVE_SET_APPLIED 14
#define CMD_DEBUG_SEND_BRAKE_SET_APPLIED 15

//non-debug codes
#define CMD_BATTERY_LEVEL 16
#define CMD_DEBUG_SEND_INIT_COMPLETE 17