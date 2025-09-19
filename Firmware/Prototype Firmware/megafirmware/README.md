# MegaFirmware
## Introduction
MegaFirmware is an attempt to streamline Bitflash Robotics receiver firmware into a single unified build with a configuration file and expandable main program. It is in early testing, so it is here.

## Design Intent
- MegaFirmware is designed to run only on the Bitflash Robotics Xiao ESP32-C3 mainboard.
- The user's primary interaction with the firmware should only be changing the defines in the config.h file. 
- The user can define a drive ESC system, weapon system, and any peripherals they have onboard, and the firmware will automatically figure it out.
- To add new drive ESC systems, weapon systems, or peripherals, the user need only edit the switch statements in the mainline setup and loop portions of the Arduino sketch.
- No matter what is going on in the code, the program should always failsafe correctly. 

## How It Works
Using the defines in config.h, the program is able to use various control schemes to control a combat robot. It does this with multiple large switch statements inside the setup and the loop functions of the code in order to setup and make use of various control schemes. 

## Flashing
Unplug everything from your board. While holding the BOOT button, connect the USB cable to the device. Release the BOOT button when it is plugged in. Then, set your defines and flash the code. Afterwards, press RESET and verify that everything is working in the serial monitor. After that, disconnect your board and connect it back to your peripheral devices, and then safely verify that everything is functioning.

## Changing Defines
| Define                | Typical Values            | Purpose                                                                                                                                                                   |
| --------------------- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| MOTORS_MINIMUM_VALUE  | Integers from -100 to 100 | Set the minimum throttle value of the drive motors, with -100 being maximum speed backwards, 0 being idle, and 100 being maximum speed forwards.                          |
| MOTORS_MAXIMUM_VALUE  | Integers from -100 to 100 | Set the maximum throttle value of the drive motors, with -100 being maximum speed backwards, 0 being idle, and 100 being maximum speed forwards.                          |
| INVERT_MOTOR_1        | true/false                | Reverses forwards/backwards on motor 1.                                                                                                                                   |
| INVERT_MOTOR_2        | true/false                | Reverses forwards/backwards on motor 1.                                                                                                                                   |
| DRIVE_ESC             | SERVO, SNAP               | Selects the drive ESC used in the robot, either servo (like a Repeat Brushed DESC) or the SnapSerial motor driver. SS drivers must be connected to the SnapSerial 1 port. |
| HAS_WEAPON            | true/false                | If set to false, disables all weapon processing.                                                                                                                          |
| WEAPON_PROFILE        | VERTICAL, HORIZONTAL      | Selects the weapon type used in the robot.                                                                                                                                |
| PERIPHERAL_TYPE_I2C   | NO_I2C                    | Currently unsupported. Peripheral daughterboard used on the I2C bus.                                                                                                      |
| PERIPHERAL_TYPE_SNAP  | NO_SNAP                   | Currently unsupported. Peripheral daughterboard used on the SnapSerial 2 bus.                                                                                             |
| COMPENSATE            | false                     | Currently unsupported. Will be used to activate gyro compensation.                                                                                                        |
| VERBOSE_LOGGING       | true/false                | Whether to send noncritical INFO messages to the serial monitor.                                                                                                          |
| SNAP_CFG_AUTO_BRAKE   | true/false                | If using SnapSerial driver, whether to automatically try to brake after 100ms.                                                                                            |
| SNAP_CFG_FAIL_BRAKE   | true/false                | If using SnapSerial driver, whether to try to brake when the receiver sends the failsafe command.                                                                         |
| SNAP_CFG_ENABLE_CURVE | true/false                | If using SnapSerial driver, whether to enable throttle response curving.                                                                                                  |
| SNAP_CFG_CURVE_VALUE  | Integers from 0 to 10     | If using SnapSerial driver, the curve value used by the throttle curving equation.                                                                                        |