#include <Arduino.h>
#include "StateManager.h"
#include "StateElement.h"
#include "../Misc/GlobalDefines.h"
#include "../Input/ButtonHandler.h"
#include "../Misc/EventQueue.h"

StateManager::StateManager()
{
    for (int i=0; i<NUM_WATCH_STATES; i++) {
        state_list[i] = NULL;
    }
    current_state = (watch_state_t)0;
}
int StateManager::init(watch_state_t start_state)
{
  Serial.println("State manager init");

  event_mask = 0;

  event_queue = get_event_queue();
  if (event_queue == NULL) {
    Serial.print("Error: State manager failed to get handle to event queue");
    return -1;
  }

  bool init_success = true;
  for (int i=0; i<NUM_WATCH_STATES; i++) {
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
    int ret;
    if (state_list[current_state] == NULL) {
        return -1;
    }

    UBaseType_t num_events = uxQueueMessagesWaiting(event_queue);
    struct event_message new_event;
    for (int i=0; i<num_events; i++) {
      if(xQueueReceive(event_queue, &new_event, (TickType_t)0) == pdFALSE) {
        Serial.println("Error: Failed to receive message from event queue");
        continue;
      }
      switch(new_event.event) {
        case B0_SHORT_PRESS:
          Serial.println("B0_SHORT_PRESS");
          break;
        case B0_LONG_PRESS:
          Serial.println("B0_LONG_PRESS");
          break;
        case B1_SHORT_PRESS:
          Serial.println("B1_SHORT_PRESS");
          break;
        case B1_LONG_PRESS:
          Serial.println("B1_LONG_PRESS");
          break;
        case ACCEL_SINGLE_TAP:
          Serial.println("ACCEL_SINGLE_TAP");
          break;
        case ACCEL_DOUBLE_TAP:
          Serial.println("ACCEL_DOUBLE_TAP");
          break;
        default:
          Serial.print("Error: Invalid state ");
          Serial.println(new_event.event);
          break;
      }
      event_mask |= (1 << new_event.event);
    }

    ret = state_list[current_state]->update();
    if (ret < 0) {
        return ret;
    }
    if (state_list[current_state]->get_auto_input_reset()) {
        resetButtonStates();
        event_mask = 0;
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