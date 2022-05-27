#pragma once

#include <FastLED.h>
#include "../Misc/GlobalDefines.h"
#define NEOPIX_PIN 6

class DoubleBuffer
{
  private:
    CRGB leds[N_LEDS];  
    
  public:
    static const int R = 0, G = 1, B = 2;
    DoubleBuffer();
    void reset();
    void setColorVal(int idx, int color_idx, int val);
    void setColorVal(int idx, int rVal, int gVal, int bVal);
    CRGB getColorVal(int idx) {return leds[idx];}
    void clear();
    void update();
};
