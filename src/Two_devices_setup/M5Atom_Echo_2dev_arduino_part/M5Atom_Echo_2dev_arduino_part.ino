/*
*******************************************************************************
* Initial copyright (c) 2021 by M5Stack
* 初始版權 (c) 2021 歸 M5Stack 所有
*                  Equipped with Atom-Lite sample source code
*                          配套  Atom-Lite 示例源代码
* Visit for more information: https://docs.m5stack.com/en/atom/atom_spk
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/atom/atom_spk
*
* Product:  SPK.
* Date: 2021/9/1
*
* Sketch: M5Atom_Echo_2dev_arduino_part.ino
* Ported to a M5Stack Atom Echo by @PaulskPt (Github).
* Date: 2024/10/7
* Using example C++ code originally designed for the M5Stack AtomSPK
* ported this to the M5Stack Atom Echo
* Changed AtomSPK.h and AtomSPK.cpp into AtomEchoSPKR.h and AtomEchoSPKR.cpp
*
* New describe: Speaker morse code beep example using ATOMECHOSPKR class
*               To play beeps on command from an iLabs cPico (RP2350) device
*               using the M5Echo GROVE Port.
*
* IDE: Arduino v2.3.4. Inside the Arduino IDE > Tools > Board chosen: M5Atom (M5Stack).
***************************************************************************************
* Example Serial monitor output:
* ----------------------------------------------------------------
* loop(): entering the while loop...
* loop(): ESP.getFreeHeap = 310904
* send_morse(): Starting...
* fq = 3000 Hz
* text to send = "cq cq cq de ct7agr ct7agr ct7agr k ", length = 35
* all characters sent
* Units sent = 346
* ----------------------------------------------------------------
*/

#include <M5Atom.h>
#include "AtomEchoSPKR.h"
#include <time.h>

bool my_debug = false;

// Sketch to receive morse dot and dash beep commands
// from an external microcontroller, 
// in my case an iLabs cPico (RP2350) microcontroller board

// 4-PIN connector type HY2.0-4P
#define GROVE_PIN1 26 // Define the first pin of Port B (Yellow wire)
#define GROVE_PIN2 32 // Define the second pin of Port B (White wire)

int debounce_delay = 500; // mSec

bool dot_rcvd = false;
bool dash_rcvd = false;

ATOMECHOSPKR echoSPKR;

// Define two beep tones
beep tone_dot = 
{
  .freq    = 1200,
  .time_ms = 100,
  .maxval  = 10000,
  .modal   = true
};

beep tone_dash = 
{
  .freq    = 1200,
  .time_ms = 300,
  .maxval  = 10000,
  .modal   = true
};

// Read the signal on pin G26
void dot_cb() {
  dot_rcvd = true;
}

// Read the signal on G32
void dash_cb() {
  dash_rcvd = true;
} 

void setup()
{
  static constexpr const char txt0[] PROGMEM = "setup(): ";

  // Pin settings to receive commands from an iLabs cPico, 
  // commands to start a morse code dot beep or a morse code dash beep.
  pinMode(GROVE_PIN1, INPUT_PULLUP); // Set the first pin of GROVE Port as input (Yellow)
  pinMode(GROVE_PIN2, INPUT_PULLUP); // Set the second pin of GROVE B as input (White)
  // mode: LOW, HIGH, CHANGE, RISING or FALLING
  // Interrupt definitions:
  attachInterrupt(digitalPinToInterrupt(GROVE_PIN1), dot_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(GROVE_PIN2), dash_cb, FALLING);

  // Note that Serial.begin() is started by M5.begin()
  M5.begin(true, false, false); // Init M5 Atom Echo  (SerialEnable, I2CEnable, DisplayEnable)

  Serial.println(F("M5Stack M5Atom Echo new \"ATOMECHOSPKR class\" morse test"));

  esp_err_t err = ESP_OK;

  try {
    //err = echoSPKR.begin(88200); // standard rate = 44100
    err = echoSPKR.begin(); // standard rate = 44100
  }
  catch (esp_err_t err) {
    Serial.print(txt0);
    Serial.print(F("While starting echoSPKR, error: "));
    Serial.print(err);
    Serial.println(F("occurred. Restarting in 5 seconds..."));
    delay(5000);  // wait 5 seconds
    ESP.restart();
  }
  // delay(100);
}

void loop() {
  static constexpr const char txt0[] PROGMEM = "loop(): ";
  delay(5000); // Wait for user to clear the serial monitor window
  Serial.print('\n');
  Serial.print(txt0);
  Serial.println(F("entering the while loop..."));

  while (true) {
    if (dot_rcvd) {
      if (0 == digitalRead(GROVE_PIN1))
        echoSPKR.playBeep(tone_dot);
      dot_rcvd = false;
    }
    if (dash_rcvd) {
      if (0 == digitalRead(GROVE_PIN2))
        echoSPKR.playBeep(tone_dash);
      dash_rcvd = false;
    }
  }
}
