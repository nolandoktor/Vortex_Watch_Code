#ifndef STATE_ELEMENT_LIB
#define STATE_ELEMENT_LIB

#include <LiFuelGauge.h>
#include "DoubleBuffer.h"
#include "GlobalDefines.h"
#include "StateManager.h"
#include "TestClock.h"
#include "WatchFace.h"

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

#endif