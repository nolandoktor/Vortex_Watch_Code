#pragma once

#define N_LEDS 12
//#define COLOR_RES 10 /*28*/
#define COLOR_RES 60 //100
#define FPS 60
#define PRE_SCALE 8 
#define CLK_FREQ 8000000L
#define TIMER_CMP CLK_FREQ/(PRE_SCALE*FPS*N_LEDS*COLOR_RES)
#define TIMER_CMP_DEFAULT 80

//volatile int isrFlag = true;

typedef enum {
  SLEEP_STATE, 
  AWAKE_STATE, 
  SET_HOUR_STATE, 
  SET_MIN_STATE, 
  TIMING_GAME_STATE, 
  BATTERY_LEVEL_STATE, 
  NUM_WATCH_STATES
} watch_state_t;
