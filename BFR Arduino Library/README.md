# This readme is AI slop rn i'll edit it extensively at some point

# BFR Arduino Library

A lightweight, robust packet-handling library designed for combat robot communication. This library provides a structured way to send and receive data over Serial or `HardwareSerial` ports, ensuring data integrity via CRC8 checksums and enforcing device-specific command limits.

## Packet Structure

Packets are structured as follows:

| Byte | Field | Description |
| :--- | :--- | :--- |
| 0 | **Start** | Fixed start byte (`255`) |
| 1 | **Device** | The target device ID |
| 2 | **Command** | The specific command to execute |
| 3 | **Payload** | The data associated with the command |
| 4 | **CRC** | CRC8 checksum of the preceding 4 bytes |

## Device Definitions

### Firefly (ID: 0)
The primary control device. It serves as the host, sending and receiving data for the entire system.

### Impulse (ID: 1)
A motor driver module. It supports a specific set of commands:
- `SET_CURVE`: 0
- `SET_BRAKE`: 1
- `SET_FAIL_BRAKE`: 2
- `SET_BRAKE_TIMEOUT`: 3
- `SET_CHANNEL_1_MAXIMUM_SPEED`: 4
- `SET_CHANNEL_2_MAXIMUM_SPEED`: 5
- `CHANNEL_1_FORWARDS`: 6
- `CHANNEL_1_BACKWARDS`: 7
- `CHANNEL_2_FORWARDS`: 8
- `CHANNEL_2_BACKWARDS`: 9
- `CHANNEL_1_STOP`: 10
- `CHANNEL_2_STOP`: 11
- `STOP_ALL`: 12
- `RESET`: 13

## Error Codes

The library uses the following global error codes for packet validation:

| Code | Error Type | Description |
| :--- | :--- | :--- |
| 0 | `NONE` | Packet is valid. |
| 1 | `WRONG_ADDRESS` | Packet was not addressed to the target device. |
| 2 | `OUT_OF_RANGE` | Command value is outside the device's allowed range. |
| 3 | `BAD_CRC` | Checksum verification failed. |
| 4 | `OTHER_PACKET` | Catch-all for unexpected errors. |
| 5 | `SERIAL_TIMEOUT` | Serial data timed out. |
| 6 | `UNFINISHED` | Packet is incomplete. |

## API Reference

### Data Structures

#### `PacketControls`
Internal structure used for accumulating incoming serial data.
- `received_data`: The current byte being read.
- `start_chain`: Boolean flag indicating if the start byte has been detected.
- `num_bytes`: Counter for bytes received so far.
- `packet[5]`: Temporary buffer for the incoming packet.

#### `PacketData`
The finalized, validated packet data.
- `device`: Device ID.
- `command`: Command ID.
- `payload`: Command payload.
- `crc`: Calculated CRC8 checksum.

#### `Configuration`
Global configuration for the packet checker.
- `address`: The target device address.
- `command_maximum`: The maximum allowed value for the command byte.

### Functions

#### `void assign(uint8_t _addr, uint8_t _max)`
Configures the packet checker with the target device's address and its maximum allowed command value.

#### `int packetBad(PacketControls _p)`
A helper function that runs an unfinished packet through all validation checks (Address, Range, CRC).
- **Returns**: `GLOBAL_PACKET_ERROR_NONE` (0) on success, or the specific error code on failure.

#### `bool receiveAndAssign(PacketData* _d, uint8_t* return_code, HardwareSerial* serialPort)`
The primary function for receiving data. It parses incoming serial data into a packet.
- **Returns**: `true` if a valid packet is received and assigned to `_d`.
- **Parameters**:
    - `_d`: Pointer to the `PacketData` struct to populate.
    - `return_code`: Pointer to an integer to store the status of the data collection.
    - `serialPort`: (Optional) Pointer to a `HardwareSerial` port. Defaults to the standard `Serial` port.

#### `uint8_t sendPacket(uint8_t _id, uint8_t _command, uint8_t _payload, HardwareSerial* serialPort)`
Builds a complete packet (adding the start byte and calculating the CRC) and transmits it.
- **Returns**: The CRC checksum of the sent packet.
- **Parameters**:
    - `_id`: Target device ID.
    - `_command`: Command to send.
    - `_payload`: Payload data.
    - `serialPort`: (Optional) Pointer to a `HardwareSerial` port. Defaults to the standard `Serial` port.
