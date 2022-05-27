#include <Arduino.h>
#include <ArduinoLowPower.h>
#include "ButtonHandler.h"
#include "../Misc/PinMapping.h"

//Button interrupt variables
static int intPins[2] = {BUTTON0_PIN, BUTTON1_PIN};
static volatile uint32_t timers[2] = {0, 0};
static volatile int prevStates[2] = {0, 0};

volatile bool longPresses[2];
volatile bool shortPresses[2];


static void button0_interrupt_handler();
static void button1_interrupt_handler();

void resetButtonStates()
{
  for (int i=0; i<2; i++)
  {
    longPresses[i] = false;
    shortPresses[i] = false;
  }
}
void enableButtonInterrupts()
{
  //PCICR |= (1 <<PCIE0);
  //PCMSK0 |= ((1 << PCINT5) | (1 << PCINT6));
    //Not supported on M0
}
void initButtonHandler()
{
  resetButtonStates();
  for (int i=0; i<2; i++)
  {
    pinMode(intPins[i], INPUT_PULLUP);
    prevStates[i] = digitalRead(intPins[i]);
  }
  //Attach interrupt handlers for pins 
  attachInterrupt(digitalPinToInterrupt(intPins[0]), button0_interrupt_handler, CHANGE);
  LowPower.attachInterruptWakeup(digitalPinToInterrupt(intPins[1]), button1_interrupt_handler, CHANGE);
}

static void button_function(int idx)
{
  int pin_status = digitalRead(intPins[idx]);
  uint32_t event_ts = millis();
  if (pin_status == 0) {
    Serial.print("Button ");
    Serial.print(intPins[idx]);
    Serial.println(" pressed");
    timers[idx] = event_ts;
  }
  else {
    Serial.print("Button ");
    Serial.print(intPins[idx]);
    Serial.println(" released");
    int32_t delta = event_ts - timers[idx];
    if (delta < 500) {
      Serial.println("Short Press");
      shortPresses[idx] = true;
    }
    else{
      Serial.println("Long Press");
      longPresses[idx] = true;
    }
  }
}
static void button0_interrupt_handler()
{
  button_function(0);
}
static void button1_interrupt_handler()
{
  button_function(1);
}



