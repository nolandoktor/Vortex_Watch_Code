#pragma once

#include <LiFuelGauge.h>
#include "StateManager.h"
#include "../Misc/GlobalDefines.h"
#include "../Time/TestClock.h"
#include "../Display/DoubleBuffer.h"
#include "../Display/WatchFace.h"
#include "../Games/Game.h"

class StateManager;
class StateElement {
  protected:
    static const int SLEEP_TIMEOUT = 150000;
    StateManager *state_manager;
    DoubleBuffer *frame_buffer;
    bool auto_input_reset;
    virtual int change_state(watch_state_t next_state, bool state_change);

  public:
    StateElement(StateManager *sm, DoubleBuffer *fb);
    virtual int init() {return 0;}
    virtual int on_enter(watch_state_t prev_state) {return 0;}
    virtual int on_exit(watch_state_t next_state) {return 0;}
    virtual int update() {return 0;}
    virtual const char* get_name() {return "INVALID_STATE";}
    virtual bool get_auto_input_reset() {return auto_input_reset;}
    
};

class SleepState : public StateElement {
  private:
    volatile TestClock *clock;
    LiFuelGauge *gas_gauge;
    bool sleep_request;
    static const int BATTERY_SLEEP_THRESH = 28;
  public:
    SleepState(StateManager *sm, DoubleBuffer *fb, volatile TestClock *tc, LiFuelGauge *gg);
    int init();
    int on_enter(watch_state_t prev_state);
    int update();
    const char* get_name() {return "SLEEP_STATE";} 
};

class AwakeState : public StateElement {
  private:
    WatchFace *watchFace;
    uint32_t timeout_timer;
  public:
    AwakeState(StateManager *sm, DoubleBuffer *fb, WatchFace *wf);
    int init();
    int on_enter(watch_state_t prev_state);
    int update();
    const char* get_name() {return "AWAKE_STATE";} 
};

class SetHourState : public StateElement {
  private:
    volatile TestClock *clock;
    uint32_t timeout_timer;
    int8_t hour;
    uint16_t flash_cnt;
    uint16_t flash_period;
    static const uint16_t DEFAULT_FLASH_PERIOD = 50;
  public:
    SetHourState(StateManager *sm, DoubleBuffer *fb, volatile TestClock *tc);
    int init();
    int on_enter(watch_state_t prev_state);
    int on_exit(watch_state_t next_state);
    int update();
    const char* get_name() {return "SET_HOUR_STATE";} 
    int set_flash_period(uint16_t fp) {flash_period = fp;}
};

class SetMinuteState : public StateElement {
  private:
    volatile TestClock *clock;
    uint32_t timeout_timer;
    int8_t hour;
    int8_t minute;
    uint16_t flash_cnt;
    uint16_t flash_period;
    static const uint16_t DEFAULT_FLASH_PERIOD = 50;
  public:
    SetMinuteState(StateManager *sm, DoubleBuffer *fb, volatile TestClock *tc);
    int init();
    int on_enter(watch_state_t prev_state);
    int on_exit(watch_state_t next_state);
    int update();
    const char* get_name() {return "SET_MINUTE_STATE";} 
    int set_flash_period(uint16_t fp) {flash_period = fp;}
};

class BatteryState : public StateElement {
  private:
    LiFuelGauge *fuel_gauge;
    int charge_level;
    double warning_flash;
    int8_t warning_dir;
    uint32_t timeout_timer;
    static constexpr double WARNING_THRESH_LOW = 0.25;
    static constexpr double WARNING_THRESH_HIGH = 1.0;
  public:
    BatteryState(StateManager *sm, DoubleBuffer *fb, LiFuelGauge *fg);
    int init();
    int on_enter(watch_state_t prev_state);
    int update();
    const char* get_name() {return "BATTERY_STATE";} 
};

class TimingGameState : public StateElement {
  private:
    Game *watch_game;
    uint32_t timeout_timer; 
  public:
    TimingGameState(StateManager *sm, DoubleBuffer *fb, Game *game);
    int init();
    int on_enter(watch_state_t prev_state);
    int update();
    const char* get_name() {return "TIMING_GAME_STATE";} 
};