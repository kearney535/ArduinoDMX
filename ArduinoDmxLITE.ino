#include <IRremote.h>

#include <TimerOne.h>

#include <DmxSimple.h>

// CONSTANTS

#define DMX_OUT_PIN 3
#define IR_PIN 4
#define NUM_OF_CHANNELS 16

#define FADE_STEP 10
#define FADE_TIME 5

uint16_t fadeCounter = 0;

uint16_t value = 0;
uint8_t IRvalue = 0;
uint16_t channel;

float channel_values[NUM_OF_CHANNELS + 1];
float channel_steps[NUM_OF_CHANNELS + 1];
int fadeTime;

uint8_t valueBuffer[NUM_OF_CHANNELS + 1];
uint8_t valueBufferPointer = 1;

IRrecv irrecv(IR_PIN);
decode_results results;

void fade(uint16_t channel, uint8_t value);
void fadeToValues(uint8_t *vals);
void increment();

/*
  COMMAND SYNTAX

    <channel>c<value>w: fade <channel> to <value> over 50ms
    f<value>,<value>,<value>,...<value>,
 */

void setup() {
  // Initialize globals
  value = 0;
  
  memset(channel_values, 0, sizeof(channel_values));
  memset(channel_steps, 0, sizeof(channel_steps));
  memset(valueBuffer, 0, sizeof(valueBuffer));

  // Initialize serial
  Serial.begin(115200);

  //Initialize DMX
  DmxSimple.usePin(3);

  irrecv.enableIRIn();

  Timer1.initialize(1000 * FADE_STEP);

  for (int i = 1; i < NUM_OF_CHANNELS + 1; i++)
    DmxSimple.write(i, 0);
}


void loop() {
  int c;

  if (Serial.available()) {
    c = Serial.read();
    Serial.print((char)c);
    if ((c>='0') && (c<='9')) {
      value = 10*value + c - '0';
   //   Serial.println(value);
    }
    else {
      //    Serial.println((char)c);
      if (c=='c') channel = value;
      else if (c=='w') {
        if (channel > 0)
        {
          fadeTime = FADE_TIME;
          fade(channel, value);
       //   DmxSimple.write(channel, value);
        }
      }
      else if (c == 'q')
        Serial.print("71001");
      else if (c == 'f')
        valueBufferPointer = 1;
      else if (c == ',')
      {
        valueBuffer[valueBufferPointer] = value;
    //    Serial.print("Writing ");
    //    Serial.print(value);
    //    Serial.print(" to value buffer at ");
    //    Serial.println(valueBufferPointer);
        valueBufferPointer++;
        if (valueBufferPointer == sizeof(valueBuffer))
        {
            valueBufferPointer = 1;
        }
      }
      else if (c == 't')
      {
        fadeTime = value;
    //    Serial.print("1:");
    //    Serial.println(fadeTime);
        fadeToValues(valueBuffer);
      }

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
  //  Serial.print(channel_steps[channel]);
    Timer1.attachInterrupt(increment);
}

void fadeToValues(uint8_t *vals)
{
    fadeCounter = 0;
    Serial.println(fadeTime);
    for (int i = 1; i < NUM_OF_CHANNELS + 1; i++)
    {
        channel_steps[i] = ((float)vals[i] - (float)channel_values[i])/(float)fadeTime;
  //      Serial.print(channel_steps[i]);
   //     Serial.print(",");
    }

    Timer1.attachInterrupt(increment);
}

void increment()
{
    for (uint8_t i = 0; i < NUM_OF_CHANNELS + 1; i++)
    {
        if (channel_steps[i] != 0)
        {
            channel_values[i] += channel_steps[i];
     //       Serial.print(channel_values[i]);
     //       Serial.print(",");
            DmxSimple.write(i, (uint8_t)(channel_values[i]));
        }
    }
 //   Serial.println();
    fadeCounter++;
//    Serial.println(fadeCounter);
    if (fadeCounter == fadeTime)
    {
        Timer1.detachInterrupt();
        Serial.print("y");
        for (uint8_t i = 0; i < NUM_OF_CHANNELS + 1; i++)
        {
            channel_steps[i] = 0;
            channel_values[i] = round(channel_values[i]);
            Serial.print((int)channel_values[i]);
            Serial.print(",");
        }
        fadeCounter = 0;
    }
}
