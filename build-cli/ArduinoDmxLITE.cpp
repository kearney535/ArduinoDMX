#include <Arduino.h>
//#include "C:\Program Files (x86)\Arduino\libraries\Arduino-IRremote-master\IRremote.h"

#include <IRremote.h>

#include <TimerOne.h>

#include <DmxSimple.h>

// CONSTANTS

#define DMX_OUT_PIN 3
#define IR_PIN 4

uint8_t value = 0;
uint8_t IRvalue = 0;
uint16_t channel;

IRrecv irrecv(IR_PIN);
decode_results results;

/*
  COMMAND SYNTAX

 <number>c : Select a DMX channel
 <number>v : Set DMX channel to new value
 */

void setup() {
  // Initialize globals
  value = 0;

  // Initialize serial
  Serial.begin(115200);

  //Initialize DMX
  DmxSimple.usePin(3);

  irrecv.enableIRIn();
}

void loop() {
  int c;

  if (Serial.available()) {
    c = Serial.read();
    if ((c>='0') && (c<='9')) {
      value = 10*value + c - '0';
    }
    else {
      //    Serial.println((char)c);
      if (c=='c') channel = value;
      else if (c=='w') {
        if (channel > 0)
        {
          DmxSimple.write(channel, value);
        }
      }
      else if (c == 'q')
        Serial.print("71001");

      value = 0;
    }
  }

  if (irrecv.decode(&results)) {
    if (results.value != 0xFFFFFFFF)
    {
    	Serial.print("IR");
    	Serial.println(results.value, HEX);
    }

//    Serial.println("Resuming");
    irrecv.resume();
  }
}
