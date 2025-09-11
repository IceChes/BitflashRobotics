//Motor settings
#define MOTOR_1_MINIMUM_VALUE -100  //-100-100
#define MOTOR_1_MAXIMUM_VALUE 100
#define MOTOR_2_MINIMUM_VALUE -100  
#define MOTOR_2_MAXIMUM_VALUE 100
#define INVERT_MOTOR_1 false  //whether the motors are wired backwards
#define INVERT_MOTOR_2 true

#define DRIVE_ESC SERVO          //SERVO or SNAP
#define HAS_WEAPON true          //does the robot have a weapon
#define WEAPON_PROFILE VERTICAL  //VERTICAL or HORIZONTAL. ignored if HAS_WEAPON is false

//peripheral boards like sensors
#define PERIPHERAL_TYPE_I2C NO_I2C    //not supported right now
#define PERIPHERAL_TYPE_SNAP NO_SNAP  //not supported right now
#define COMPENSATE false;             //not supported right now

#define VERBOSE_LOGGING false  //whether to send INFO messages to serial

#define SNAP_CFG_AUTO_BRAKE false
#define SNAP_CFG_FAIL_BRAKE false
#define SNAP_CFG_ENABLE_CURVE false
#define SNAP_CFG_CURVE_VALUE 0