#ifndef BUTTON_HANDLER_LIB
#define BUTTON_HANDLER_LIB

#define BUTTON0_PIN 4
#define BUTTON1_PIN 3

enum Buttons
{
  up_button,
  down_button
};

//Button interrupt variables
int intPins[2] = {BUTTON0_PIN, BUTTON1_PIN};
volatile uint32_t timers[2] = {0, 0};
volatile int prevStates[2] = {0, 0};
volatile bool longPresses[2];
volatile bool shortPresses[2];

void button0_interrupt_handler();
void button1_interrupt_handler();

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
    prevStates[i] = digitalRead(intPins[0]);
  }
  //Attach interrupt handlers for pins 
  attachInterrupt(digitalPinToInterrupt(intPins[0]), button0_interrupt_handler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(intPins[1]), button1_interrupt_handler, CHANGE);
}

void button_function(int idx)
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
    if (delta < 200) {
      Serial.println("Short Press");
      shortPresses[idx] = true;
    }
    else if (delta < 2000){
      Serial.println("Long Press");
      longPresses[idx] = true;
    }
    else {
      Serial.println("Extra Long Press");
    }
  }
}
void button0_interrupt_handler()
{
  button_function(0);
}
void button1_interrupt_handler()
{
  button_function(1);
}

#endif
