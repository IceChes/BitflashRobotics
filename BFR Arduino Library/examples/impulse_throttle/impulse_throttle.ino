#include <bfr_provisions.h>

// define a pin for the potentiometer
#define POT_PIN A0

int pot_reading;

// this is the datastream associated with impulse
// to intialize it, its device address is passed
// no serial port is passed so the default is used
DataStream impulse(IMPULSE_DEVICE_ADDRESS);

void setup(){

  // we use 115200baud for this protocol
  Serial.begin(115200);

  pinMode(A0, INPUT);
}

void loop(){

  pot_reading = analogRead(POT_PIN);

  // send a packet to impulse's device address along its serial port
  impulse.sendPacket(IMPULSE_COMMAND_CHANNEL_1_FORWARDS, map(pot_reading, 0, 1024, 0, 255));

}