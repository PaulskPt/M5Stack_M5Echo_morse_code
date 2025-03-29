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
* Update: 2025-03-26 morse code speed test. 
*    Repeating to send the word "paris" in morse code during one minute.
*    At the end counting the number of words sent and displaying the speed
*    of the morse code sent.
*
* IDE: Arduino v2.3.4. Inside the Arduino IDE > Tools > Board chosen: M5Atom (M5Stack).
***************************************************************************************
*
* +--------------------------------------------------------------------+
* | Note: You can interrupt the sending of morse code                  |
* | by pressing one of the two buttons on the M5Stack Dualbutton unit. |
* | The sketch then will print (in case the blue button was pressed):  |
* | send_morse(): button blue pressed. Exiting function                |
* +--------------------------------------------------------------------+
*/

#include <Arduino.h>
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

// In function send_morse() print unit space indicator "|1|" 
// between the dots and dashes.
// If you don't want the unit space indicator printed,
// comment-out the next line.
#define SHOW_UNITS  

// Activate USE_DUALBTN if you want to use a M5Stack Dualbutton unit
// to increase/decrease the speed of the morse (function in development)
// and to stop an active sending a morse text to the speaker.
#define USE_DUALBTN 

#ifdef USE_DUALBTN
#define GROVE_PIN1 26 // Define the first  pin of Port B (Yellow wire)
#define GROVE_PIN2 32 // Define the second pin of Port B (White wire)
#endif

// Uncomment this when you want to view the audio timings on an oscilloscope
// However, when using USE_AUDIO_VIA_PORT_B, comment-out USE_DUALBTN above.
// #define USE_AUDIO_VIA_PORT_B

#ifdef USE_AUDIO_VIA_PORT_B
#define GROVE_PIN1 26 // Define the first  pin of Port B (Yellow wire)
#define GROVE_PIN2 32 // Define the second pin of Port B (White wire)
#endif

CRGB leds[NUM_LEDS];

enum my_colors {RED=0, GREEN, BLUE, WHITE, BLACK};

unsigned long start_t = millis();

ATOMECHOSPKR echoSPKR;  // Create an instance of the ATOMECHOSPKR class
// +-------+-----------+-------------+
// | var   |  milli-   | morse speed |
// |       |  sonds    | (wpm)       |
// +-------+-----------+-------------+
// | dly1  |    20     |    54       |
// +-------+-----------+-------------+
// | dly1  |    30     |    35       |
// +-------+-----------+-------------+
// | dly1  |    40     |    26       |
// +-------+-----------+-------------+
// | dly1  |    50     |    19       |
// +-------+-----------+-------------+
// | dly1  |    60     |    17       |
// +-------+-----------+-------------+
// | dly1  |    70     |    15       |
// +-------+-----------+-------------+
// | dly1  |    80     |    13       |
// +-------+-----------+-------------+
// | dly1  |    90     |    12       |
// +-------+-----------+-------------+
// | dly1  |   100     |    10       |
// +-------+-----------+-------------+
uint16_t dly1 = 50;       // unit delay
uint16_t dly3 = dly1 * 3; // character delay
uint16_t dly7 = dly1 * 7; // word delay

int debounce_delay = 100; // mSec

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
// Example: in the array {1,2}, the 1 represents a dot, the 2 a dash.
std::unordered_map<char, std::vector<int>> morse_txt_dict = {
  {'a', {1,2}},
  {'i', {1,1}},
  {'p', {1,2,2,1}},
  {'r', {1,2,1}},
  {'s', {1,1,1}},
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
  Serial.printf("%3d", tone_dot.time_ms);
  Serial.println(F(" mSeconds"));
  Serial.print(spaces);
  Serial.print(F("tone_dash.time_ms "));
  Serial.print(txt1);
  Serial.printf("%3d", tone_dash.time_ms);
  Serial.println(F(" mSeconds"));

  delete[] spaces; // Free memory when done
}

void show_delays() {
  Serial.println(F("Values for dly1, dly3 and dly7:"));
  Serial.printf("dly1: %d, dly3: %d, dly7: %d mSeconds\n", dly1, dly3, dly7);
}

const int tone_time_lst[] = {40, 60, 80, 100, 120, 140, 160, 180, 200};
int le_tone_time_lst = sizeof(tone_time_lst)/sizeof(tone_time_lst[0]);
int tone_time_lst_idx = 4; // index to 100

#ifdef USE_DUALBTN
int blu_last_value, red_last_value = 0;

void set_speed() {
  static constexpr const char txt0[] PROGMEM = "set_speed(): ";
  const char *txts[] PROGMEM = {
    "dualbutton",                      // 0
    "released",                        // 1
    "pressed",                         // 2
    " red ",                           // 3
    " blue ",                          // 4
    "tone_time_list_idx changed to: ", // 5
    "speed has been reached. "         // 6
  };
  
  int blu_value= 0;
  int red_value = 0;
  int tone_dly1 = 100;
  int unit_dly1 = 50;
  bool btn_blu_pressed = false;
  bool btn_red_pressed = false;
  red_value = digitalRead(GROVE_PIN1);  // read the value of the RED BUTTON (Pin 26). 读取26号引脚的值
  blu_value = digitalRead(GROVE_PIN2);  // read the value of the BLUE BUTTON (Pin 32). 读取32号引脚的值

  if (red_value == 1 && blu_value == 1)
    return;  // No button pressed

  btn_red_pressed = red_value == 0 ? true : false;
  btn_blu_pressed = blu_value == 0 ? true : false;

  if (btn_red_pressed) {
    Serial.print(txt0);
    Serial.print(txts[0]);
    Serial.print(txts[3]);
    Serial.println(txts[2]);
  }
  else if (btn_blu_pressed) {
    Serial.print(txt0);
    Serial.print(txts[0]);
    Serial.print(txts[4]);
    Serial.println(txts[2]);
  }

  // Debounce wait
  if (btn_red_pressed) {
    while (red_value == 0) {
      red_value = digitalRead(GROVE_PIN1);
      delay(debounce_delay);
    }
    Serial.print(txt0);
    Serial.print(txts[0]);
    Serial.print(txts[3]);
    Serial.print(txts[1]);
    Serial.printf("? %s\n", (red_value == 1) ? "Yes" : "No ");
  }
  else if (btn_blu_pressed) {
    // Debounce wait
    while (blu_value == 0) {
      blu_value = digitalRead(GROVE_PIN2);
      delay(debounce_delay);
    }
    Serial.print(txt0);
    Serial.print(txts[0]);
    Serial.print(txts[4]);
    Serial.print(txts[1]);
    Serial.printf("? %s\n", (blu_value == 1) ? "Yes" : "No ");
  }
  tone_dly1 = tone_dot.time_ms;
  unit_dly1 = dly1;

  if (btn_red_pressed) {
    tone_time_lst_idx += 1;
    if (tone_time_lst_idx > le_tone_time_lst-1) {
      tone_time_lst_idx = le_tone_time_lst -1;
      Serial.print(txt0);
      Serial.print(F("maximum "));
      Serial.print(txts[6]);
      if (my_debug) {
        Serial.printf("tone_time_lst_idx = %d, le_tone_time_lst-1 = %d, tone_time_lst[%d] = %d\n", 
          tone_time_lst_idx, le_tone_time_lst-1, tone_time_lst_idx, tone_time_lst[tone_time_lst_idx]);
      } else {
        Serial.println();
      }
      Serial.print(txt0);
      Serial.print(txts[5]);
      Serial.printf("%d\n", tone_time_lst_idx);
    }
    tone_dly1 = tone_time_lst[tone_time_lst_idx];
    unit_dly1  = tone_dly1/2;
    red_value = -1;
    red_last_value = (btn_red_pressed == true) ? 0 : 1;
  }
  else if (btn_blu_pressed) {
    tone_time_lst_idx -= 1;
    if (tone_time_lst_idx <= 0) {
      tone_time_lst_idx = 0;
      Serial.print(txt0);
      Serial.print(F("minimum "));
      Serial.print(txts[6]);
      if (my_debug) {
        Serial.printf("tone_time_lst_idx = %d, le_tone_time_lst-1 = %d, tone_time_lst[%d] = %d\n", 
          tone_time_lst_idx, le_tone_time_lst-1, tone_time_lst_idx, tone_time_lst[tone_time_lst_idx]);
      } else {
        Serial.println();
      }
      Serial.print(txt0);
      Serial.print(txts[5]);
      Serial.printf("%d\n", tone_time_lst_idx);
    }
    tone_dly1 = tone_time_lst[tone_time_lst_idx];
    unit_dly1 = tone_dly1/2;
    blu_value = -1;
    blu_last_value = (btn_blu_pressed == true) ? 0 : 1;
  }

  tone_dot.time_ms = tone_dly1;
  tone_dash.time_ms = 3 * tone_dot.time_ms; // keep ratio 3 : 1

  dly1 = unit_dly1;
  dly3 = dly1 * 3;
  dly7 = dly1 * 7;
  
  dot_dash_time();

  Serial.print(txt0);
  show_delays();
}
#endif

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
         0             1               2               3               4             5
         1 2 345 6 789 0 1 234 5 6 789 012 3 4 567 8 9 012 2 4 5 678 9 0 1 2 3 4567890 
                 p        /   /   a   /   /     r     /   /  i  /   /    s    /       /
         ./~/---/~/---/~/./~~~/./~/---/~~~/./~/---/~/./~~~/./~/./~~~/./~/./~/./~~~~~~~/  
                 11       / 3 /   5   / 3 /     7     / 3 /  3  / 3 /    5    /   7   /  = 50 units 
         1,1,123,1,223,1,2/123/1,1,123/123/1,1,123,1,2/123/1,1,2/123/1,1,2,1,3/1234567/
      ____/\________             ______/\_______                             ____/\____
      dot/dash space             character space                             word space
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
  bool stop = false;
  int unit_count = 0;
  int word_count = 0;
  unsigned long curr_t = 0;
  unsigned long interval_t = 60 * 1000;
  unsigned long elapsed_t = 0;
  int btn_red = 0;
  int btn_blu = 0;
  Serial.print(txt0);
  Serial.println(F("Starting..."));
  Serial.print(F("tone dot (and dash) frequency = "));
  Serial.printf("%d Hz\n", tone_dot.freq);
  Serial.printf("tone_dot.modal  = %s\n", tone_dot.modal  == 1 ? "true": "false");
  Serial.printf("tone_dash.modal = %s\n", tone_dash.modal == 1 ? "true": "false");
  echoSPKR.setVolume(1);  // Initial volume (in class) set to 8.
  dot_dash_time();
  const char txt[] = "paris";
  le = strlen(txt);
  Serial.print(F("Text to send = "));
  Serial.printf("\"%s\"", txt);
  Serial.print(F(", length = "));
  Serial.printf("%d\n", le);
  if (my_debug) {
    Serial.println(F("Going to send the text now..."));
    Serial.println(F("In the lists like: [2,1], 2 represents a dash, 1 respresents a dot"));
  }
  show_delays();
  
  while (true) {
    curr_t = millis();
    elapsed_t = curr_t - start_t;
    if (elapsed_t >= interval_t) {
      stop = true;
      Serial.print("\nOne minute limit reached.\n");
      break;
    }
    if (strlen(txt) == 0)
      return;
    Serial.printf("%2d) ", word_count+1);
    for (i = 0; txt[i] != '\0'; i++) {
#ifdef USE_DUALBTN
      btn_red = digitalRead(GROVE_PIN1);  // read the value of BUTTON. 读取33号引脚的值
      btn_blu = digitalRead(GROVE_PIN2);
      if (btn_red == 0 || btn_blu == 0) {
        Serial.println();
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
      n = static_cast<int>(c); // Calculate the ASCII value
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
        if (morse_txt_dict.find(c2) == morse_txt_dict.end()) {
          Serial.printf("character \'%s\' not found in morse_txt_dict\n", n2);
        } else {
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
#ifdef USE_AUDIO_VIA_PORT_B
                digitalWrite(GROVE_PIN1, HIGH);
#endif
                writeSize = echoSPKR.playBeep(tone_dash);
                //writeSize = echoSPKR.playBeep(tone_dash.freq, tone_dash.time_ms, tone_dash.maxval, tone_dash.modal);
              } else if (lst[j] == 1) {
                if (my_debug)
                  Serial.println(F("Sending a dot"));
                if (my_morse_debug)
                  Serial.print(".");
                //Serial.printf("tone_dot.modal = %d\n", tone_dot.modal);
                unit_count += 1;
#ifdef USE_AUDIO_VIA_PORT_B
                digitalWrite(GROVE_PIN1, HIGH);
#endif
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
#ifdef SHOW_UNITS
                  Serial.print("|1|");
#else
                  Serial.print(" ");
#endif
                delay(dly1); // was: (dly1)
#ifdef USE_AUDIO_VIA_PORT_B
                digitalWrite(GROVE_PIN1, LOW);
#endif
                unit_count += 1;
              }
            }
            if (i == le -1) {
              if (my_debug)
                Serial.println(F("word space delay"));
              if (my_morse_debug)
                Serial.print("|   7   |");
              delay(dly7);
#ifdef USE_AUDIO_VIA_PORT_B
              digitalWrite(GROVE_PIN1, LOW);
#endif
              unit_count += 7;
            } else {
              if (my_debug)
                Serial.println(F("character space delay"));
              if (my_morse_debug)
                Serial.print("| 3 |");
              delay(dly3);
#ifdef USE_AUDIO_VIA_PORT_B
              digitalWrite(GROVE_PIN1, LOW);
#endif
              unit_count += 3;
            }
          }
        }
      }
    } // end-of-for-loop
    Serial.println(); // New line after the last character (morse code representation)
    if (my_debug) {
      Serial.println(F("All characters sent"));
      Serial.print(F("Units sent = "));
      Serial.printf("%d\n", unit_count);
    }
    word_count += 1;
  }
  Serial.print(F("Number of units sent = "));
  Serial.printf("%d, ", unit_count);
  Serial.print(F("this is "));
  Serial.printf("%d ", static_cast<int>(unit_count/word_count)); 
  Serial.println(F("units per word."));
  Serial.print(F("Number of words sent: "));
  Serial.print(word_count);
  Serial.println(".");
  Serial.print(F("This equals to a morse code speed of "));
  Serial.printf("%d ", word_count);
  Serial.println(F("words/minute."));
}

void setup()
{
  static constexpr const char txt0[] PROGMEM = "setup(): ";

#ifdef USE_DUALBTN 
  pinMode(GROVE_PIN1, INPUT); // Set the first pin of GROVE Port as input (Yellow)
  pinMode(GROVE_PIN2, INPUT); // Set the second pin of GROVE B as input (White)
#endif

#ifdef USE_AUDIO_VIA_PORT_B
  pinMode(GROVE_PIN1, OUTPUT);
  digitalWrite(GROVE_PIN1, LOW);
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

  delay(5000); // Wait for user to clear the serial monitor window
  Serial.print('\n');
  Serial.print(txt0);
  Serial.println(F("entering the while loop..."));
  Serial.print(txt0);
  Serial.print(F("ESP.getFreeHeap = "));
  Serial.println(ESP.getFreeHeap());

  while (true) {
  
    if (start) {
      start = false;
      send_morse();
    }

#ifdef USE_DUALBTN
    set_speed();
#endif

    if (M5.Btn.wasPressed()) {
      Serial.print(txt0);
      Serial.println(F("Button was pressed"));
      btn_beep();
      delay(1000);
      start_t = millis();
      send_morse();
    }
    M5.update();
    LedColor(RED);
  }
}
