M5Stack M5Echo morse code test

PURPOSE: 
Create morse code audio using a M5Stack M5Echo device.

VERSIONS:

There are three versions of this software.
```
  a) a one device setup;
  b) a two devices setup;
  c) a morse code speed test.
```
In the ```src``` folder are three subfolders, one for each software version.

Hardware used:

In the one device setup version:
```
  1) M5Stack M5Echo;
  2) M5Stack Dualbutton unit;
```
In the two device setup version also:
```
  3) a second microcontroller board, in my case an iLabs cPico (2035A) board.
```
The software consists of:

In the one device setup version:
```
  a) an Arduino sketch that runs on the M5Echo device.
```
In the two device setup version:
```
  a) an Arduino sketch that runs on the M5Echo device (a different, more lines of code, compared to the one device setup version);
  b) a MicroPython script to run on the second device, in my case an iLabs cPico (RP2350A).
```
In the morse code speed test:
```
a) an Arduino sketch that runs on the M5Echo device (one device setup version).
   Note that in this sketch, lines 313-341, show how the morse code is timed.
```
SHORT DESCRIPTION:

THE M5ECHO DEVICE

In the one device setup:
  At startup de M5Echo speaker device will start to sound a pre-programmed text:
  "cq cq cq de ct7agr ct7agr ct7agr k", meaning: "To all stations (3x) this is 
  (the portuguese hamradio station (with callsign)) ct7agr (3x) over (k)".

  When you have the M5 Dualbutton unit connected to the GROVE connector of the 
  M5Echo device, you can interrupt the sending of the morse code by pressing or
  the red or the blue button.

  When no morse code is sending, then the buttons of the M5 Dualbutton unit
  can be used to change the morse speed setting.
  Note that this speed change feature is under development and is not perfect yet.
  Lines 78-93 contain a table that shows various settings of the ```unit_delay``` (dly1),
  as a function of the actual morse code speed sent.

In the two device setup:

THE SECOND DEVICE (EVENTUALLY)

I created this second version because I wanted to increase my experience with programming
of the ```Programmable Input/Output``` (PIO) peripheral of the RP2040/RP2350.
Instead of creating audio morse code under control of an Arduino sketch running on the
M5Echo device, you can use a second microcontroller board to send morse code
dot and dash commands. The dash command by sending a logical state '0' (= active) 
on the GROVE connector of the M5Echo (yellow line), Pin G26. 
To send a morse code dot command, by sending a logical state '0' on the GROVE connector 
of the M5Echo (white line), Pin G32. 
The Arduino sketch running on the M5Echo has two interrupt callback functions:
```dot_cb()``` and ```dash_cb()```. 
To keep the interrupt callback function as short as possible,
these callback functions only set a boolean flag: ```dot_rcvd``` and ```dash_rcvd```.
Next, from within the loop() function, the functions ```handle_dot()``` or ```handle_dash()```
will be called upon which of the two flags has been set. 
The MicroPython script running on the second device (in my case: the iLabs cPico) 
determines the timing for a good morse code spacing and rythm.

Experience with the two devices setup:
Initially I criated the two devices setup. During tests I was not satisfied with the 
timing of the dots and dashes audible through the speaker of the M5Echo when controlled 
by a second microcontroller board.
I have reason to believe that other processes running on core 0 of the M5Echo 
caused unreliable timing in the morse code produced. I already tried to use semaphore 
techniques however I was not satisfied with the outcome, so I removed the
semaphore functionality. Because of these problems I crated also the "one device setup"
version for the M5Echo.

In the morse speed test:

THE M5ECHO DEVICE

During one minute the word "paris" repeatedly will be send in morse code.
At the end of the test, the number of times the word "paris" was sent will be printed.
This value represents the speed in morse code (words per minute). 
In the sketch there are three global variables that determine the speed and the duration of 
the morse code being send. The variable ```dly1``` (unit delay) is created
and set in line 96. The values of ```dly3``` (character delay) and ```dly7``` (word delay)
are derived from variable dly1 as: dly3 = 3 * dly1 and dly7 = 7 * dly1. 
The value of dly1 determines the speed the morse code will be send. 
The morse speed to send can be varied between 10 and 54 words per minute. 
In lines 78-99 of the Arduino sketch is a table that shows 
the relation between the value of dly1 and the morse speed being send.
For example: the default value of dly1 is 50 (milliSeconds). With this value 
the morse speed will be 19 words per minute (wpm). The value of dly1 is derived from the
setting of the global variable: tone_dot.time_ms as: ```dly1 = tone_dot.time_ms / 2```.
And the global variable: ```tone_dash.time_ms``` is derived from the value of ```tone_dot.time_ms``` as:
```tone_dash.time_ms = 3 * tone_dot.time_ms```. (see the function: ```set_speed()```)


```

Docs:

```
text files containing Arduino Monitor Output texts.


Images: 

Images, are in the folder: ```images```.


Links to product pages of the hardware used:

- M5Stack M5Echo [info](https://shop.m5stack.com/products/atom-echo-smart-speaker-dev-kit?variant=34577853415588);
- M5Stack Dualbutton unit [info](https://shop.m5stack.com/products/mini-dual-button-unit);
- iLabs cPico (RP2350A) [info](https://ilabs.se/product/cpico-rp2350/);


Known Issues:

The one device setup runs OK. ToDo: investigate how the command impulses can changes in such a way that the
resulting audio of the morse code does appear less "staccato" (rigid).
In the two devices setup, the morse code speed control by use of the red and blue button on the M5Stack Dualbutton unit
is in an initial state of development;
The performance of the MicroPython sketch pio-morse_v3.py needs work to make it better, 
eventually with changes in the Arduino sketch running on the M5Echo device.
In the morse code test version I experienced that setting ```tone_dot.time_ms``` to uneven decimal values, for example: 90, 110, 130,
caused the tone for the dash to sound garbled. Also the tone of the ```push button``` on the M5Echo sounded garbled alike. That is why I
created an global array of integer values: ```const int tone_time_lst[] = {10, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200};``` which 
will be used in function ```set_speed()``` to change the value of  ```tone_dot.time_ms```, 
from which are derived the values of other global variables (see the explanation about morse speed test above).


