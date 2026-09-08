#include "CRC8.h"
#include "config.h"

#include "HardwareSerial.h"

//timers
unsigned long timer;
unsigned long last_timer;
unsigned long last_packet;
unsigned long last_motor_1_active;
unsigned long last_motor_2_active;

//settings
int curve_exponent;
int motor_1_maximum_speed;
int motor_2_maximum_speed;
bool brake_enabled;
unsigned int brake_timeout;
bool failsafe_brake;

//motor controls
bool motor_1_active;
bool motor_2_active;
MotorControls m1;
MotorControls m2;

//packet controls
PacketControls pi;
PacketData d;

//initialize crc
CRC8 crc;






void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //reset everything to default values
  timer = 0;
  last_timer = 0;
  curve_exponent = 0;
  brake_enabled = true;
  failsafe_brake = true;
  motor_1_maximum_speed = USR_MOTOR_1_MAXIMUM_SPEED;
  motor_2_maximum_speed = USR_MOTOR_2_MAXIMUM_SPEED;

  crc.restart();

  sendPacket(FIREFLY_DEVICE_ADDRESS, IMPULSE_OUTBOUND_INFO, GLOBAL_INFO_DEVICE_READY);
}

void loop() {

  timer = millis();

  m1.curve_exponent = curve_exponent;
  m2.curve_exponent = curve_exponent;

  m1.maximum_speed = motor_1_maximum_speed;
  m2.maximum_speed = motor_2_maximum_speed;



  while (Serial.available()) {
    d = recieveAndAssign(pi, nullptr, &packet_code);
    if (!packet_code) {
      //main switch
      //if this was a really complicated or nested switch, i'd use a LUT, but this is just one long-ish thing
      switch (d.command) {
        case IMPULSE_COMMAND_SET_CURVE:
          curve_exponent = d.payload;
          break;

        case IMPULSE_COMMAND_SET_BRAKE:
          brake_enabled = d.payload;
          break;

        case IMPULSE_COMMAND_SET_FAIL_BRAKE:
          failsafe_brake = d.payload;
          break;

        case IMPULSE_COMMAND_SET_BRAKE_TIMEOUT:
          brake_timeout = d.payload;
          break;

        case IMPULSE_COMMAND_SET_CHANNEL_1_MAXIMUM_SPEED:
          motor_1_maximum_speed = d.payload;
          break;

        case IMPULSE_COMMAND_SET_CHANNEL_2_MAXIMUM_SPEED:
          motor_2_maximum_speed = d.payload;
          break;

        case IMPULSE_COMMAND_CHANNEL_1_FORWARDS:
          m1.payload = d.payload;
          analogWrite(PIN_CHANNEL_1_A, calculatePWM(m1));
          digitalWrite(PIN_CHANNEL_1_B, 0);

          motor_1_active = true;
          last_motor_1_active = timer;
          break;

        case IMPULSE_COMMAND_CHANNEL_1_BACKWARDS:
          m1.payload = d.payload;
          digitalWrite(PIN_CHANNEL_1_A, 0);
          analogWrite(PIN_CHANNEL_1_B, calculatePWM(m1));

          motor_1_active = true;
          last_motor_1_active = timer;
          break;

        case IMPULSE_COMMAND_CHANNEL_2_FORWARDS:
          m2.payload = d.payload;
          analogWrite(PIN_CHANNEL_2_A, calculatePWM(m2));
          digitalWrite(PIN_CHANNEL_2_B, 0);

          motor_2_active = true;
          last_motor_2_active = timer;
          break;

        case IMPULSE_COMMAND_CHANNEL_2_BACKWARDS:
          m2.payload = d.payload;
          digitalWrite(PIN_CHANNEL_2_A, 0);
          analogWrite(PIN_CHANNEL_2_B, calculatePWM(m2));

          motor_2_active = true;
          last_motor_2_active = timer;
          break;

        case IMPULSE_COMMAND_CHANNEL_1_STOP:
          if (brake_enabled && (timer - last_motor_1_active > brake_timeout)) {
            digitalWrite(PIN_CHANNEL_1_A, 1);
            digitalWrite(PIN_CHANNEL_1_B, 1);
          } else {
            digitalWrite(PIN_CHANNEL_1_A, 0);
            digitalWrite(PIN_CHANNEL_1_B, 0);
          }
          break;

        case IMPULSE_COMMAND_CHANNEL_2_STOP:
          if (brake_enabled) {
            digitalWrite(PIN_CHANNEL_2_A, 1);
            digitalWrite(PIN_CHANNEL_2_B, 1);
          } else {
            digitalWrite(PIN_CHANNEL_2_A, 0);
            digitalWrite(PIN_CHANNEL_2_B, 0);
          }
          break;

        case IMPULSE_COMMAND_STOP_ALL:
          if (brake_enabled) {
            digitalWrite(PIN_CHANNEL_1_A, 1);
            digitalWrite(PIN_CHANNEL_1_B, 1);
            digitalWrite(PIN_CHANNEL_2_A, 1);
            digitalWrite(PIN_CHANNEL_2_B, 1);
          } else {
            digitalWrite(PIN_CHANNEL_1_A, 0);
            digitalWrite(PIN_CHANNEL_1_B, 0);
            digitalWrite(PIN_CHANNEL_2_A, 0);
            digitalWrite(PIN_CHANNEL_2_B, 0);
          }
          break;

        case IMPULSE_COMMAND_RESET:
          timer = 0;
          last_timer = 0;
          curve_exponent = 0;
          brake_enabled = true;
          failsafe_brake = true;
          motor_1_maximum_speed = USR_MOTOR_1_MAXIMUM_SPEED;
          motor_2_maximum_speed = USR_MOTOR_2_MAXIMUM_SPEED;
      }
    } else {
      sendPacket(FIREFLY_DEVICE_ADDRESS, IMPULSE_OUTBOUND_ERROR, packet_code);
    }
    for (int i = 0; i <= 4; i++) {  //reset the packet to all 0
      pi.packet[i] = 0;
    }

    pi.start_chain = false;
    pi.num_bytes = 0;
  }
  if (timer - last_packet > USR_SERIAL_SAFETY_TIMEOUT) {
    digitalWrite(PIN_CHANNEL_1_A, brake_enabled);
    digitalWrite(PIN_CHANNEL_1_B, brake_enabled);
    digitalWrite(PIN_CHANNEL_2_A, brake_enabled);
    digitalWrite(PIN_CHANNEL_2_B, brake_enabled);
  }
}
