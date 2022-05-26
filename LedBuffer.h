#ifndef LED_BUFFER_LIB
#define LED_BUFFER_LIB

#include <Arduino.h>

#include "src/Misc/GlobalDefines.h"

//#define N_LEDS 12


class LedBuffer 
{
  private:
    int ledBuffer[N_LEDS][3];
    
  public:
    enum COLORS{RED, GREEN, BLUE};
    LedBuffer();
    void setColorVal(int idx, int color_idx, int val);
    void setColorVal(int idx, int rVal, int gVal, int bVal);
    int getColorVal(int idx, int color_idx);
    void clear();
    void copyFrom(LedBuffer* src);
};
LedBuffer::LedBuffer()
{
  clear();
}
void LedBuffer::setColorVal(int idx, int color_idx, int val)
{
  ledBuffer[idx][color_idx] = val;
}
void LedBuffer::setColorVal(int idx,int rVal, int gVal, int bVal)
{
  ledBuffer[idx][RED] = rVal;
  ledBuffer[idx][GREEN] = gVal;
  ledBuffer[idx][BLUE] = bVal;
}
int LedBuffer::getColorVal(int idx, int color_idx)
{
  return ledBuffer[idx][color_idx];
}
void LedBuffer::clear()
{
  for (int i=0; i<N_LEDS; i++)
  {
    for (int j=0; j<3; j++)
    { 
      ledBuffer[i][j] = 0; 
    }
  }
}
void LedBuffer::copyFrom(LedBuffer* src)
{
  for (int i=0; i<N_LEDS; i++)
  {
    for (int j=0; j<3; j++)
    { 
      ledBuffer[i][j] = src->getColorVal(i, j); 
    }
  }
}

#endif


