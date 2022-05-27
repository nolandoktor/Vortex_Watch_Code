#include <FastLED.h>
#include "DoubleBuffer.h"
#include "../Misc/GlobalDefines.h"
#include "../Misc/PinMapping.h"

DoubleBuffer::DoubleBuffer()
{
  FastLED.addLeds<NEOPIXEL, LED_DAT_PIN>(leds, N_LEDS);
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

