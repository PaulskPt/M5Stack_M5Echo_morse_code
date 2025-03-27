# pio_morse_v3.py
# Trial to creat morse code with using Pico PIO peripherals
# 2025-03-20 by @PaulskPt

import time
from rp2 import PIO, StateMachine, asm_pio
from machine import Pin

@asm_pio(set_init=PIO.OUT_LOW)
def led_off():
    set(pins, 0)
    
@asm_pio(set_init=PIO.OUT_LOW)
def dot_sound_on():
    set(pins, 1)    [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    set(pins, 0)    [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]

    
@asm_pio(set_init=PIO.OUT_LOW)
def dash_sound_on():
    set(pins, 1)    [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    set(pins, 0)    [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]
    nop()           [31]

@asm_pio(set_init=PIO.OUT_HIGH)
def space():
    set(pins, 0)   [31]  # 2 x 32
    nop()          [31]
    #set(pins, 0)   [31]
    

fq = 3000

sm0      = StateMachine(0, led_off,  freq=fq, set_base=Pin(25))

dot   = StateMachine(1, dot_sound_on,  freq=fq, set_base=Pin(26))
dash  = StateMachine(2, dash_sound_on,  freq=fq, set_base=Pin(27))

dly1 = 0.1 # This causes a single beep in the connected M5Stack Echo device
dly3 = 0.3
dly7 = 0.7

morse_txt_dict = {
  '7': [2,2,1,1,1],
  'a': [1,2],
  'c': [2,1,2,1],
  'd': [2,1,1],
  'e': [1],
  'g': [2,2,1],
  'k': [2,1,2],
  'q': [2,2,1,2],
  'r': [1,2,1],
  't': [2]
}

def unit_space_delay():
    # 1 count
    time.sleep(dly1) # 100 mSeconds
    
def char_space_delay():
    # 3 counts
    time.sleep(dly3) # 300 mSeconds

def word_space_delay():
    # 7 counts
    time.sleep(dly7) # 700 mSeconds

def main():
    word_end = False
    print("Starting...")
    print(f"fq = {fq} Hz")
    txt = "  cq cq cq de ct7agr ct7agr ct7agr k " # text to send
    print(f"text to send = \"{txt}\"")
    le = len(txt)
    lst = [] # empty list
    for i in range(le):
        c = txt[i]
        if len(c) == 0:
            break
        if i+1 < le and ord(txt[i+1]) == 32:
            word_end = True
        n = ord(c)
        if n == 32:
            # word_space_delay()
            time.sleep(dly7)
        else:
            if n >= 65 and n <= 90:  # Convert upper case to lower case
                n2 = n + 32    # example: ord('A') = 65. ord('a') = 97
            else:
                n2 = n
            c2 = chr(n2) # convert back to ASCII
            if c2 in morse_txt_dict.keys():
                lst = morse_txt_dict[c2]
                le2 = len(lst)
                if le2 > 0:
                    for j in range(le2):
                        if lst[j] == 2:
                            # Send a dash
                            dash.active(1)
                            time.sleep(dly1) 
                            dash.active(0)
                        elif lst[j] == 1:
                            # Send a dot
                            dot.active(1)
                            time.sleep(dly1)
                            dot.active(0)
                        time.sleep(dly1) # unit space delay()
                    if (j < le2-1):
                        if not word_end:
                            time.sleep(dly3) # word space delay
                        else:
                            word_end = False
                time.sleep(dly1) # unit space delay ?
    print("all characters sent")    

    
# if __name__ == "__main__()":
main()