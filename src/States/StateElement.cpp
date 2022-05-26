
#include <Arduino.h>
#include <LiFuelGauge.h>
#include "StateElement.h"
#include "StateManager.h"
#include "../Misc/GlobalDefines.h"
#include "../Input/ButtonHandler.h"
#include "../Time/RTC_API.h"
#include "../Time/TestClock.h"
#include "../Display/DoubleBuffer.h"
#include "../Display/WatchFace.h"

StateElement::StateElement(StateManager *sm, DoubleBuffer *fb) 
{
    state_manager = sm;
    frame_buffer = fb;
}

SleepState::SleepState(StateManager *sm, DoubleBuffer *fb,  volatile TestClock *tc, LiFuelGauge *gg) : StateElement(sm, fb)
{
    clock = tc;
    gas_gauge = gg;
}
int SleepState::init()
{
    sleep_request = false;
}
int SleepState::on_enter(watch_state_t prev_state)
{   
    sleep_request = true;
    return 0;
}
int SleepState::update()
{
    frame_buffer->clear();
    frame_buffer->update();
   
    double chargeLevel = gas_gauge->getSOC();
    bool low_battery = (chargeLevel <= BATTERY_SLEEP_THRESH) && (chargeLevel > 0);
    
    if (sleep_request || low_battery) {
        Serial.println("Entering sleep mode");
        //enter_sleep_mode();
        sleep_request = false;
    }

    watch_state_t next_state;
    bool state_change = false;
    if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button])
    {
        next_state = AWAKE_STATE;
        state_change = true;
        byte sec_, min_, hour_;
        //readDS3231time (&sec_, &min_, &hour_);

        /*
        rtc_set_clkout(CLK_1HZ);
        delay(10);
        
        while (digitalRead(clockInterruptPin) == HIGH);
        while (digitalRead(clockInterruptPin) == LOW);
        */

        rtc_get_time(&sec_, &min_, &hour_);
        Serial.print("Sec: ");
        Serial.println(sec_);
        Serial.print("Min: ");
        Serial.println(min_);
        Serial.print("Hour: ");
        Serial.println(hour_);

        clock->initClock(hour_, min_, sec_);
        clock->printClock();
        Serial.println("-------------");
        

        /*
        //rtc_set_clkout(CLK_1HZ);
        //delay(10);
        attachInterrupt(digitalPinToInterrupt(clockInterruptPin), tickClock, RISING);
        //set1HzClock(0);
        pinMode(NEOPIX_PIN, OUTPUT);
        digitalWrite(MOSFET, HIGH);
        */
    }

    if (state_change) {
        //next_state = AWAKE_STATE;
        state_manager->change_state(next_state);
    }
    return 0;
}

AwakeState::AwakeState(StateManager *sm, DoubleBuffer *fb, WatchFace *wf) : StateElement(sm, fb)
{
    watchFace = wf;
}
int AwakeState::init()
{
    timeout_timer = 0;
}
int AwakeState::update()
{
    watchFace->update();
    watchFace->draw(frame_buffer);
    frame_buffer->update();

    watch_state_t next_state;
    bool state_change = false;
    
    if (longPresses[up_button])
    {
        next_state = SET_HOUR_STATE;
        state_change = true;
    }
    else if (longPresses[down_button])
    {
        next_state = TIMING_GAME_STATE;
        state_change = true;
    }
    else if (shortPresses[up_button])
    {
        next_state = BATTERY_LEVEL_STATE;
        state_change = true;
    }
    else if (shortPresses[down_button])
    {
        next_state = SLEEP_STATE;
        state_change = true;
    }
    else if ((millis() - timeout_timer) >= SLEEP_TIMEOUT)
    {
        next_state = SLEEP_STATE;
        state_change = true;
    }

    
    if (state_change) {
        next_state = SLEEP_STATE;
        state_manager->change_state(next_state);
    }
    
    return 0;
}
int AwakeState::on_enter(watch_state_t prev_state)
{
    timeout_timer = millis();
    return 0;
}