#ifndef DOUBLE_BUFFER_LIB
#define DOUBLE_BUFFER_LIB

#include <Adafruit_NeoPixel.h>
#define NEOPIX_PIN 6
class DoubleBuffer
{
  private:
    Adafruit_NeoPixel *strip;  
    
  public:
    static const int R = 0, G = 1, B = 2;
    DoubleBuffer();
    void reset();
    void setColorVal(int idx, int color_idx, int val);
    void setColorVal(int idx, int rVal, int gVal, int bVal);
    //int getColorVal(int idx, int color_idx);
    void clear();
    void update();
};
DoubleBuffer::DoubleBuffer()
{
  strip = new Adafruit_NeoPixel(N_LEDS, NEOPIX_PIN);
  strip->begin();
  strip->show();
  
  //reset();
}
void DoubleBuffer::reset()
{
  strip->clear();
  strip->show();
  
  //buffer1.clear();
  //buffer2.clear();
  //wBuf = &buffer1;
  //rBuf = &buffer2;  
}
void DoubleBuffer::setColorVal(int idx, int color_idx, int val)
{
  uint32_t c = strip->getPixelColor(idx);
  uint8_t rgb[3];
  rgb[0] = (uint8_t)(c >> 16);
  rgb[1] = (uint8_t)(c >>  8);
  rgb[2] = (uint8_t)c;

  switch (color_idx)
  {
    case 0:
      strip->setPixelColor(idx, val, rgb[1], rgb[2]);
      break;
    case 1:
      strip->setPixelColor(idx, rgb[0], val, rgb[2]);
      break;
    case 2:
      strip->setPixelColor(idx, rgb[0], rgb[1], val);
      break;
  }
  
  //wBuf->setColorVal(idx, color_idx, val);
}
void DoubleBuffer::setColorVal(int idx, int rVal, int gVal, int bVal)
{
  strip->setPixelColor(idx, rVal, gVal, bVal);
  
  //wBuf->setColorVal(idx, rVal, gVal, bVal);
}
//int DoubleBuffer::getColorVal(int idx, int color_idx)
//{
  
  //return rBuf->getColorVal(idx, color_idx);
//}
void DoubleBuffer::clear()
{
  strip->clear();

  //wBuf->clear();
}
void DoubleBuffer::update()
{
  strip->show();
}

#endif
