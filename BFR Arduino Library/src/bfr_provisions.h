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
#define IMPULSE_EXPECTED_COMMAND_MAXIMUM 13
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

/*
this is the packetcontrols struct. 
it takes all the variables and information related to incoming packets and bundles it all into a neat struct.

received_data : the current byte being received through Serial.read() (or hwSerial.read())
start_chain : a bool which indicates that a start byte has been received. helps receiveAndAssign() start looking for the rest of the data
num_bytes : an index for which bytes have been received
packet : the incoming data's temporary storage array
*/
struct PacketControls {
  uint8_t received_data;  //raw received serial data
  bool start_chain;       //whether or not the start packet was received
  int num_bytes = 0;      //the number of bytes received so far
  uint8_t packet[5];      //the actual packet
};

/*
this is the finished packetdata struct.
it contains a finished packet.
*/
struct PacketData {
  uint8_t device;
  uint8_t command; 
  uint8_t payload; 
  uint8_t crc; //crc8 checksum
};

/*
configuration struct.
helps the packet checker determine what's allowed to be in a packet and what isn't. only one of these is allowed to exist.

address : device address, used in the BFR packet protocol.
command_maximum : the maximum value that a device should expect to receive in the COMMAND portion of a packet assigned to it.
*/
struct {
  uint8_t address;
  uint8_t command_maximum;
} Configuration;

//function section

/*
void assign(address, max);
sets up the packet checker config.

int address : device address, used in the BFR packet protocol. all of my devices have pre-defined addresses. 
notably, they are just aliases for regular numbers.

predefined values:
FIREFLY_DEVICE_ADDRESS (0)
IMPULSE_DEVICE_ADDRESS (1)

int max : the maximum value that a device should expect to receive in the COMMAND portion of a packet assigned to it.
for example, impulse's commands end at IMPULSE_COMMAND_RESET, which is an alias for 13. 
if impulse receives a 14, something has gone wrong and that packet should be rejected.

predefined values:
FIREFLY_EXPECTED_COMMAND_MAXIMUM (255)
IMPULSE_EXPECTED_COMMAND_MAXIMUM (14)
*/
void assign(uint8_t _addr, uint8_t _max);

/*
void packetBad(PacketControls p);
a helper function that runs an unfinished packet through all of the checks it has to go through before it is assigned to a PacketData struct.
you probably won't use this.

PacketControls p : the unfinished packet to check

returns GLOBAL_PACKET_ERROR_NONE (0) if everything is okay. otherwise returns:

GLOBAL_PACKET_ERROR_WRONG_ADDRESS (1)
packet was not addressed to the address given in assign()

GLOBAL_PACKET_ERROR_OUT_OF_RANGE (2)
device does not have a corresponding command value for the command received

GLOBAL_PACKET_ERROR_BAD_CRC (3)
all of the data is valid, but the CRC checksum doesn't seem to match up with the one expected

GLOBAL_PACKET_ERROR_OTHER_PACKET (4)
something else went wrong (catch-all, shouldn't happen)

*/
int packetBad(PacketControls _p);

/*
bool receiveAndAssign(PacketData* d, uint8_t* return_code, HardwareSerial* serialPort);
the primary function for receiving data into a packet.
returns true and assigns the packet information to the passed PacketData item when a finished and good packet is received.

PacketData* d : a reference to the PacketData struct to assign new packets to
uint8_t* return code : a reference to a return code value to assign the current status of the data collection to
the following return codes can be assigned to return_code based on what is going on:

GLOBAL_PACKET_ERROR_NONE (0)
no error

GLOBAL_PACKET_ERROR_WRONG_ADDRESS (1)
packet was not addressed to the address given in assign()

GLOBAL_PACKET_ERROR_OUT_OF_RANGE (2)
device does not have a corresponding command value for the command received

GLOBAL_PACKET_ERROR_BAD_CRC (3)
all of the data is valid, but the CRC checksum doesn't seem to match up with the one expected

GLOBAL_PACKET_ERROR_OTHER_PACKET (4)
something else went wrong with the packetBad() function (catch-all, shouldn't happen)

these values come from the packetBad() function, which is used inside of this function.

HardwareSerial* serialPort : an optional parameter which allows the user to pass on a HardwareSerial port.
if this parameter is not passed (or nullptr is passed), the default Serial.read() will be used for reading new data.
*/

bool receiveAndAssign(PacketData* _d, uint8_t* return_code, HardwareSerial* serialPort = nullptr);

/*
sendPacket(uint8_t id, uint8_t command, uint8_t payload, HardwareSerial* serialPort)
the primary function for sending a packet. automatically adds the preceding start byte and calculates/appends the CRC checksum.
returns the CRC checksum of the sent packet.

uint8_t id : the device ID of the target
uint8_t command : the command value to send to the target
uint8_t payload : the payload value to send to the target
*/
uint8_t sendPacket(uint8_t _id, uint8_t _command, uint8_t _payload, HardwareSerial* serialPort = nullptr);

#endif
