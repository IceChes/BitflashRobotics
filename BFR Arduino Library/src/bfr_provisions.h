// Code comments written by me, formatted by Gemma 4, and edited by me again.

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

// Firefly
#define FIREFLY_DEVICE_ADDRESS 0
#define FIREFLY_EXPECTED_COMMAND_MAXIMUM 255  // [currently undefined]
// Firefly's inbound and outbound commands section is empty.
// It is the primary control device, so it sends and receives the data outlined elsewhere.

// Impulse
#define IMPULSE_DEVICE_ADDRESS 1
#define IMPULSE_EXPECTED_COMMAND_MAXIMUM 13
// Commands Impulse can receive
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
// Commands Impulse can send
#define IMPULSE_OUTBOUND_ERROR 0
#define IMPULSE_OUTBOUND_INFO 1
// Error codes Impulse can throw (remember that 0-5 are reserved for global packet errors)
// [there's nothing here]
// Info codes Impulse can send (remember that 0 is reserved for global alive signals)
// [there's nothing here]

/*
 * This is the PacketControls struct.
 * It contains all of the variables and information related to handling incoming packets.
 *
 * received_data : The current byte being received through Serial.read() (or hwSerial.read()).
 * start_chain   : A bool which indicates that a start byte has been received.
 *                 Helps receiveAndAssign() start looking for the rest of the data.
 * num_bytes     : An index for which bytes have been received.
 * packet        : The incoming data's temporary storage array.
 */

struct PacketControls {
  uint8_t received_data;  //raw received serial data
  bool start_chain;       //whether or not the start packet was received
  int num_bytes = 0;      //the number of bytes received so far
  uint8_t packet[5];      //the actual packet
};

/*
 * This is the finished PacketData struct.
 * It contains a finished packet.
 */

struct PacketData {
  uint8_t device;
  uint8_t command; 
  uint8_t payload; 
  uint8_t crc; //crc8 checksum
};

/*
 * Configuration struct.
 * Helps the packet checker determine what's allowed to be in a packet and what isn't.
 * Only one of these is allowed to exist.
 *
 * address         : Device address, used in the BFR packet protocol.
 * command_maximum : The maximum value that a device should expect to receive in the
 *                   command portion of a packet assigned to it.
 */

struct {
  uint8_t address;
  uint8_t command_maximum;
} Configuration;

// Function section

/*
 * void assign(address, max);
 * Sets up the packet checker config.
 *
 * int address : Device address, used in the BFR packet protocol. All of my devices
 *               have pre-defined addresses (they are just aliases for regular numbers).
 *
 * Predefined values:
 * FIREFLY_DEVICE_ADDRESS (0)
 * IMPULSE_DEVICE_ADDRESS (1)
 *
 * int max : The maximum value that a device should expect to receive in the
 *           COMMAND portion of a packet assigned to it.
 *           For example, Impulse's commands end at IMPULSE_COMMAND_RESET,
 *           which is an alias for 13. If Impulse receives a 14, something
 *           has gone wrong and that packet should be rejected.
 *
 * Predefined values:
 * FIREFLY_EXPECTED_COMMAND_MAXIMUM (255)
 * IMPULSE_EXPECTED_COMMAND_MAXIMUM (14)
 */

void assign(uint8_t _addr, uint8_t _max);

/*
 * void packetBad(PacketControls p);
 * A helper function that runs an unfinished packet through all of the checks it
 * has to go through before it is assigned to a PacketData struct.
 * You probably won't use this, it's used by receiveAndAssign().
 *
 * PacketControls p : The unfinished packet to check.
 *
 * Returns GLOBAL_PACKET_ERROR_NONE (0) if everything is okay. Otherwise returns:
 *
 * GLOBAL_PACKET_ERROR_WRONG_ADDRESS (1)
 * Packet was not addressed to the address given in assign().
 *
 * GLOBAL_PACKET_ERROR_OUT_OF_RANGE (2)
 * Device does not have a corresponding command value for the command received.
 *
 * GLOBAL_PACKET_ERROR_BAD_CRC (3)
 * All of the data is valid, but the CRC checksum doesn't seem to match up with the
 * one expected.
 *
 * GLOBAL_PACKET_ERROR_OTHER_PACKET (4)
 * Something else went wrong (catch-all, shouldn't happen).
 */

int packetBad(PacketControls _p);

/*
 * bool receiveAndAssign(PacketData* d, uint8_t* return_code, HardwareSerial* serialPort);
 * The primary function for receiving data into a packet. Returns true and
 * assigns the packet information to the passed PacketData item when a finished
 * and good packet is received.
 *
 * PacketData* d         : A reference to the PacketData struct to assign new packets to.
 * uint8_t* return_code  : A reference to a return code value to assign the current status
 *                         of the data collection. The following return codes can be
 *                         assigned to return_code based on what is going on:
 *
 * GLOBAL_PACKET_ERROR_NONE (0)         : No error.
 * GLOBAL_PACKET_ERROR_WRONG_ADDRESS (1): Packet was not addressed to the address
 *                                        given in assign().
 * GLOBAL_PACKET_ERROR_OUT_OF_RANGE (2) : Device does not have a corresponding command
 *                                        value for the command received.
 * GLOBAL_PACKET_ERROR_BAD_CRC (3)      : All of the data is valid, but the CRC
 *                                        checksum doesn't seem to match up with the
 *                                        one expected.
 * GLOBAL_PACKET_ERROR_OTHER_PACKET (4) : Something else went wrong with the
 *                                        packetBad() function (catch-all,
 *                                        shouldn't happen).
 *
 * HardwareSerial* serialPort : An optional parameter which allows the user to pass
 *                              on a HardwareSerial port. If this parameter is not
 *                              passed (or nullptr is passed), the default
 *                              Serial.read() will be used for reading new data.
 */

bool receiveAndAssign(PacketData* _d, uint8_t* return_code, HardwareSerial* serialPort = nullptr);

/*
 * sendPacket(uint8_t id, uint8_t command, uint8_t payload, HardwareSerial* serialPort)
 * The primary function for sending a packet. Automatically adds the preceding
 * start byte and calculates/appends the CRC checksum. Returns the CRC checksum
 * of the sent packet.
 *
 * uint8_t id       : The device ID of the target.
 * uint8_t command   : The command value to send to the target.
 * uint8_t payload   : The payload value to send to the target.
 */

uint8_t sendPacket(uint8_t _id, uint8_t _command, uint8_t _payload, HardwareSerial* serialPort = nullptr);

#endif
