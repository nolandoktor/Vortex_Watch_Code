#ifndef LED_DRIVE_LIB
#define LED_DRIVE_LIB

#include <Arduino.h>
#include "GlobalDefines.h"

#define COMMON_GND_LED true
#if COMMON_GND_LED 
#define COMMON_ON LOW
#define COMMON_OFF HIGH
#define LED_ON HIGH
#define LED_OFF LOW
#else
#define COMMON_ON HIGH
#define COMMON_OFF LOW
#define LED_ON LOW
#define LED_OFF HIGH
#endif


volatile int ledPin = 2;

volatile int ledSinks_[6] = {3, 4, 5, 6, 7, 8}; //
volatile int rPWM_[2] = {12, 11}; //to match eagle =>{12, 11}
volatile int gPWM_[2] = {15, 14};
volatile int bPWM_[2] = {17, 16};




volatile int ledCounter = 0;
volatile int ledCounter_prev = -1;
volatile int pwmCounter = 0;

DoubleBuffer *lBuffer_ = NULL;

void useBuffer(DoubleBuffer *buf)
{
  lBuffer_ = buf;
}

void turnOffSinks()
{
  return;
  
  /*
  for (int i=0; i<N_LEDS; i++)
  {
    digitalWrite(ledSinks[i], LOW);
  }
  */
  for (int i=0; i<6; i++)
  {
    digitalWrite(ledSinks_[i], /*HIGH*/COMMON_OFF);
    //pinMode(ledSinks_[i], INPUT);
  }
}

void turnOffSources()
{
  return;
  
  for (int i=0; i<2; i++)
  {
    digitalWrite(rPWM_[i], /*LOW*/LED_OFF);
    digitalWrite(gPWM_[i], /*LOW*/LED_OFF);
    digitalWrite(bPWM_[i], /*LOW*/LED_OFF);

    /*
    pinMode(rPWM_[i], INPUT);
    pinMode(gPWM_[i], INPUT);
    pinMode(bPWM_[i], INPUT);
    */
  }
}
void turnOnSources()
{
  return;
  
  for (int i=0; i<2; i++)
  {
    digitalWrite(rPWM_[i], /*LOW*/LED_ON);
    digitalWrite(gPWM_[i], /*LOW*/LED_ON);
    digitalWrite(bPWM_[i], /*LOW*/LED_ON);
  }
}
void initDriver()
{
  return;
  /*
  for (int i=0; i<N_LEDS; i++)
    pinMode(ledSinks[i], OUTPUT);
  */
  for (int i=0; i<6; i++)
    pinMode(ledSinks_[i], OUTPUT);

  /*
  pinMode(rPWM, OUTPUT);
  pinMode(gPWM, OUTPUT);
  pinMode(bPWM, OUTPUT);
  */
  for (int i=0; i<2; i++)
  {
    pinMode(rPWM_[i], OUTPUT);
    pinMode(gPWM_[i], OUTPUT);
    pinMode(bPWM_[i], OUTPUT);
  }
  
  turnOffSinks();
  turnOffSources();
}
void turnOffDrawingISR()
{
  //TIMSK2 &= ~(1 << OCIE2A);
}
void turnOnDrawingISR()
{
  //TIMSK2 |= (1 << OCIE2A);
}
//int drawLEDs(uint32_t dummy) /** Simblee Specific **/
/*
void drawLEDs() 
{
  
  if(lBuffer_ == NULL)
    //return -1; 
    return;      
  if (ledCounter_prev != ledCounter)
    turnOffSinks();

  if (lBuffer_->getColorVal(ledCounter, 0) > pwmCounter)
    digitalWrite(rPWM, 0);  
  else
    digitalWrite(rPWM, 1);
    
  if (lBuffer_->getColorVal(ledCounter, 1) > pwmCounter)
    digitalWrite(gPWM, 0); 
  else
    digitalWrite(gPWM, 1);
    
  if (lBuffer_->getColorVal(ledCounter, 2) > pwmCounter)
    digitalWrite(bPWM, 0);  
  else
    digitalWrite(bPWM, 1);
  

  if (ledCounter_prev != ledCounter)
    digitalWrite(ledSinks[ledCounter], HIGH);
  ledCounter_prev = ledCounter;


  //PWM
  pwmCounter++;
  if (pwmCounter >= COLOR_RES)
  {
    pwmCounter = 0;
    ledCounter++;  
  }
  if (ledCounter >= 12)
    ledCounter = 0;

  

  //PDM
  //ledCounter++;
  //if (ledCounter >= 12)
  //{
  //  ledCounter = 0;
  //  pwmCounter++;  
  //}
  //if (pwmCounter >= COLOR_RES)
  //  pwmCounter = 0;  
  
  
  return;
}
*/


#endif
