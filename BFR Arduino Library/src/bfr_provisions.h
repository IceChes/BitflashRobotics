#ifndef BFR_PROVISIONS_H
#define BFR_PROVISIONS_H

#include <Arduino.h>
#include <CRC8.h>

#define EXPECTED_START 255

#define PACKET_START 0
#define PACKET_DEVICE 1
#define PACKET_COMMAND 2
#define PACKET_PAYLOAD 3
#define PACKET_CRC 4

#define GLOBAL_PACKET_ERROR_NONE 0
#define GLOBAL_PACKET_ERROR_WRONG_ADDRESS 1
#define GLOBAL_PACKET_ERROR_OUT_OF_RANGE 2
#define GLOBAL_PACKET_ERROR_BAD_CRC 3
#define GLOBAL_PACKET_ERROR_OTHER_PACKET 4
#define GLOBAL_PACKET_ERROR_SERIAL_TIMEOUT 5
#define GLOBAL_PACKET_ERROR_UNFINISHED 6

#define GLOBAL_INFO_DEVICE_READY 0

//firefly
#define FIREFLY_DEVICE_ADDRESS 0
#define FIREFLY_EXPECTED_COMMAND_MAXIMUM 255  //[currently undefined]
//firefly's inbound and outbound commands section is empty
//it is the primary control device, so it sends and receives the data outlined elsewhere

//impulse
#define IMPULSE_DEVICE_ADDRESS 1
#define IMPULSE_EXPECTED_COMMAND_MAXIMUM 12
//commands impulse can receive
#define IMPULSE_COMMAND_SET_CURVE 0
#define IMPULSE_COMMAND_SET_BRAKE 1
#define IMPULSE_COMMAND_SET_FAIL_BRAKE 2
#define IMPULSE_COMMAND_SET_BRAKE_TIMEOUT 3
#define IMPULSE_COMMAND_SET_CHANNEL_1_MAXIMUM_SPEED 4
#define IMPULSE_COMMAND_SET_CHANNEL_2_MAXIMUM_SPEED 5
#define IMPULSE_COMMAND_CHANNEL_1_FORWARDS 6
#define IMPULSE_COMMAND_CHANNEL_1_BACKWARDS 7
#define IMPULSE_COMMAND_CHANNEL_2_FORWARDS 8
#define IMPULSE_COMMAND_CHANNEL_2_BACKWARDS 9
#define IMPULSE_COMMAND_CHANNEL_1_STOP 10
#define IMPULSE_COMMAND_CHANNEL_2_STOP 11
#define IMPULSE_COMMAND_STOP_ALL 12
#define IMPULSE_COMMAND_RESET 13
//commands impulse can send
#define IMPULSE_OUTBOUND_ERROR 0
#define IMPULSE_OUTBOUND_INFO 1
//error codes impulse can throw (remember that 0-5 are reserved for global packet errors)
//[there's nothing here]
//info codes impulse can send (remember that 0 is reserved for global alive signals)
//[there's nothing here]

struct PacketControls {
  uint8_t received_data;  //raw received serial data
  bool start_chain;       //whether or not the start packet was received
  int num_bytes = 0;      //the number of bytes received so far
  uint8_t packet[5];      //the actual packet
};

struct PacketData {
  uint8_t device;
  uint8_t command;  //the command part of the packet
  uint8_t payload;  //the payload part of the packet
  uint8_t crc;
};

struct {
  uint8_t address;
  uint8_t command_maximum;
} Configuration;

void assign(uint8_t _addr, uint8_t _max);

int packetBad(PacketControls _p);

bool receiveAndAssign(PacketControls _p, PacketData* _d, uint8_t* return_code, HardwareSerial* serialPort = nullptr);

int sendPacket(byte _id, byte _command, byte _payload, HardwareSerial* serialPort = nullptr);

#endif
