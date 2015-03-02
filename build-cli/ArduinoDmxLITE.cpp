#include <Arduino.h>
#include <IRremote.h>

#include <TimerOne.h>

#include <DmxSimple.h>

// CONSTANTS

#define DMX_OUT_PIN 3
#define IR_PIN 4
#define NUM_OF_CHANNELS 16

#define FADE_STEP 10
#define FADE_TIME 5

uint8_t fadeCounter = 0;

uint8_t value = 0;
uint8_t IRvalue = 0;
uint16_t channel;

float channel_values[NUM_OF_CHANNELS + 1];
float channel_steps[NUM_OF_CHANNELS + 1];

IRrecv irrecv(IR_PIN);
decode_results results;

void fade(uint16_t channel, uint8_t value);
void increment();

/*
  COMMAND SYNTAX

 <number>c : Select a DMX channel
 <number>v : Set DMX channel to new value
 */

void setup() {
  // Initialize globals
  value = 0;

  memset(channel_values, 0, sizeof(channel_values));
  memset(channel_steps, 0, sizeof(channel_steps));

  // Initialize serial
  Serial.begin(115200);

  //Initialize DMX
  DmxSimple.usePin(3);

  irrecv.enableIRIn();

  Timer1.initialize(1000 * FADE_STEP);
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
          fade(channel, value);
       //   DmxSimple.write(channel, value);
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

void fade(uint16_t channel, uint8_t value)
{
    fadeCounter = 0;
    channel_steps[channel] = ((float)value - (float)channel_values[channel])/(float)FADE_TIME;
    Serial.print(channel_steps[channel]);
    Timer1.attachInterrupt(increment);
}

void increment()
{
    for (uint8_t i = 0; i < NUM_OF_CHANNELS; i++)
    {
        if (channel_steps[i] != 0)
        {
            channel_values[i] += channel_steps[i];
            DmxSimple.write(i, (uint8_t)(channel_values[i]));
        }
    }
    fadeCounter++;
    if (fadeCounter == FADE_TIME)
    {
        Timer1.detachInterrupt();
        for (uint8_t i = 0; i < NUM_OF_CHANNELS; i++)
        {
            channel_steps[i] = 0;
            channel_values[i] = round(channel_values[i]);
        }
        fadeCounter = 0;
    }
}
