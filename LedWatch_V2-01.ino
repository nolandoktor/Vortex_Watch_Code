//testing testing

#include <avr/sleep.h>
#include <avr/power.h>`

#include <Wire.h>
#include <LiFuelGauge.h>


#include "DS3231_lib.h"
//#include "LedBuffer.h"
#include "DoubleBuffer.h"
#include "GlobalDefines.h"
#include "LedDriver.h"
#include "TestClock.h"
#include "WatchFace.h"
#include "ButtonHandler.h"
#include "Game.h"

//#define FPS 90.0
volatile byte sec_, min_, hour_, weekday_, day_, month_, year_;
volatile bool useRTC = false;

//bool isrFlag = true;
WatchFace *watchFace = NULL;
Game *watchGame = NULL;
volatile TestClock testClock;
DoubleBuffer *lBuffer;

LiFuelGauge *gauge;
double battery_threshold = 28;

int clockInterruptPin = 7;


int sec_ticker = 1;
int ticker_delay = 50;

int update_timer;

enum Watch_States
{
  SLEEP, AWAKE, SET_HOUR, SET_MIN, TIMING_GAME, BATTERY_LEVEL, NUM_STATES
};
int state_var = AWAKE;
int prev_state = AWAKE;
bool state_change = false;
int hour_temp, min_temp;
long timeout_val = 150000; //300000;//60000;
long timeout_timer;


void goToSleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode(); 
}

int MOSFET = 5;
void sleepNow();
void setup() {
  power_adc_disable();
  power_spi_disable();
  
  pinMode(13, INPUT);
/*
  for (int i=0; i<5; i++)
  {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }
*/

  cli();
  enableButtonInterrupts();
  sei();

  
  
  
  //initDriver();

  gauge = new LiFuelGauge(MAX17043);
  lBuffer = new DoubleBuffer();
  
  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, HIGH);

  
  initRTC();
  setDS3231time(30,24,23,2,22,02,16);
  byte sec_, min_, hour_;
  readDS3231time (&sec_, &min_, &hour_);
  testClock.initClock((int)hour_, (int)min_, (int)sec_);
  
  //watchFace = new StandardFace(&testClock);
  watchFace = new CascadeFace(&testClock);
  //watchFace = new FelixFace(&testClock);
  
  watchGame = new TimingGame(longPresses, shortPresses, 2);

  
  
  initButtonHandler();  

  /*
  digitalWrite(13, HIGH);
  sleepNow();
  digitalWrite(13, LOW);
  */
  
  gauge->reset();    

  

  pinMode(clockInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), tickClock, RISING);

  //Setup interrupt
  //cli();
  //enableButtonInterrupts();
  //sei();

  
}

/*
ISR(TIMER2_COMPA_vect)
{ 
  if(lBuffer_ == NULL)
    return;
  if (ledCounter_prev != ledCounter)
    turnOffSinks();

  int ledCounter_scaled = 2*ledCounter;
  for (int i=0; i<2; i++)
  {
    if (lBuffer_->getColorVal(ledCounter_scaled+i, 0) > pwmCounter)
      digitalWrite(rPWM_[i], LED_ON);//On  
    else
      digitalWrite(rPWM_[i], LED_OFF);//Off
      
    if (lBuffer_->getColorVal(ledCounter_scaled+i, 1) > pwmCounter)
      digitalWrite(gPWM_[i], LED_ON);//On
    else
      digitalWrite(gPWM_[i], LED_OFF);//Off
      
    if (lBuffer_->getColorVal(ledCounter_scaled+i, 2) > pwmCounter)
      digitalWrite(bPWM_[i], LED_ON);//On
    else
      digitalWrite(bPWM_[i], LED_OFF);//Off
  }

  if (ledCounter_prev != ledCounter)
    digitalWrite(ledSinks_[ledCounter], COMMON_ON);
  ledCounter_prev = ledCounter;
  

  //PWM
  pwmCounter++;
  if (pwmCounter >= COLOR_RES)
  {
    pwmCounter = 0;
    ledCounter++;  
  }
  if (ledCounter >= 6)
    ledCounter = 0;
}
*/
void sleepNow()
{
    cli();
  
    // Choose our preferred sleep mode:
    //set_sleep_mode(SLEEP_MODE_IDLE);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
    // Set sleep enable (SE) bit:
    sleep_enable();

    sei();
 
    // Put the device to sleep:
    sleep_mode();
 
    // Upon waking up, sketch continues from this point.
    sleep_disable();
}
void enter_sleep_mode()
{
  detachInterrupt(digitalPinToInterrupt(clockInterruptPin));
  /*
  turnOffDrawingISR();
  turnOffSinks();
  //turnOffSources();
  for (int i=0; i<2; i++)
  {
    digitalWrite(rPWM_[i], 1);
    digitalWrite(gPWM_[i], 1);
    digitalWrite(bPWM_[i], 1);
  }
  */
  //lBuffer->clear();
  //lBuffer->update();
  pinMode(NEOPIX_PIN, INPUT_PULLUP);
  digitalWrite(MOSFET, LOW);
  set1HzClock(1);
  //goToSleep();
  
  //digitalWrite(13, HIGH);
  //delay(1000);
  //PCICR &= ~(1 << PCIE0);
  sleepNow();
  //PCICR |= (1 << PCIE0);
  //digitalWrite(13, LOW);
}
void wake_from_sleep()
{
  
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), tickClock, RISING);
  set1HzClock(0);
  //turnOnDrawingISR();
  pinMode(NEOPIX_PIN, OUTPUT);
  digitalWrite(MOSFET, HIGH);
}



void tickClock()
{
  testClock.update();
}


int printCnt = 9;
//double chg = 50.0;
double chargeLevel = 50.0;
double warning_flash = 1.0;
int warning_dir = -1;
void loop() 
{
  /*
  double chargeLevel = 4;//chg;
  int charge_idx = (int)(N_LEDS*chargeLevel/100);
  lBuffer->clear();
  int intensity = COLOR_RES;
  double warning_thresh = 5.0;
  


  for (int i=0; i<N_LEDS; i++)
  {
    int j = N_LEDS - i - 1;
    if (chargeLevel < warning_thresh)
    {
      lBuffer->setColorVal(i, LedBuffer::RED, (int)(intensity*warning_flash));
      lBuffer->setColorVal(j, LedBuffer::GREEN, (int)(intensity*warning_flash));
    }
    else
    {
      lBuffer->setColorVal(i, LedBuffer::RED, intensity);
      lBuffer->setColorVal(j, LedBuffer::GREEN, intensity);
    }
    intensity--;
    if (intensity < 0)
      intensity; 
  }
  for (int i=0; i<N_LEDS; i++)
  {
    if (i > charge_idx)
      lBuffer->setColorVal(i, 0, 0, 0);
    //if ((int)chargeLevel < 8 && (int)chargeLevel >= i)
    //  lBuffer->setColorVal(i, LedBuffer::BLUE, COLOR_RES);
  }
  lBuffer->update();
  chg -= 1;
  if (chg < 0)
    chg = 100;

  if (warning_dir < 0)
  {
    warning_flash -= 0.05;
  }
  if (warning_dir > 0)
  {
    warning_flash += 0.05;
  }
  if (warning_flash < 0.25)
  {
    warning_flash = 0.25;
    warning_dir *= -1;
  }
  if (warning_flash > 1.0)
  {
    warning_flash = 1.0;
    warning_dir *= -1;
  }
  delay(30);
  return;
  */
  
  /*
  for (int i=0; i<6; i++)
  {
    turnOffSinks();
    turnOffSources();
    pinMode(rPWM_[0], OUTPUT);
    digitalWrite(rPWM_[0], 0);
    pinMode(rPWM_[1], OUTPUT);
    digitalWrite(rPWM_[1], 0);
    pinMode(gPWM_[0], OUTPUT);
    digitalWrite(gPWM_[0], 0);
    pinMode(gPWM_[1], OUTPUT);
    digitalWrite(gPWM_[1], 0);
    pinMode(bPWM_[0], OUTPUT);
    digitalWrite(bPWM_[0], 0);
    pinMode(bPWM_[1], OUTPUT);
    digitalWrite(bPWM_[1], 0);

    pinMode(ledSinks_[0], OUTPUT);
    digitalWrite(ledSinks_[0], HIGH);
    delay(200000);
  }

  return;
  */
  /*
  if(watchGame->gameIsOver())
  {
    watchGame->reset();
  }
  watchGame->update();
  watchGame->draw(lBuffer);
  delay(16);
  return;
  */
  /*
  for (int i=0; i<6; i++)
  {
    turnOffSinks();
    turnOffSources();
    digitalWrite(rPWM_[0], 1);
    digitalWrite(rPWM_[1], 1);
    digitalWrite(gPWM_[0], 1);
    digitalWrite(gPWM_[1], 1);
    digitalWrite(bPWM_[0], 1);
    digitalWrite(bPWM_[1], 1);

    //digitalWrite(ledSinks_[0], HIGH);
    delay(50000);
  }

  return;
  */
  /*
  lBuffer->clear();
  for (int i=0; i<N_LEDS; i++)
  {
    lBuffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
  }
  lBuffer->update();
  delay(1000);
  return;
  */
  
  int delay_val = 10;
  timeout_timer = millis();
  update_timer = millis();
  while (1)
  {
    if (millis() - update_timer >= 1000)
    {
      //testClock.update();
      update_timer = millis();
    }


    
    printCnt++;
    if (printCnt >= 10)
    {
      printCnt = 0;
      //Serial.print("SOC: ");
      //Serial.println(gauge->getSOC());
    }
    
    //digitalWrite(13, LOW);
    prev_state = state_var;
    switch (state_var)
    {
      case SLEEP:
      {
        //digitalWrite(13, HIGH);
        bool enterSleep = false;
        if (state_change)
        {
          state_change = false;

          enterSleep = true;
          /*
          detachInterrupt(digitalPinToInterrupt(clockInterruptPin));
          //turnOffDrawingISR();
          //turnOffSinks();
          //turnOffSources();
          //for (int i=0; i<2; i++)
          //{
          //  digitalWrite(rPWM_[i], 0);
          //  digitalWrite(gPWM_[i], 0);
          //  digitalWrite(bPWM_[i], 0);
          //}

          lBuffer->clear();
          lBuffer->update();

          
          set1HzClock(1);
          goToSleep();
          */
        }


        lBuffer->clear();
        lBuffer->update();

        
        double chargeLevel = gauge->getSOC();
        if (chargeLevel <= battery_threshold && chargeLevel > 0)
        {
          enterSleep = true;
        }
        
        if (enterSleep)
        {
          enter_sleep_mode();
          //break;
        }
        
        
        

        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
          timeout_timer = millis();
        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
        {
          state_var = AWAKE;
          state_change = true;
          byte sec_, min_, hour_;
          readDS3231time (&sec_, &min_, &hour_);
          testClock.initClock((int)hour_, (int)min_, (int)sec_);
          
          attachInterrupt(digitalPinToInterrupt(clockInterruptPin), tickClock, RISING);
          set1HzClock(0);
          turnOnDrawingISR();
          pinMode(NEOPIX_PIN, OUTPUT);
          digitalWrite(MOSFET, HIGH);
        }
        resetButtonStates();
        break;
      }
      case AWAKE:
      {
        if (state_change)
        {
          state_change = false;
        }
        
        //testClock.draw(lBuffer);
        //testClock.printClock();
        watchFace->update();
        watchFace->draw(lBuffer);
        lBuffer->update();

        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
          timeout_timer = millis();
        if (shortPresses[up_button])
        {
          state_var = BATTERY_LEVEL;
          state_change = true;
        }
        else if (shortPresses[down_button])
        {
          state_var = SLEEP;
          state_change = true;
        }
        if (longPresses[up_button]/* || longPresses[down_button]*/)
        {
          state_var = SET_HOUR;
          state_change = true;
        }
        else if (longPresses[down_button])
        {
          state_var = TIMING_GAME;
          state_change = true;
        }
        else if (millis() - timeout_timer >= timeout_val)
        {
          state_var = SLEEP;
          state_change = true;
        }
        resetButtonStates();
        break;
      }
      case SET_HOUR:
      {
        if (state_change)
        {
          state_change = false;

          hour_temp = testClock.getHours();
          sec_ticker = 0;
        }
        lBuffer->clear();
        sec_ticker++;
        if (sec_ticker >= ticker_delay)
          sec_ticker = 0;

        /*
        if (sec_ticker < ticker_delay/2)
        {
          for (int i=0; i<=hour_temp; i++)
            lBuffer->setColorVal(i, 0, COLOR_RES);
        }
        */
        if (sec_ticker < ticker_delay/2)
        {
          for (int i=0; i<N_LEDS; i++)
          {
            lBuffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
            if (i <= hour_temp)
              lBuffer->setColorVal(i, COLOR_RES, 0, 0);
          }
        }
        lBuffer->update();

        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
          timeout_timer = millis();
        if (shortPresses[up_button])
        {
          hour_temp++;
          if (hour_temp >= N_LEDS)
            hour_temp = 0;
        }
        else if (shortPresses[down_button])
        {
          hour_temp--;
          if (hour_temp < 0)
            hour_temp = N_LEDS-1;
        }
        if (longPresses[up_button] || longPresses[down_button])
        {
          state_var = SET_MIN;
          state_change = true;
        }
        else if (millis() - timeout_timer >= timeout_val)
        {
          state_var = SLEEP;
          state_change = true;
        }
        resetButtonStates();
        break;
      }
      case SET_MIN:
      {
        if (state_change)
        {
          state_change = false;

          min_temp = testClock.getMinutes();
          sec_ticker = 0;
        }
        
        lBuffer->clear();
       
        
        sec_ticker++;
        if (sec_ticker >= ticker_delay)
          sec_ticker = 0;

        if (sec_ticker < ticker_delay/2)
        {
          
          int min_val = min_temp;
          int min_idx = 0;
          if (min_val > 0)
          {
            min_val--;
            min_idx = min_val/5;
            min_val = min_val%5 + 1;
            min_val = (min_val*COLOR_RES)/5;
          }
          for (int i=0; i<N_LEDS; i++)
          {
            lBuffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
            if (i <= min_idx)
              lBuffer->setColorVal(i, 0, min_val, 0);
          }

          /*   
          int min_rem = ((min_temp%5 + 1)*COLOR_RES)/5;
          int min_temp_idx = min_temp/5;
          
          for (int i=0; i<N_LEDS; i++)
          {
              lBuffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
              if (i <= min_temp_idx)
                lBuffer->setColorVal(i, 0, min_rem, 0);
          }
          */
        }
        lBuffer->update();

        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
          timeout_timer = millis();
        if (shortPresses[up_button])
        {
          min_temp++;
          if (min_temp >= 60)
            min_temp = 0;
        }
        else if (shortPresses[down_button])
        {
          min_temp--;
          if (min_temp < 0)
            min_temp = 60-1;
        }
        if (longPresses[up_button] || longPresses[down_button])
        {
          state_var = AWAKE;
          state_change = true;
          testClock.initClock(hour_temp, min_temp, 0);
          setDS3231time(testClock.getSeconds(), testClock.getMinutes(), testClock.getHours(),2,22,02,16);
        }
        else if (millis() - timeout_timer >= timeout_val)
        {
          state_var = SLEEP;
          state_change = true;
        }
        resetButtonStates();
        break;
      }
      case TIMING_GAME:
      {
        if (state_change)
        {
          state_change = false;
          watchGame->reset();
        }
        watchGame->update();
        watchGame->draw(lBuffer);
        timeout_timer = millis();

        if (watchGame->gameIsOver())
        {
          state_var = AWAKE;
          state_change = true;
        }
        break;
      }
      case BATTERY_LEVEL:
      {
        if (state_change)
        {
          state_change = false;
          chargeLevel = gauge->getSOC();
          //chargeLevel = rand()%101;
        }

        int charge_idx = (int)(N_LEDS*chargeLevel/100);
        int intensity = COLOR_RES;
        double warning_thresh = 10.0;
        lBuffer->clear();

        for (int i=0; i<N_LEDS; i++)
        {
          int j = N_LEDS - i - 1;
          if (chargeLevel < warning_thresh)
          {
            lBuffer->setColorVal(i, LedBuffer::RED, (int)(intensity*warning_flash));
            lBuffer->setColorVal(j, LedBuffer::GREEN, (int)(intensity*warning_flash));
          }
          else
          {
            lBuffer->setColorVal(i, LedBuffer::RED, intensity);
            lBuffer->setColorVal(j, LedBuffer::GREEN, intensity);
          }
          intensity -= COLOR_RES/8; //1;
          if (intensity < 0)
            intensity = 0; 
        }
        for (int i=0; i<N_LEDS; i++)
        {
          if (i > charge_idx)
            lBuffer->setColorVal(i, 0, 0, 0);
        }
        lBuffer->update();

        if (warning_dir < 0)
        {
          warning_flash -= 0.05;
        }
        if (warning_dir > 0)
        {
          warning_flash += 0.05;
        }
        if (warning_flash < 0.25)
        {
          warning_flash = 0.25;
          warning_dir *= -1;
        }
        if (warning_flash > 1.0)
        {
          warning_flash = 1.0;
          warning_dir *= -1;
        }

        if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
        {
          timeout_timer = millis();
        }
        if (shortPresses[up_button])
        {
          state_var = AWAKE;
          state_change = true;
        }
        else if(shortPresses[down_button])
        {
          state_var = SLEEP;
          state_change = true;
        }
        else if (millis() - timeout_timer >= timeout_val)
        {
          state_var = SLEEP;
          state_change = true;
        }
        resetButtonStates();
        
        break;
      }
    }
    
    delay(delay_val);    
  }
}


