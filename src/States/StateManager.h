#pragma once

#include "StateElement.h"
#include "../Misc/GlobalDefines.h"

#define STATE_MANAGER_SCRATCH 64

class StateElement;
class StateManager
{
  private:
    StateElement *state_list[NUM_WATCH_STATES];
    watch_state_t current_state;
    uint8_t scratch[STATE_MANAGER_SCRATCH] __attribute__((aligned(4)));
  public:
    StateManager();
    int init(watch_state_t start_state);
    int assign_state(watch_state_t state, StateElement *element); 
    int change_state(watch_state_t next_state);
    int update();
    const char* get_state_name(watch_state_t state);
    uint8_t *get_scratch() {return scratch;}
};