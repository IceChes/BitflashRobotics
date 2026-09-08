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
  int device;
  int command;  //the command part of the packet
  int payload;  //the payload part of the packet
  int crc;
};

int packet_code;  //return code for the packet checker

int packetBad(PacketControls _p) {
  CRC8 crc;


  crc.add(_p.packet, 4);
  int crc_value = crc.calc();
  crc.restart();
  if (
    _p.packet[PACKET_START] == EXPECTED_START && _p.packet[PACKET_DEVICE] == INTERNAL_DEVICE_ADDRESS && _p.packet[PACKET_COMMAND] < INTERNAL_DEVICE_EXPECTED_COMMAND_MAXIMUM && _p.packet[PACKET_CRC] == crc_value) {
    return GLOBAL_PACKET_ERROR_NONE;  //no error
  } else if (_p.packet[PACKET_DEVICE] != INTERNAL_DEVICE_ADDRESS) {
    return GLOBAL_PACKET_ERROR_WRONG_ADDRESS;  //data recieved is not addressed to target device
  } else if (_p.packet[PACKET_COMMAND] >= INTERNAL_DEVICE_EXPECTED_COMMAND_MAXIMUM) {
    return GLOBAL_PACKET_ERROR_OUT_OF_RANGE;  //command out of range
  } else if (_p.packet[PACKET_CRC] != crc_value) {
    return GLOBAL_PACKET_ERROR_BAD_CRC;  //crc value incorrect
  } else {
    return GLOBAL_PACKET_ERROR_OTHER_PACKET;  //other error
  }
}


//function that looks for serial data and builds a packet with it
PacketData recieveAndAssign(PacketControls _p, HardwareSerial* serialPort = nullptr, int* return_code = 0) {
  if (serialPort == nullptr) {
    _p.received_data = Serial.read();

  } else {
    _p.received_data = serialPort->read();
  }  //if got start bit
  if (_p.received_data == EXPECTED_START && !_p.start_chain) {

    _p.packet[0] = _p.received_data;
    _p.start_chain = true;  //start the chain
  } else if (_p.start_chain) {

    _p.num_bytes++;
    _p.packet[_p.num_bytes] = _p.received_data;
  }

  if (_p.num_bytes >= 4) {  //if we have enough bytes, calculate the checksum
    int packet_status = packetBad(_p);
    *return_code = packet_status;
    if (!packet_status) {  //if there is no error, continue

      PacketData _d;

      _d.device = _p.packet[PACKET_DEVICE];
      _d.command = _p.packet[PACKET_COMMAND];
      _d.payload = _p.packet[PACKET_PAYLOAD];
      _d.crc = _p.packet[PACKET_CRC];

      return _d;
    }
  }
}


int sendPacket(byte _id, byte _command, byte _payload, HardwareSerial* serialPort = nullptr) {
  CRC8 crc;

  uint8_t packet[5] = { 255, _id, _command, _payload };

  crc.add(packet, 4);
  packet[4] = crc.calc();

  if (serialPort == nullptr) {
    Serial.write(packet, sizeof(packet));
  } else {
    serialPort->write(packet, sizeof(packet));
  }

  return packet[4];
}