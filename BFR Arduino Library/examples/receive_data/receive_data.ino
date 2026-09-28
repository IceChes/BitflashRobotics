// for usage on an ESP32 or other device that supports HardwareSerial
// pretends to be an impulse device and prints intercepted data
#include <bfr_provisions.h>

// change device serial port here
#define HW_SERIAL_PORT 1

// initialize a HardwareSerial port (1) for the firefly
HardwareSerial FireflySerial(HW_SERIAL_PORT);

// set up firefly's datastream using its address and our firefly serial port
DataStream firefly(FIREFLY_DEVICE_ADDRESS, &FireflySerial);

// set up firefly's data struct
PacketData firefly_data;

// firefly's return code
uint8_t return_code;

void setup(){

  // set up the device using impulse's information
  assign(IMPULSE_DEVICE_ADDRESS, IMPULSE_EXPECTED_COMMAND_MAXIMUM);

  // start up our communication
  Serial.begin(9600);
  FireflySerial.begin(115200);
}

void loop(){
  while(FireflySerial.available()){

    //assign data to the firefly_data struct if there is new data
    if(firefly.receiveAndAssign(&firefly_data, &return_code)){

      // if there is no error, new data will be written to firefly_data and printed to debug
      if(return_code == GLOBAL_PACKET_ERROR_NONE){
        Serial.println("Packet OK");
        Serial.println("DEV: " + String(firefly_data.device));
        Serial.println("CMD: " + String(firefly_data.command));
        Serial.println("PLD: " + String(firefly_data.payload));
        Serial.println("CRC: " + String(firefly_data.crc));
        Serial.println("");
      }
      
      // if there is an error, the data will not be written but the return code will be updated with the error code
      else{
        Serial.println("Packet error");
        Serial.println("ERR: " + String(return_code));
        Serial.println("");
      }
    }
  }
}