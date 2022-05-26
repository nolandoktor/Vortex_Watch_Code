#include <FastLED.h>
#include "DoubleBuffer.h"
#include "../Misc/GlobalDefines.h"
//#define NEOPIX_PIN 6

//static CRGB leds[N_LEDS];
/*
class DoubleBuffer
{
  private:
    //Adafruit_NeoPixel *strip;  
    
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
*/
DoubleBuffer::DoubleBuffer()
{
  FastLED.addLeds<NEOPIXEL, NEOPIX_PIN>(leds, N_LEDS);
  FastLED.clear(true);
}
void DoubleBuffer::reset()
{
  FastLED.clear(true);
}
void DoubleBuffer::setColorVal(int idx, int color_idx, int val)
{
  switch (color_idx)
  {
    case 0:
      leds[idx].r = val;
      break;
    case 1:
      leds[idx].g = val;
      break;
    case 2:
      leds[idx].b = val;
      break;
  }
}
void DoubleBuffer::setColorVal(int idx, int rVal, int gVal, int bVal)
{
  leds[idx].setRGB(rVal, gVal, bVal);
}
void DoubleBuffer::clear()
{
  FastLED.clear();
}
void DoubleBuffer::update()
{
  FastLED.show();
}

