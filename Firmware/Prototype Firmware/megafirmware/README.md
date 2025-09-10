# MegaFirmware
## Introduction
MegaFirmware is an attempt to streamline Bitflash Robotics receiver into a single unified build with a configuration file and expandable main program. 

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