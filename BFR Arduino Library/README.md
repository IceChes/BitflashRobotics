# BitflashRobotics Arduino Library

A lightweight packet-based communication protocol and definition set designed for communication between components inside of a combat robot, but usable elsewhere. Communicates over standard UART serial supporting either standard Serial or ESP32 HardwareSerial ports. Packets and the data inside are rigorously checked for data integrity using checksums and expected range values.

This library makes heavy use of aliases for regular numbers to do its calculations and referencing.

To see the full list of aliases, view `bfr_provisions.h`.

## Packet Structure

Packets are structured as follows:

| Byte | Field | Description |
| :--- | :--- | :--- |
| 0 | **Start** | Fixed start byte (`255`) |
| 1 | **Device** | The target device ID |
| 2 | **Data Section 1 (Command)** | Used to tell the target device what to do |
| 3 | **Data Section 2 (Payload)** | Any data associated with the command that the target needs. |
| 4 | **CRC** | CRC8 checksum of the preceding 4 bytes |

## Packet Checks

Incoming packets must undergo a series of checks. 

- Start byte exists.
- Packet is addressed to device.
- Device has a cooresponding command to the received command value.
- Packet checksum is valid.

The library uses global error codes for packet validation. These codes are returned by the packet assignment function `receiveAndAssign()` via a pointer. If these codes are sent over the packet protocol, they are sent in the payload section. 

Checking if a packet has an error:
```arduino
if(receiveAndAssign(&d, &return_code)){
    if(return_code != GLOBAL_PACKET_ERROR_NONE){
        Serial.println("Got a garbled packet");
    }
}
```

Impulse reporting a garbled input packet to firefly:
```arduino
if(receiveAndAssign(&d, &return_code)){
    // code...
} 
else {
    sendPacket(FIREFLY_DEVICE_ADDRESS, IMPULSE_OUTBOUND_ERROR, return_code);
}
```

| Code | Error Type | Description |
| :--- | :--- | :--- |
| 0 | `GLOBAL_PACKET_ERROR_NONE` | Packet is valid. |
| 1 | `GLOBAL_PACKET_ERROR_WRONG_ADDRESS` | Packet was not addressed to the target device. |
| 2 | `GLOBAL_PACKET_ERROR_OUT_OF_RANGE` | Command value is outside the device's allowed range. |
| 3 | `GLOBAL_PACKET_ERROR_BAD_CRC` | Checksum verification failed. |
| 4 | `GLOBAL_PACKET_ERROR_OTHER_PACKET` | Catch-all for unexpected errors. |

Semi-related, there is also another global alias for 0, but it is only used in info packets.

`GLOBAL_INFO_DEVICE_READY = 0`

Used like this:
```arduino
if(receiveAndAssign(&d, &return_code)){
    if(d.payload == GLOBAL_INFO_DEVICE_READY){
        ready = true;
    }
} 
```

## Supported Device Definitions

### Firefly | ID: 0 (FIREFLY_DEVICE_ID)
ESP32-based primary robot controller. Controls all other areas of the robot and runs heavier compute loads.

### Impulse | ID: 1 (IMPULSE_DEVICE_ID)
Inexpensive, feature-rich motor driver module. 

#### Writing to Impulse
---

**IMPULSE_COMMAND_SET_CURVE** (Code: 0)
- Configures exponential throttle curve. 
- Cooresponding payload: the exponent of the curve. If this is not set, it defaults to 0 (linear).

Example function for host: 

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_SET_CURVE, 3); //set curve to 3
```
**IMPULSE_COMMAND_SET_BRAKE** (Code: 1)
- Tells Impulse if a stop command should mean to apply power to both terminals of the motor, or stop applying power. 
- Cooresponding payload: false or true (0/1) (coast/brake). If this is not set, it defaults to false.

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_SET_BRAKE, true); //turn on braking
```

**IMPULSE_COMMAND_SET_FAIL_BRAKE** (Code: 2)
- Tells Impulse if braking should be applied if the serial connection to the host is lost.
- Cooresponding payload: false or true (0/1) (coast/brake). If this is not set, it defaults to false.

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_SET_FAIL_BRAKE, false); //turn off failsafe braking
```

**IMPULSE_COMMAND_SET_BRAKE_TIMEOUT** (Code: 3)
- Impulse will not apply braking until this amount of time has passed since the motors have moved. This helps prevent transient EMF spikes from braking. Has no effect if braking is turned off.
- Cooresponding payload: 0-255 in ms for however long to set the timeout to. Usually 50-100ms is good. If this is not set, it defaults to 100ms. 

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_SET_BRAKE_TIMEOUT, 120); //brake after 120ms
```

**IMPULSE_COMMAND_SET_CHANNEL_X_MAXIMUM_SPEED** (Code: 4 + 5)
- Impulse will restrict motor maximum speeds in both directions to this value.
- Cooresponding payload: Value from 0-255 to limit motor speed to. Note that very low values will not provide enough power to move the motors at all.

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_SET_CHANNEL_2_MAXIMUM_SPEED, 127); //limit channel 2 to 50% throttle
```

**IMPULSE_COMMAND_CHANNEL_X_FORWARDS** (Code: 6 + 7)
- Energizes terminal 1 on the selected channel to a PWM value based on the payload given and the result of several calculations involving the motor's maximum speed configuration and Impulse's curve programming.
- Cooresponding payload: Value from 0-255 to write. Note that very low values will not provide enough power to move the motors at all. Artificially limiting the motor speed using this function (only writing 0-200, for example) is not recommended.

Example function for host:

```arduino
uint8_t motor_1_value;

motor_1_value = filtered_throttle;

sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_CHANNEL_1_FORWARDS, motor_1_value); //writes a value to motor 1 to move forward
```

**IMPULSE_COMMAND_CHANNEL_X_BACKWARDS** (Code: 8 + 9)
- Energizes terminal 2 on the selected channel to a PWM value based on the payload given and the result of several calculations involving the motor's maximum speed configuration and Impulse's curve programming.
- Cooresponding payload: Value from 0-255 to write. Note that very low values will not provide enough power to move the motors at all. Artificially limiting the motor speed using this function (only writing 0-200, for example) is not recommended.

Example function for host:

```arduino
uint8_t motor_1_value;

motor_1_value = reverse_throttle;

sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_CHANNEL_1_BACKWARDS, motor_1_value); //writes a value to motor 1 to move back
```

**IMPULSE_COMMAND_CHANNEL_X_STOP** (Code: 10 + 11)
- Stops motor output on one channel, either braking or coasting depending on the brake setting and timeout.
- Cooresponding payload: Payload is ignored.

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_CHANNEL_2_STOP, 0); //stops motor 2 - the 0 payload is ignored by impulse
```

**IMPULSE_COMMAND_STOP_ALL** (Code: 12)
- Stops both channels, either braking or coasting depending on the brake setting and timeout.
- Cooresponding payload: Payload is ignored.

Example function for host:

```arduino
sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_STOP_ALL, 0); //stops both motors - the 0 payload is ignored by impulse
```

**IMPULSE_COMMAND_RESET** (Code: 13)
- Resets all values to default. Also resets Impulse's internal timer.
    - Curve: n = 0
    - Brake timeout: 100ms
    - Brake enabled: false
    - Failsafe brake enabled: False

- Cooresponding payload: Payload is ignored.

Example function for host:

```arduino
void setup(){
    sendPacket(IMPULSE_DEVICE_ID, IMPULSE_COMMAND_RESET, 0); //reset impulse on host startup
}
```

#### Reading from Impulse

Impulse can send the following packets to the host device:

**IMPULSE_OUTBOUND_ERROR** (Code: 0)
- For error reporting only.
- Cooresponding payload: Impulse currently only supports the global packet error codes.

Example function for host:

```arduino
if(receiveAndAssign(&d_impulse, &impulse_return_code, &impulse)){
    if(d_impulse.command == IMPULSE_OUTBOUND_ERROR){
        Serial.println("Impulse reports error code: " + String(d_impulse.payload)); 
        // will print something like "Impulse reports error code: 2"
        // which cooresponds to the global "out of range" error 
        // GLOBAL_PACKET_ERROR_OUT_OF_RANGE
    }
} 

```
**IMPULSE_OUTBOUND_INFO** (Code: 1)
- For info reporting only.
- Cooresponding payload: Impulse currently only supports the global ready code.

Example function for host:

```arduino
if(receiveAndAssign(&d_impulse, &impulse_return_code, &impulse)){
    if(d_impulse.payload == GLOBAL_INFO_DEVICE_READY){
        impulse_ready = true;
    }
} 
```

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

```arduino
PacketData d;

if(receiveAndAssign(&d, &return_code)){
    // code
}
```

#### `Configuration`
Global configuration for the packet checker on the device running the code. For example, in Firefly's code, the `address` and `command_maximum` are set to `FIREFLY_DEVICE_ADDRESS` and `FIREFLY_EXPECTED_COMMAND_MAXIMUM`

- `address`: The target device address.
- `command_maximum`: The maximum allowed value for the command byte.

### Functions

#### `void assign(uint8_t _addr, uint8_t _max)`
Configures the packet checker with the target device's address and its maximum allowed command value.

#### `int packetBad(PacketControls _p)`
A helper function that runs an unfinished packet through all validation checks (Address, Range, CRC). You should never use this function, it's a helper function to be used inside of `receiveAndAssign()`.
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
