#pragma once

#include <LiFuelGauge.h>
#include "StateManager.h"
#include "../Misc/GlobalDefines.h"
#include "../Time/TestClock.h"
#include "../Display/DoubleBuffer.h"
#include "../Display/WatchFace.h"

class StateManager;
class StateElement {
  protected:
    static const int SLEEP_TIMEOUT = 150000;
    StateManager *state_manager;
    DoubleBuffer *frame_buffer;

  public:
    StateElement(StateManager *sm, DoubleBuffer *fb);
    virtual int init() {return 0;}
    virtual int on_enter(watch_state_t prev_state) {return 0;}
    virtual int on_exit(watch_state_t next_state) {return 0;}
    virtual int update() {return 0;}
    virtual const char* get_name() {return "INVALID_STATE";} 
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
    AwakeState(StateManager *sm, DoubleBuffer *fb, WatchFace *watchFace);
    int init();
    int on_enter(watch_state_t prev_state);
    int update();
    const char* get_name() {return "AWAKE_STATE";} 
};
