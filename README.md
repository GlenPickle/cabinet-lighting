Arduino controlled cabinet lighting with a Nextion Enhanced 3.2 inch TFT Display

## Design Description:

The Nextion screen allows the user to interface with the lights. A leader Arduino Pro Micro listens for instructions from the Nextion Interface over serial, and sends these instructions to a follower Arduino Pro Micro. The Leader/Follower design is used to prevent Nextion signals being missed while calling the blocking functions to change the color and brightness of the lights.

## Limitations:
In order for a Nextion callback function or Wire receiveEvent to update variables, they must be declared globally. This software and its Makefiles were made to be compiled on Linux. Windows compilation may be possible, but has not been tested.

## Screen Images:  
### The home screen:
![](https://imgur.com/g3xNJNB.jpg)  
### The color selection screen:
![](https://imgur.com/DqQiqbl.jpg)
### The sliders to update brightness and speed:
![](https://imgur.com/NN62oMc.jpg)
### The settings screen:
![](https://imgur.com/PV3pKLu.jpg)
### The Clock update screen. This is also used to set the times to turn off and on automatically:
![](https://imgur.com/fFX4if3.jpg)  

## Light Images:
### Solid color choices:
![](https://imgur.com/GQHbBjE.jpg)
![](https://imgur.com/pAfZUNj.jpg)
![](https://imgur.com/WPkfoqP.jpg)
![](https://imgur.com/Iu7Ayx7.jpg)
![](https://imgur.com/XyIGFUx.jpg)
![](https://imgur.com/LpxenCV.jpg)
![](https://imgur.com/Vy53HG7.jpg)
![](https://imgur.com/F3ouQmn.jpg)
### Pulse Effect:
<img src="https://imgur.com/eF5KMjp.gif"/>
### Alternating Effect
<img src="https://imgur.com/i9FhxUL.gif"/>
### Snake Effect
<img src="https://imgur.com/GRvgo3x.gif"/>
### Rainbow Effect
<img src="https://imgur.com/aAgiqMC.gif"/>
### Dance Party Effect
<img src="https://imgur.com/QXkDob8.gif"/>

## How To Recreate:
### Software:
#### Dependencies:
* [Nextion Arduino Library](https://nextion.tech/resources/download/libraries/)
* [FastLED Library](https://github.com/FastLED/FastLED/releases)
* arduino-mk

#### Steps:
1. Install dependencies
2. Change the variable NUM_LEDS in libraries/ledInterface/led_interface.h to the number of LEDs that you plan to use
3. Compile LED interface by running `make` in the led_interface directory
4. Compile Screen interface by running `make` in the screen_interface directory
5. Upload the resulting sketch files to the arduinos following these [instructions](https://www.dummies.com/computers/arduino/how-to-upload-a-sketch-to-an-arduino/)
6. Use the [Nextion Editor](https://nextion.tech/nextion-editor/) to compile the HMI file into a Nextion .tft file
7. Upload the .tft to the Nextion following these [instructions](https://nextion.tech/faq-items/using-nextion-microsd/)

### Hardware
#### Hardware List:
1. Two [Arduino Pro Micros](https://www.sparkfun.com/products/12640) (cheaper clones available on Amazon)
2. One [Nextion 3.2 inch Enhanced Display](https://nextion.tech/datasheets/nx4024k032/) (available on Amazon)
3. A Five volt power supply with sufficient amperage. 256 LEDs use 15 Amps.
4. A strip of WS2812b LEDs
5. The file to have the wall plate laser cut is available in the Nextion Directory of this repo. I used Ponoko to have it made. It fits a low voltage double gang wall box.

Wire the power supply, Nextion Display, LED strip, and two Arduino Pro Micros according to the following circuit diagram:  
![](https://imgur.com/bCpbyhI.png)  
If running the LEDs across two sides of the room like I did, be careful not to leave more than 15 feet of wire between any two lights. This causes loss of data.  
