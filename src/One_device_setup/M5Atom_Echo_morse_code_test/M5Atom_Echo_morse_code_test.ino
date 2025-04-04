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
* Ported to a M5Stack Atom Echo by @PaulskPt (Github).
* Date: 2024/10/7
* Using example C++ code originally designed for the M5Stack AtomSPK
* ported this to the M5Stack Atom Echo
* Changed AtomSPK.h and AtomSPK.cpp into AtomEchoSPKR.h and AtomEchoSPKR.cpp
*
* New description: Speaker playing morse code example using ATOMECHOSPKR class
*
* Update: 2025-03-25 added code to send a text in morse code.
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
* And when one of the two buttons on the M5Stack Dualbutton unit
* is pressed during the sending of a morse code text:
* send_morse(): button blue pressed. Exiting function
* ----------------------------------------------------------------
*/

#include <M5Atom.h>
#include <FastLED.h>
#include "AtomEchoSPKR.h"
#include <time.h>

#include <unordered_map>
#include <vector>
#include <iostream>
#include <string.h>

bool my_debug = false;

#define NUM_LEDS 1
#define ATOMECHO_LED_PIN 27


// Activate USE_DUALBTN if you want to use a M5Stack Dualbutton unit
// to increase/decrease the speed of the morse (function in development)
// and to stop an active sending a morse text to the speaker.
#define USE_DUALBTN 

#ifdef USE_DUALBTN
#define GROVE_PIN1 26 // Define the first pin of Port B (Yellow wire)
#define GROVE_PIN2 32 // Define the second pin of Port B (White wire)
#endif

int debounce_delay = 500; // mSec

CRGB leds[NUM_LEDS];

enum my_colors {RED=0, GREEN, BLUE, WHITE, BLACK};

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


// First tone for M5 Echo button press
beep btn_tone1 = 
{
  .freq    = 1200,
  .time_ms = 100,
  .maxval  = 10000,
  .modal   = false
};

// Second tone for M5 Echo button press
beep btn_tone2 = 
{
  .freq    = 1400,
  .time_ms = 100,
  .maxval  = 10000,
  .modal   = false
};

// Define the morse code, dot-dash sequences for 
// the characters that are in the text string to send in morse code
// in the array {2,1}, the 1 represents a dot, the 2 a dash.
std::unordered_map<char, std::vector<int>> morse_txt_dict = {
  {'7',{2,2,1,1,1}},
  {'a', {1,2}},
  {'c', {2, 1, 2, 1}},
  {'d', {2, 1, 1}},
  {'e', {1}},
  {'g', {2,2,1}},
  {'k', {2, 1, 2}},
  {'q', {2, 2, 1, 2}},
  {'r', {1,2,1}},
  {'t', {2}}
};

void dot_dash_time() {
  static constexpr const char txt0[] PROGMEM = "dot_dash_time(): ";
  static constexpr const char txt1[] PROGMEM = "set to: ";
  size_t length = strlen(txt0); // Get the length of txt0
  char* spaces = new char[length + 1]; // +1 for null terminator
  memset(spaces, ' ', length); // No 'std::' prefix needed
  spaces[length] = '\0'; // Add null terminator
  Serial.print(txt0);
  Serial.print(F("tone_dot.time_ms  "));
  Serial.print(txt1);
  Serial.print(tone_dot.time_ms);
  Serial.println(F(" mSeconds"));
  Serial.print(spaces);
  Serial.print(F("tone_dash.time_ms "));
  Serial.print(txt1);
  Serial.print(tone_dash.time_ms);
  Serial.println(F(" mSeconds"));

  delete[] spaces; // Free memory when done
}

void set_speed(int speedChg) {
  static constexpr const char txt0[] PROGMEM = "set_speed(): ";
  
  Serial.print(txt0);
  Serial.printf("param speedChg = %d\n", speedChg);
  //if (speedChg == NULL)
  //  return;
  if (speedChg <= 0)
    speedChg = 0;
  if (speedChg > 100)
    speedChg = 100;
  tone_dot.time_ms = 100 + speedChg;
  if (tone_dot.time_ms < 100)
    tone_dot.time_ms = 100;
  if (tone_dot.time_ms > 10000)
    tone_dot.time_ms = 10000;
  
  tone_dash.time_ms = 3 * tone_dot.time_ms; // keep ratio 3 : 1

  dot_dash_time();
}

void LedColor(my_colors color)
{
  switch (color)
  {
    case RED:
      leds[0] = CRGB::Red; // Set the LED to red
      break;
    case GREEN:
      leds[0] = CRGB::Green; // Set the LED to green
      break;
    case BLUE:
      leds[0] = CRGB::Blue; // Set the LED to blue
      break;
    case WHITE:
      leds[0] = CRGB::White; // Set the LED to white
      break;
  case BLACK:
      leds[0] = CRGB::Black; // Set the LED to black
      break;
  default:
      leds[0] = CRGB::Black; // Set the LED to black
      break;
  };
    FastLED.show(); // Update the LED
}

void btn_beep() {
  LedColor(GREEN);
  echoSPKR.playBeep(btn_tone1);
  //echoSPKR.playBeep(btn_tone1.freq, btn_tone1.time_ms, btn_tone1.maxval, btn_tone1.modal);
  delay(100);
  echoSPKR.playBeep(btn_tone2);
  //echoSPKR.playBeep(btn_tone2.freq, btn_tone2.time_ms, btn_tone2.maxval, btn_tone2.modal);
  delay(100);
  LedColor(RED);
}

// Dot interrupt callback function
// Read the signal on pin G26
void dot_cb() {
  dot_rcvd = true;
}

// Dash interrupt callback function
// Read the signal on G32
void dash_cb() {
  dash_rcvd = true;
} 

/* 
   The unit of morse code speed is the number of times
   the word "paris" is sent in morse code per minute.
   A speed of morse code of 16 words per minute is equal to
   sending the word "paris" 16 times per minute.

   Morse code counting:
   +-----------------+--------+
   | what:           | units: |
   +-----------------+--------+
   | dot             |    1   |
   +-----------------+--------+
   | dot-dash space  |    1   |
   +-----------------+--------+
   | dash            |    3   |
   +-----------------+--------+
   | character space |    3   |
   +-----------------+--------+
   | word space      |    7   |
   +---------------.-+--------+

   The length of the word "paris" in morse code units (see table above) is: 
           p        /   /   a   /   /     r     /   /  i  /   /    s    /
   .   ---   ---   ./~~~/.   ---/~~~/.   ---   ./~~~/.   ./~~~/.   .   ./  
   =       11       / 3 /   5   / 3 /     7     / 3 /  3  / 3 /    5    /  = 43 units 
   1,1,123,1,223,1,2/123/1,1,123/123/1,1,123,1,2/123/1,1,2/123/1,1,2,1,3/
*/
void send_morse() {
  // Define variables:
  static constexpr const char txt0[] PROGMEM = "send_morse(): ";
  std::vector<int> lst; // create an empty list
  int i;
  int j;
  int le;
  int le2;
  int n;
  int n2;
  char c;
  char c2;
  size_t writeSize;
  bool my_morse_debug = true;
  bool word_end = false;
  int unit_count = 0;
  int delay_mult = 16;
  const uint8_t dly1 = 100 * delay_mult; // unit delay
  const uint8_t dly2 = 200 * delay_mult; // longer unit delay (for test)
  const uint8_t dly3 = 300 * delay_mult; // character delay
  const uint8_t dly7 = 700 * delay_mult; // word delay
  const unsigned long fq = 3000;
  int btn_red = 0;
  int btn_blu = 0;
  Serial.print(txt0);
  Serial.println(F("Starting..."));
  Serial.printf("fq = %lu Hz\n", fq);
  echoSPKR.setVolume(1);  // Initial volume (in class) set to 8.
  dot_dash_time();
  const char txt[] = "cq cq cq de ct7agr ct7agr ct7agr k ";
  le = strlen(txt);
  Serial.print(F("Text to send = "));
  Serial.printf("\"%s\"", txt);
  Serial.print(F(", length = "));
  Serial.printf("%d\n", le);
  if (my_debug) {
    Serial.println(F("Going to send the text now..."));
    Serial.println(F("In the lists like: [2,1], 2 represents a dash, 1 respresents a dot"));
  }

  for (i = 0; txt[i] != '\0'; i++) {
#ifdef USE_DUALBTN
    btn_red = digitalRead(GROVE_PIN1);  // read the value of BUTTON. 读取33号引脚的值
    btn_blu = digitalRead(GROVE_PIN2);
    if (btn_red == 0 || btn_blu == 0) {
      Serial.print(txt0);
      if (btn_red == 0)
        Serial.print(F("button red "));
      else if (btn_blu == 0)
        Serial.print(F("button blue "));
      Serial.println(F("pressed. Exiting function"));
      if (btn_red == 0) {
        // Debounce wait
        while (btn_red == 0) {
          btn_red = digitalRead(GROVE_PIN1);
          delay(debounce_delay);
        }
      }
      if (btn_blu == 0) {
        // Debounce wait
        while (btn_blu == 0) {
          btn_blu = digitalRead(GROVE_PIN1);
          delay(debounce_delay);
        }
      }
      return;
    }
#endif
    c = txt[i];
    // check if the next character is a space.
    // If so, set the word_end flag. We must not make a character_space_delay
    // behind the current character but a word_space_delay
    if (i+1 < le && static_cast<int>(txt[i+1]) == 32) 
      word_end = true;
    n = static_cast<int>(c); // Get the ASCII value
    if (n == 32) {
      if (my_debug)
        Serial.println(F("word space delay"));
      if (my_morse_debug)
        Serial.print("|  7  |\n");
      delay(dly7);
      unit_count += 7;
    } else {
      if (n >= 65 and n <= 90)  // Convert upper case to lower case
        n2 = n + 32;    // python example: ord('A') = 65. ord('a') = 97
      else
        n2 = n;
      c2 = static_cast<char>(n2);  // convert back to ASCII
      if (morse_txt_dict.find(c2) != morse_txt_dict.end()) {
        // Extract the list and assign it to a variable
        lst = morse_txt_dict[c2];
        if (my_debug) {
          Serial.print(F("Sending character "));
          Serial.print(c2);
          Serial.print(F(" = "));
        }
        // Calculate the length of the extracted list
        size_t le2 = lst.size();
        if (le2 > 0) {
          if (my_debug) {
            Serial.print("[");
            for (int n = 0; n < le2; n++) {
              Serial.print(lst[n]);
              if (n < le2 -1) {
                Serial.print(", ");
              }
            }
            Serial.println("]");
          }
          for (j = 0; j < le2; j++) {
            if (lst[j] == 2) {
              if (my_debug)
                Serial.println(F("Sending a dash"));
              if (my_morse_debug)
                Serial.print("---");
              //Serial.printf("tone_dash.modal = %d\n", tone_dash.modal);
              unit_count += 3;
              writeSize = echoSPKR.playBeep(tone_dash);
              //writeSize = echoSPKR.playBeep(tone_dash.freq, tone_dash.time_ms, tone_dash.maxval, tone_dash.modal);
            } else if (lst[j] == 1) {
              if (my_debug)
                Serial.println(F("Sending a dot"));
              if (my_morse_debug)
                Serial.print(".");
              //Serial.printf("tone_dot.modal = %d\n", tone_dot.modal);
              unit_count += 1;
              writeSize = echoSPKR.playBeep(tone_dot);
              //echoSPKR.playBeep(tone_dot.freq, tone_dot.time_ms, tone_dot.maxval, tone_dot.modal);
            }
            // if (writeSize > 0) {
            //   Serial.print(F("writeSize = "));
            //   Serial.println(writeSize);
            // }
            // for example: after 'e' or 't' no character space delay
            // and not after the last dot or dash of the character
            if (le2 > 1 && j < le2-1) {
              if (my_debug)
                Serial.println(F("unit space delay"));
              if (my_morse_debug)
                Serial.print(" ");
              delay(dly1); // was: (dly1)
              unit_count += 1;
            }
          }
          if (!word_end) {
            if (my_debug)
              Serial.println(F("character space delay"));
            if (my_morse_debug)
              Serial.print("|3|");
            delay(dly3);
            unit_count += 3;
          } else {
              word_end = false;  // reset flag
          }
        }
      }
    }
  }
  Serial.println(); // New line after the last character (morse code representation)
  Serial.println(F("All characters sent"));
  Serial.print(F("Units sent = "));
  Serial.printf("%d\n", unit_count);
}

void setup()
{
  static constexpr const char txt0[] PROGMEM = "setup(): ";

#ifdef USE_DUALBTN 
  pinMode(GROVE_PIN1, INPUT); // Set the first pin of GROVE Port as input (Yellow)
  pinMode(GROVE_PIN2, INPUT); // Set the second pin of GROVE B as input (White)
#endif

  // Note that Serial.begin() is started by M5.begin()
  M5.begin(true, false, false); // Init M5 Atom Echo  (SerialEnable, I2CEnable, DisplayEnable)

  Serial.println(F("M5Stack M5Atom Echo new \"ATOMECHOSPKR class\" beep test"));

  FastLED.addLeds<NEOPIXEL, ATOMECHO_LED_PIN>(leds, NUM_LEDS); // Initialize FastLED
  FastLED.setBrightness(50); // Set brightness
  LedColor(RED);

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
  
  bool start = true;
#ifdef USE_DUALBTN
  int speed = 10;
  int speed_step = 10;
  int speed_max = 10000;
  int blu_value= 0, blu_last_value = 0;
  int red_value = 0, red_last_value = 0;
#endif

  delay(5000); // Wait for user to clear the serial monitor window
  Serial.print('\n');
  Serial.print(txt0);
  Serial.println(F("entering the while loop..."));
    Serial.print(txt0);
  Serial.print(F("ESP.getFreeHeap = "));
  Serial.println(ESP.getFreeHeap());

  while (true) {
  
    #ifndef USE_ILABS_CPICO
    if (start) {
      start = false;
      send_morse();
    }
  #endif

#ifdef USE_DUALBTN
    red_value = digitalRead(GROVE_PIN1);  // read the value of BUTTON. 读取33号引脚的值
    blu_value = digitalRead(GROVE_PIN2);
    
    if (red_value != red_last_value || blu_value != blu_last_value) { 
      if (my_debug) {
        Serial.print(txt0);
        Serial.printf("red_value = %d, blu_value = %d\n", red_value, blu_value);
      }
    }
    if (red_value != red_last_value) {
      if (red_value == 0) {
        Serial.print(txt0);
        Serial.println(F("dualbutton red pressed"));
        // Debounce wait
        while (red_value == 0) {
          red_value = digitalRead(GROVE_PIN1);
          delay(debounce_delay);
        }
        speed += speed_step;
        if (speed > speed_max)
          speed = speed_max;
        set_speed(speed);
        red_value = -1;
      }
      red_last_value = red_value;
    }
    if (blu_value != blu_last_value) {
      if (blu_value == 0) {
        Serial.print(txt0);
        Serial.println(F("dualbutton blue pressed"));
        // Debounce wait
        while (blu_value == 0) {
          blu_value = digitalRead(GROVE_PIN1);
          delay(debounce_delay);
        }
        speed -= speed_step;
        if (speed < 0)
          speed = 0;
        set_speed(speed);
        blu_value = -1;
      }
      blu_last_value = blu_value;
    }
#endif
    if (M5.Btn.wasPressed()) {
      Serial.print(txt0);
      Serial.println(F("Button was pressed"));
      btn_beep();
      delay(1000);
      send_morse();
    }
    M5.update();
    LedColor(RED);
  }
}
