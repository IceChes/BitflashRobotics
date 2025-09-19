//Version 0.0

#define CMD_WRITE_LED_BATTERY 0           //modulates red and green LEDS to display a battery voltage gradient
#define CMD_WRITE_LED_BATTERY_CRITICAL 1  //flashes red LED endlessly
#define CMD_WRITE_LED_FAILSAFE 2          //flashes green LED endlessly

#define CMD_WRITE_LED_SNAP_INIT_FAIL 3        //flashes blue and white LED together 3 times
#define CMD_WRITE_LED_SNAP_LOST_CONNECTION 4  //flashes blue and white LED together 2 times
#define CMD_WRITE_LED_SNAP_FAULT_GENERAL 5    //flashes blue and white LED together 1 time

#define CMD_WRITE_LED_FIREFLY_INIT_FAIL 6        //flashes blue LED 3 times
#define CMD_WRITE_LED_FIREFLY_LOST_CONNECTION 7  //flashes blue LED 2 times
#define CMD_WRITE_LED_FIREFLY_FAULT_GENERAL 8    //flashes blue LED 1 time

#define CMD_WRITE_LED_LUMEN_INIT_FAIL 9  //flashes white led 3 times
#define CMD_WRITE_LED_LUMEN_IMU_FAIL 10  //flashes white LED 2 times
#define CMD_WRITE_LED_LUMEN_INVERTED 11  //flashes white LED 1 time

#define CMD_GET_IMU_ACCEL_COMBINED 12
#define CMD_GET_IMU_ACCEL_X 13
#define CMD_GET_IMU_ACCEL_Y 14
#define CMD_GET_IMU_ACCEL_Z 15

#define CMD_GET_IMU_ORIEN_PITCH 16  //forward
#define CMD_GET_IMU_ORIEN_YAW 17    //rotation
#define CMD_GET_IMU_ORIEN_ROLL 18   //side

#define PIN_LED_1 0  //all of these are pwm pins
#define PIN_LED_2 1
#define PIN_LED_3 2
#define PIN_LED_4 3

#define STT_LED_BATTERY 0
#define STT_LED_BATTERY_CRITICAL 1
#define STT_LED_FAILSAFE 2

#define STT_LED_SNAP_INIT_FAIL 0
#define STT_LED_SNAP_LOST_CONNECTION 1
#define STT_LED_SNAP_FAULT_GENERAL 2

#define STT_LED_FIREFLY_INIT_FAIL 3
#define STT_LED_FIREFLY_LOST_CONNECTION 4
#define STT_LED_FIREFLY_FAULT_GENERAL 5

#define STT_LED_LUMEN_INIT_FAIL 6
#define STT_LED_LUMEN_IMU_FAIL 7
#define STT_LED_LUMEN_INVERTED 8



//led states
byte led_1_2_states[3] = { STT_LED_BATTERY,
                           STT_LED_BATTERY_CRITICAL,
                           STT_LED_FAILSAFE };

byte led_3_4_states[9] = { STT_LED_SNAP_INIT_FAIL,
                           STT_LED_SNAP_LOST_CONNECTION,
                           STT_LED_SNAP_FAULT_GENERAL,
                           STT_LED_FIREFLY_INIT_FAIL,
                           STT_LED_FIREFLY_LOST_CONNECTION,
                           STT_LED_FIREFLY_FAULT_GENERAL,
                           STT_LED_LUMEN_INIT_FAIL,
                           STT_LED_LUMEN_IMU_FAIL,
                           STT_LED_LUMEN_INVERTED };

byte led_1_2_state;
byte led_3_4_state;

//incoming data var
byte data;

//timer vars
unsigned long timer;
unsigned long last_timer;

unsigned long last_led_1_flash;
unsigned long last_led_2_flash;
unsigned long last_led_3_flash;
unsigned long last_led_4_flash;

//led vars
byte led_1;
byte led_2;
bool led_3;
bool led_4;

//imu vars
int imu_accel_combined;
int imu_accel_x;
int imu_accel_y;
int imu_accel_z;

int imu_pitch;
int imu_yaw;
int imu_roll;

//sensor vars
byte battery_level;

void setup() {
  Serial.begin(9600);  //start serial comm
}

void loop() {
  timer = millis();    //enable timer
  last_timer = timer;  //refresh timer

  if (Serial.available()) {
    data = Serial.read();

    if (data < 3) {  //0, 1, and 2 are LED 1+2 commands
      led_1_2_state = led_1_2_states[data];
    }

    else if (data >= 3 && data < 12) {  //3-11 are LED 3+4 commands
      led_3_4_state = led_3_4_states[data-3];
    }

    else if (data >= 12 && data < 100) {
      //TODO: add switch to read and use the BNO data
    }

    else if (data >= 100 && data < 201) {
      battery_level = map(data, 100, 200, 0, 100);
    }
  }

  switch (led_1_2_state) {
    case STT_LED_BATTERY:

      break;


    case STT_LED_BATTERY_CRITICAL:

      break;


    case STT_LED_FAILSAFE:

      break;


    default:

      break;
  }

  switch (led_3_4_state) {
    case STT_LED_SNAP_INIT_FAIL:

      break;


    case STT_LED_SNAP_LOST_CONNECTION:

      break;


    case STT_LED_SNAP_FAULT_GENERAL:

      break;


    case STT_LED_FIREFLY_INIT_FAIL:

      break;


    case STT_LED_FIREFLY_LOST_CONNECTION:

      break;


    case STT_LED_FIREFLY_FAULT_GENERAL:

      break;


    case STT_LED_LUMEN_INIT_FAIL:

      break;


    case STT_LED_LUMEN_IMU_FAIL:

      break;


    case STT_LED_LUMEN_INVERTED:

      break;


    default:

      break;
  }
}
