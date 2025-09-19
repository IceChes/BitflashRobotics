//Motor settings
#define USR_MOTOR_1_MINIMUM_VALUE -100  //-100-100
#define USR_MOTOR_1_MAXIMUM_VALUE 100
#define USR_MOTOR_2_MINIMUM_VALUE -100  
#define USR_MOTOR_2_MAXIMUM_VALUE 100
#define USR_INVERT_MOTOR_1 false  //whether the motors are wired backwards
#define USR_INVERT_MOTOR_2 true

#define USR_STT_DRIVE_ESC SERVO          //SERVO or SNAP
#define USR_STT_HAS_WEAPON true          //does the robot have a weapon
#define USR_STT_WEAPON_PROFILE VERTICAL  //VERTICAL or HORIZONTAL. ignored if HAS_WEAPON is false

//peripheral boards like sensors
#define USR_STT_PERIPHERAL_TYPE_I2C NO_I2C    //not supported right now
#define USR_STT_PERIPHERAL_TYPE_SNAP NO_SNAP  //not supported right now
#define USR_STT_COMPENSATE false;             //not supported right now

#define USR_STT_VERBOSE_LOGGING false  //whether to send INFO messages to serial

#define USR_SNAP_CFG_AUTO_BRAKE false
#define USR_SNAP_CFG_FAIL_BRAKE false
#define USR_SNAP_CFG_ENABLE_CURVE false
#define USR_SNAP_CFG_CURVE_VALUE 0