#ifndef GLOBAL_DEFINES
#define GLOBAL_DEFINES

#define N_LEDS 12
//#define COLOR_RES 10 /*28*/
#define COLOR_RES 60 //100
#define FPS 60
#define PRE_SCALE 8 
#define CLK_FREQ 8000000L
#define TIMER_CMP CLK_FREQ/(PRE_SCALE*FPS*N_LEDS*COLOR_RES)
#define TIMER_CMP_DEFAULT 80

volatile int isrFlag = true;

#endif


