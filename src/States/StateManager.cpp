#include <Arduino.h>
#include "StateManager.h"
#include "StateElement.h"
#include "../Misc/GlobalDefines.h"
#include "../Input/ButtonHandler.h"

StateManager::StateManager()
{
    for (int i=0; i<NUM_WATCH_STATES; i++) {
        state_list[i] = NULL;
    }
}
int StateManager::init(watch_state_t start_state)
{
  Serial.println("State manager init");
  bool init_success = true;
  for (int i=0; i<NUM_WATCH_STATES; i++) {
    Serial.println(i);
    if (state_list[i] != NULL) {
      if (state_list[i]->init() < 0) {
        init_success = false;
        Serial.print("Failed to init state: ");
        Serial.println(state_list[i]->get_name());
      }
    }
  }

  //At least one state inits failed
  if (!init_success) {
    return -1;
  }
  if (state_list[start_state] == NULL) {
    return -1;
  }
  current_state = start_state;
  return state_list[current_state]->on_enter(current_state);
}
int StateManager::assign_state(watch_state_t state, StateElement *element)
{
  state_list[state] = element;
  return 0;
}
int StateManager::change_state(watch_state_t next_state)
{
  int ret;
  if (state_list[current_state] == NULL) {
    return -1;
  }
  if (state_list[next_state] == NULL) {
    return -1;
  }

  ret = state_list[current_state]->on_exit(next_state);
  Serial.print("Exiting State: ");
  Serial.println(state_list[current_state]->get_name());
  if (ret < 0) {
    return ret;
  }
  
  watch_state_t prev_state = current_state;
  current_state = next_state;

  Serial.print("Entering State: ");
  Serial.println(state_list[current_state]->get_name());
  ret = state_list[current_state]->on_enter(prev_state);
  if (ret < 0) {
    return ret;
  }
  return 0;
}
int StateManager::update()
{
    if (state_list[current_state] == NULL) {
        return -1;
    }
    int ret = state_list[current_state]->update();
    if (ret < 0) {
        return ret;
    }
    Serial.print("Auto reset inputs: ");
    Serial.println(state_list[current_state]->get_auto_input_reset());
    if (state_list[current_state]->get_auto_input_reset()) {
        resetButtonStates();
    }
    return 0;
}
const char* StateManager::get_state_name(watch_state_t state)
{
  if (state_list[state] == NULL) {
      return NULL;
  }
  return state_list[state]->get_name();
}