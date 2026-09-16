# What is this project?
Bitflash Robotics is an attempt to modernize combat robotics electronics. 

## Devices (see devices page for more information)
### Firefly

ESP32C3-based BLE combat robot receiver. Connects to a standard BLE game controller and controls all other robotic components. Also runs more complex compute loads if needed.

Has 2 snap (serial) ports, 1 servo port, and 1 Qwiic port. 

### Impulse

megaAVR-based brushed motor controller. Connects to one of Firefly's snap ports using a standard Qwiic/Stemma QT cable. Controls 2 brushed motors with short circuit protection and supplies 3.3v power to the robot's low voltage subsystem. 

## Standards
### BFR Packet Protocol (See Bitflash Robotics Arduino Library page for more information)

High speed structured protocol for writing sets of bytes over a standard UART bus to communicate with components. Due to its efficiency, it is able to run at extremely high speeds. Has basic error checking procedures to ensure safety and reliability in high-noise, high-intensity environments.

### Snap Serial (See Snap Serial page for more information)

Snap Serial (or just Snap) is simply UART over a Qwiic/Stemma QT port. This helps reduce connector size in the extremely tight electronics bays inside of combat robots. 

## Robots
### ???
