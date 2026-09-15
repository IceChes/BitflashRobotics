#include "bfr_provisions.h"

void assign(uint8_t _addr, uint8_t _max) {
    Configuration.address = _addr;
    Configuration.command_maximum = _max;
}

int packetBad(PacketControls _p) {
    CRC8 crc;


    crc.add(_p.packet, 4);
    int crc_value = crc.calc();
    crc.restart();
    if (
        _p.packet[PACKET_START] == EXPECTED_START && _p.packet[PACKET_DEVICE] == Configuration.address && _p.packet[PACKET_COMMAND] < Configuration.command_maximum && _p.packet[PACKET_CRC] == crc_value) {
        return GLOBAL_PACKET_ERROR_NONE;  //no error
        } else if (_p.packet[PACKET_DEVICE] != Configuration.address) {
            return GLOBAL_PACKET_ERROR_WRONG_ADDRESS;  //data recieved is not addressed to target device
        } else if (_p.packet[PACKET_COMMAND] >= Configuration.command_maximum) {
            return GLOBAL_PACKET_ERROR_OUT_OF_RANGE;  //command out of range
        } else if (_p.packet[PACKET_CRC] != crc_value) {
            return GLOBAL_PACKET_ERROR_BAD_CRC;  //crc value incorrect
        } else {
            return GLOBAL_PACKET_ERROR_OTHER_PACKET;  //other error
        }
}


//function that looks for serial data and builds a packet with it
bool receiveAndAssign(PacketData* _d, uint8_t* return_code, HardwareSerial* serialPort) {
   PacketControls _p;
   
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

    if (_p.num_bytes >= 4) {   //if we have enough bytes, calculate the checksum
        _p.start_chain = false;  //whether or not the start packet was received
        _p.num_bytes = 0;        //the number of bytes received so far
        int packet_status = packetBad(_p);
        *return_code = packet_status;

        if (!packet_status && _d != nullptr) {  //if there is no error, continue

            _d->device = _p.packet[PACKET_DEVICE];
            _d->command = _p.packet[PACKET_COMMAND];
            _d->payload = _p.packet[PACKET_PAYLOAD];
            _d->crc = _p.packet[PACKET_CRC];
        }
        return true;
    } else {
        return false;
    }
}


uint8_t sendPacket(byte _id, byte _command, byte _payload, HardwareSerial* serialPort) {
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
