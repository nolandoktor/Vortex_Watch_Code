#ifndef BUTTON_HANDLER_LIB
#define BUTTON_HANDLER_LIB


enum Buttons
{
  up_button,
  down_button
};

//Button interrupt variables
int intPins[2] = {10, 9};
volatile double timers[2];
volatile int prevStates[2] = {0, 0};
volatile bool longPresses[2];
volatile bool shortPresses[2];

void resetButtonStates()
{
  for (int i=0; i<2; i++)
  {
    if (longPresses[i])
      longPresses[i] = false;
    if (shortPresses[i])
      shortPresses[i] = false;
  }
}
void enableButtonInterrupts()
{
  PCICR |= (1 <<PCIE0);
  PCMSK0 |= ((1 << PCINT5) | (1 << PCINT6));
}
void initButtonHandler()
{
  resetButtonStates();
  for (int i=0; i<2; i++)
  {
    pinMode(intPins[i], INPUT);
    prevStates[i] = digitalRead(intPins[0]);
    shortPresses[i] = false;
    longPresses[i] = false;
  }
}
ISR(PCINT0_vect)
{
  //Serial.println("Interrupt:");
  int states[2];
  for (int i=0; i<2; i++)
    states[i] = digitalRead(intPins[i]);
  delayMicroseconds(200);

  for (int i=0; i<2; i++)
  {
    if (states[i] == digitalRead(intPins[i]))
    {
      if (prevStates[i] == 1 && states[i] == 0)
      {
        //Rising Edge -------- OLD
        //Falling edge
        //Serial.print(i);
        //Serial.println(": Falling Edge");
        double pressTime = millis() - timers[i];

        if (pressTime < 1000)
        {
          if (pressTime > 20)
          {
            shortPresses[i] = true;
            //Serial.println("Short press");
          }
        }
        else if (pressTime < 100000)
        {
          longPresses[i] = true;
          //Serial.println("Long press");
        }
      }
      else if (prevStates[i] == 0 && states[i] == 1)
      {
        //Falling edge -------- OLD
        //Rising edge
        //Serial.print(i);
        //Serial.println(": Rising edge");
        timers[i] = millis();
      }
      prevStates[i] = states[i];
    }
  }
}

#endif
