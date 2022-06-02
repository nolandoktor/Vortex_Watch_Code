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
int StateElement::change_state(watch_state_t next_state, bool state_change)
{
    if (state_change) {
        if (state_manager->change_state(next_state) < 0) {
            Serial.print("Error: Failed to change to state: ");
            const char *state_name = state_manager->get_state_name(next_state);
            if (state_name == NULL) {
                Serial.println("NULL_STATE");
            }
            else {
                Serial.println(state_name);
            }
        }
    }
    return 0;
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
        
        while (digitalRead(CLK_1HZ_PIN) == HIGH);
        while (digitalRead(CLK_1HZ_PIN) == LOW);
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
        attachInterrupt(digitalPinToInterrupt(CLK_1HZ_PIN), tickClock, RISING);
        //set1HzClock(0);
        pinMode(LED_DAT_PIN, OUTPUT);
        digitalWrite(LED_MOSFET_EN_PIN, HIGH);
        */
    }

    change_state(next_state, state_change);

    return 0;
}

//-------------------------------------------------

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
    watch_state_t next_state;
    bool state_change = false;

    watchFace->update();
    watchFace->draw(frame_buffer);
    frame_buffer->update();
    
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

    
    change_state(next_state, state_change);
    
    return 0;
}
int AwakeState::on_enter(watch_state_t prev_state)
{
    timeout_timer = millis();
    return 0;
}

//-------------------------------------------------------

typedef struct {
    uint8_t hour;
    uint8_t minute;
} time_data_t;

SetHourState::SetHourState(StateManager *sm, DoubleBuffer *fb,  volatile TestClock *tc) : StateElement(sm, fb)
{
    clock = tc;
}
int SetHourState::init()
{
    timeout_timer = 0;
    hour = 0;
    flash_cnt = 0;
    flash_period = DEFAULT_FLASH_PERIOD;
}
int SetHourState::update()
{
    watch_state_t next_state;
    bool state_change = false;

    frame_buffer->clear();
    if (flash_cnt++ >= flash_period) {
        flash_cnt = 0;
    }
    
    if (flash_cnt < (flash_period / 2)) {
        for (int i=0; i<N_LEDS; i++) {
            if (i <= hour) {
                frame_buffer->setColorVal(i, COLOR_RES, 0, 0);
            }
            else {
                frame_buffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
            }
        }
    }
    frame_buffer->update();

    if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button]) {
        timeout_timer = millis();
    }
    if (shortPresses[up_button]) {
        hour++;
        if (hour >= N_LEDS)
        hour = 0;
    }
    else if (shortPresses[down_button]) {
        hour--;
        if (hour < 0)
        hour = N_LEDS-1;
    }
    if (longPresses[up_button] || longPresses[down_button]) {
        next_state = SET_MIN_STATE;
        state_change = true;
    }
    else if ((millis() - timeout_timer) >= SLEEP_TIMEOUT)
    {
        next_state = SLEEP_STATE;
        state_change = true;
    }

    change_state(next_state, state_change);    
    return 0;
}
int SetHourState::on_enter(watch_state_t prev_state)
{
    timeout_timer = millis();
    hour = clock->getHours();
    flash_cnt = 0;
    return 0;
}
int SetHourState::on_exit(watch_state_t next_state)
{
    if (next_state == SET_MIN_STATE) {
        uint8_t *hour_shared = state_manager->get_scratch();
        if (hour_shared == NULL) {
            return -1;
        }
        *hour_shared = hour;
    }
    return 0;
}


SetMinuteState::SetMinuteState(StateManager *sm, DoubleBuffer *fb,  volatile TestClock *tc) : StateElement(sm, fb)
{
    clock = tc;
}
int SetMinuteState::init()
{
    timeout_timer = 0;
    hour = 0;
    minute = 0;
    flash_cnt = 0;
    flash_period = DEFAULT_FLASH_PERIOD;
}
int SetMinuteState::update()
{
    watch_state_t next_state;
    bool state_change = false;

    frame_buffer->clear();
    if (flash_cnt++ >= flash_period) {
        flash_cnt = 0;
    }
    
    if (flash_cnt < (flash_period / 2)) {
        int min_val = minute;
        int min_idx = 0;

        if (min_val > 0) {
            min_val--;
            min_idx = min_val / 5;
            min_val = (min_val % 5) + 1;
            min_val = (min_val * COLOR_RES) / 5;
        }

        for (int i=0; i<N_LEDS; i++) {
            frame_buffer->setColorVal(i, COLOR_RES, COLOR_RES, COLOR_RES);
            if (i <= min_idx) {
                frame_buffer->setColorVal(i, 0, min_val, 0);
            }
        }
    }
    frame_buffer->update();

    if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button]) {
        timeout_timer = millis();
    }
    if (shortPresses[up_button]) {
        minute++;
        if (minute >= 60) {
            hour = 0;
        }
    }
    else if (shortPresses[down_button]) {
        minute--;
        if (minute < 0)
        minute = 60-1;
    }
    if (longPresses[up_button] || longPresses[down_button]) {
        next_state = AWAKE_STATE;
        state_change = true;
    }
    else if ((millis() - timeout_timer) >= SLEEP_TIMEOUT)
    {
        next_state = SLEEP_STATE;
        state_change = true;
    }

    change_state(next_state, state_change);    
    return 0;
}
int SetMinuteState::on_enter(watch_state_t prev_state)
{
    timeout_timer = millis();
    minute = clock->getMinutes();
    flash_cnt = 0;

    if (prev_state == SET_HOUR_STATE) {
        uint8_t *hour_shared = state_manager->get_scratch();
        hour = *hour_shared;
    }
    else {
        Serial.print("Error: ");
        Serial.print(get_name());
        Serial.print(" not entered from ");
        Serial.println(state_manager->get_state_name(SET_HOUR_STATE));
        hour = 0;
    }
    return 0;
}
int SetMinuteState::on_exit(watch_state_t next_state)
{   
    if (next_state == AWAKE_STATE) {
        byte sec_, min_, hour_, weekday_, day_month_, month_, year_; 
        int ret = rtc_get_datetime(&sec_, &min_, &hour_, &weekday_, &day_month_, &month_, &year_);
        clock->initClock(hour, minute, 0);
        if (ret < 0) {
            return -1;
        }
        return rtc_set_datetime(0, minute, hour, (weekday_t)weekday_, day_month_, month_, year_);
    }
    return 0;
}

//----------------------------------------------------------

BatteryState::BatteryState(StateManager *sm, DoubleBuffer *fb, LiFuelGauge *fg) : StateElement(sm, fb)
{
    fuel_gauge = fg;
}
int BatteryState::init()
{
    timeout_timer = 0;
    charge_level = 0;
    warning_flash = 1.0;
    warning_dir = -1;
}
int BatteryState::update()
{
    watch_state_t next_state;
    bool state_change = false;

    int charge_idx = (int)((N_LEDS*charge_level) / 100);
    int intensity = COLOR_RES;
    int warning_thresh = 10;
    frame_buffer->clear();
    
    for (int i=0; i<N_LEDS; i++) {
        int j = (N_LEDS - i) - 1;
        if (charge_level < warning_thresh)
        {
            frame_buffer->setColorVal(i, DoubleBuffer::R, (int)(intensity*warning_flash));
            frame_buffer->setColorVal(j, DoubleBuffer::G, (int)(intensity*warning_flash));
        }
        else
        {
            frame_buffer->setColorVal(i, DoubleBuffer::R, intensity);
            frame_buffer->setColorVal(j, DoubleBuffer::G, intensity);
        }
        intensity -= COLOR_RES / 8;
        if (intensity < 0) {
            intensity = 0; 
        }
    }
    for (int i=0; i<N_LEDS; i++) {
        if (i > charge_idx) {
            frame_buffer->setColorVal(i, 0, 0, 0);
        }
    }
    frame_buffer->update();

    if (warning_dir < 0) {
        warning_flash -= 0.05;
    }
    if (warning_dir > 0) {
        warning_flash += 0.05;
    }
    if (warning_flash < 0.25) {
        warning_flash = 0.25;
        warning_dir *= -1;
    }
    if (warning_flash > 1.0) {
        warning_flash = 1.0;
        warning_dir *= -1;
    }

    if (shortPresses[up_button] || shortPresses[down_button] || longPresses[up_button] || longPresses[down_button]) {
        timeout_timer = millis();
    }

    if (shortPresses[up_button])
    {
        next_state = AWAKE_STATE;
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

    change_state(next_state, state_change);
    return 0;
}
int BatteryState::on_enter(watch_state_t prev_state)
{
    timeout_timer = millis();
    charge_level = fuel_gauge->getSOC();
    return 0;
}