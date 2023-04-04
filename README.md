# Old-Timer

## 3D printed Arduino based interval timer display
Old Timer is a 3D printed wall mountable interval timer, intented for HIIT (High Intensity Interval Training). This is a DIY-project for a 64x64 RGB LED matrix display with a speaker and an interface, powered by an Arduino microcontroller board.

![image](/Old_Timer.png "Old Timer")

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
- 1 x Prototype Shield Board Kit for Arduino Mega \*
- 1 x (64x64) RGB LED matrix 
- 1 x DFPlayer mini MP3 player
- 1 x TF-card (DFPlayer supports up to: 32 GB)
- 1 x 3 W 8 &Omega; Speaker 
- 1 x rotary encoder with switch function (on press down)
- 1 x tactile switch (6 x 6 x 4.3 mm) 
- 1 x ON/OFF toggle round rocker switch 
- 1 x 1 k&Omega; resistor
- DuPont cables

\* *Note: The stock shield board kit for the Arduino Mega has female pins for DuPont cable connections. These can be desoldered and replaced with male pin connectors, since the DATA IN cable of the RGB LED matrix board comes with a female to female pin cable. Alternatively you can use DuPont cable connections and ommit the shield altogether. Also the FDPlayer can be mounted on the shield.*

## 3D printed case

The 3D printed case printed with PLA filament is split into a top and lower part. The top part is connected with the matrix display with 6 (M3 x 10 mm) screw bolts (red dots in the picture). The matrix display provides the structural strength, so the case walls can be very thin (2 - 3 mm). The lower body containing the speaker and buttons can be swapped/upgraded in the future. The bottom and top bodies are connected with 6 (M4 x 15 mm) screw bolts and M4 hex nuts (green dots in the picture). The case is sealed with two covers which can be screwed on using 8 (M3 x 10 mm) bolts (red dots in the picture). There is a mount for the Arduino board and a cover for the speaker which also can be fastened with (M3 x 10 mm) screw bolts. I have also made a case for the mini switch button (4 parts). It's a cheap alternative to buying a switch button with housing. The top part of the button can be screwed on the housing containing the mini switch to make a nice clicking action. Two hex nuts can be printed instead of one to better secure the housing and top part of the button. 

3D printed case assembly parts:

- 8 x 3D printed parts 
- 23 x (M3 x 10 mm) screw bolts 
- 6 x (M4 x 15 mm) screw bolts
- 6 x (M4 9 mm diameter) flat washers 
- 6 x M4 hex nuts

![image](/3D_printed_case_Exploded_view.png "exploded view of 3D printed case")

## Wiring 

![image](/Old_Timer_wiring_diagram.png "Old Timer wiring diagram")

For clarity the wiring can be split down into serveral parts:

### Power supply
The 64x64 RGB LED matrix alone requires a 5 V regulated DC power input and 4 A with all LEDs on at full brightness. However, a 5V 4A is sufficient to power to entire setup. For testing, the Arduino controller board can be powered directly from a computer using an USB connector without an external power supply. An ON/OFF toggle round rocker switch is added to switch the power supply on and off. 

### RGB LED Matrix 
The 64x64 RGB LED Matrix Panel has 4096 full-color RGB LEDs, which can be controlled independently. These kind of displays are designed to be driven by FPGAs (field-programmable gate arrays) and dont have any built in pulse-width modulation control. An Arduino MEGA 2560 R3 Controller Board is used since the RGB LED display needs 13 GPIOs (general-purpose input/output pins) to be controlled. Other microcontrollers (Raspberry Pi) with large enough speed and memory can be used. The Mega 2560 works great for this project, but higher speed controllers are needed to drive this display in "full glory". Alternatively smaller LED Matrix displays are available (for example a 1024 bright RGB LED display on a 32x32 grid). The matrix panel has two IDC (insulation-displacement contact) connectors DATA IN and DATA OUT to cascade displays. The Arduino isn't powerful enough however to control larger displays.

DATA IN IDC signal pins connected to Arduino MEGA 2560, L if left and R is right side of connector pins:

|DATA IN L|Arduino pins| DATA IN R|Arduino pins|
|:---:|:---:|:---:|:---:|
|R1|24|G1|25|
|B1|26|GND|GND|
|R2|27|G2|28|
|B2|29|E|A4|
|A|A0|B|A1|
|C|A2|D|A3|
|CLK|11|LAT|10|
|OE|9|GND|GND|

Power interface:
|RGB LED Matrix|AC-DC power adapter|
|:---:|:---:|
|2 x +5V | positive terminal|
|2 x GND | negative terminal|

### Interrupts: rotary encoder and push buttons

The interface of the interval timer is a rotary encoder and a push switches. The rotart enocoder has a built in push switch, which is used to navigate the menu. A mini tactile switch serves to pause the interval timer. Both inputs are connect via interrupt pins to the Arduino. Pins 2,3,18,19,20,21 can be used on the Arduino Mega 2560 as external interrupt pins.

|rotary encoder|mini push switch|
|--|--|
|<table> <tr><th>pin</th><th>Arduino</th></tr><tr><td>CLK</td><td>18</td></tr><tr><td>DT</td><td>19</td></tr><tr><td>SW</td><td>20</td></tr><tr><td>VCC</td><td>5V</td></tr><tr><td>GND</td><td>GND</td></tr></table>|<table><tr><th>pin</th><th>Arduino</th></tr><tr><td>HIGH</td><td>21</td></tr><tr><td>GND</td><td>GND</td></tr></table>|

### DFPlayer mini MP3 player

The DFPlayer is a MP3 player module with a TF card/MicroSD slot. The MP3 player can be powered by the 3.3 V or 5 V output of the Arduino. After testing the DFPlayer module in a mono circuit with a 3 W 8 &Omega; speaker-setup I found that module doesn't work reliably with 3.3 V input. With 5 V input, the Arduino doesn't seem to provide enough current when changing the volume to max. I've ordered and tested three MP3 modules. Two modules worked perfect, one was faulty. An external power supply with a max voltage of 5 V is ideal to reduce noise and increase the power from the speaker. However it is not necessary since the MP3 player won't be operating at max volume for an extented period of time in this project. I would recommend ordering a few MP3 player modules to serve as backup, since they are very cheap. The DFPlayer  has a 3 W amplifier. A speaker with 4 &Omega; or more impedance can also be used for a louder maximum volume.

The pins on the DFPlayer are not indicated so be mindful of the orientation. The signal pins of the DFPlayer mini module are not 5V tolerant so a voltage divider (1 k&Omega; resistor) is added between the RX signal pin and the Arduino. 

The DFPLayer pins connected to the Arduino:

|DFLPLayer|Arduino|
|:---:|:---:|
|VCC|5V|
|RX|52|
|TX|53|
|SPK1|SPK+|
|GND|GND|
|SPK2|SPK-|

## Code

## Credits and References
