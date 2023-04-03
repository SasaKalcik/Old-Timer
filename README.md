# Old-Timer

## 3D printed Arduino based interval timer display
Old Timer is a 3D printed wall mountable interval timer, intented for HIIT (High Intensity Interval Training). This is a DIY-project for a 64x64 RGB LED matrix display with a speaker and an interface, powered by an Arduino microcontroller board.

## Hardware
This project started out with parts from an Arduino starter kit, 3D printed parts and a 64x64 RGB LED matrix. For an affordable minimalistic parts list (without the 3D printed case) I would recommend the following:

- 1 x AC-DC power adapter 5.0 V / 4.0 A  
- 1 x ELEGOO Mega R3 2560 Project Starter Kit 
- 1 x (64x64) RGB LED matrix
- 1 x DFPlayer mini MP3 player
- 1 x TF-card (DFPlayer supports up to: 32 GB)

Complete list of Old Timer's "electronic guts":

- 1 x AC-DC power adapter 5.0 V / 4.0 A  
- 1 x Female DC Threaded Barrel Jack Panel Adapter (or any connector suitable for plug of AC-DC power adapter)
- 1 x MEGA 2560 R3 Controller Board
- 1 x Prototype Shield Board Kit for Arduino Mega
- 1 x (64x64) RGB LED matrix 
- 1 x DFPlayer mini MP3 player
- 1 x TF-card (DFPlayer supports up to: 32 GB)
- 1 x 3 W 8 &Omega; Speaker 
- 1 x rotary encoder with push button
- 1 x mini push button
- 1 x ON/OFF toggle round rocker switch 
- 1 x 1 k&Omega; resistor
- DuPont cables

3D printed case assembly parts:

- 3D printed case 
- 19 x (M3 x 10 mm) screw bolts 
- 6 x (M4 x 15 mm) screw bolts
- 6 x (M4 9 mm diameter) flat washers 
- 6 x M4 hex nuts

## 3D printed case

## Wiring 

![Alt text](/Old_Timer_wiring_diagram.png "Old Timer wiring diagram")

For ease the wiring can be split down into serveral parts:

### Power supply
The 64x64 RGB LED matrix alone requires a 5V regulated power input and 4A with all LEDs on at full brightness. However, a 5V 4A is sufficient to power to entire setup. For testing, the Arduino controller board can be powered directly from a computer using an USB connector without an external power supply. An ON/OFF toggle round rocker switch is added to switch the power supply on and off. 

### RGB LED Matrix 
The 64x64 RGB LED Matrix Panel has 4096 full-color RGB LEDs, which can be controlled independently. These kind of displays are designed to be driven by FPGAs (field-programmable gate arrays) and dont have any built in pulse-width modulation control. An Arduino MEGA 2560 R3 Controller Board is used since the RGB LED display needs 13 GPIOs (general-purpose input/output pins) to be controlled. Other microcontrollers (Raspberry Pi) with large enough speed and memory can be used. The Mega 2560 works great for this project, but higher speed controllers are needed to drive this display in "full glory". Alternatively smaller LED Matrix displays are available (for example a 1024 bright RGB LED display on a 32x32 grid). The matrix panel has two IDC (insulation-displacement contact) connectors DATA IN and DATA OUT to cascade displays. The Arduino isn't powerful enough however to control larger displays.

DATA IN IDC signal pins connected to MEGA2560:

- R1  ->  24  |   G1  ->  25
- B1  ->  26  |   GND ->  GND
- R2  ->  27  |   G2  ->  28
- B2  ->  29  |   E   ->  A4
- A   ->  A0  |   B   ->  A1
- C   ->  A2  |   D   ->  A3
- CLK ->  11  |   LAT ->  10
- OE  ->  9   |   GND ->  GND

Power interface:
- 2 x +5V -> DC power supply positive
- 2 x GND -> DC power supply negative

### Interrupts: rotary enocoder and push buttons

### DFPlayer mini MP3 player



## Code

## Credits and References
