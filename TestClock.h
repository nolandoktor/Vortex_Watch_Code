#ifndef TEST_CLK_LIB
#define TEST_CLK_LIB

#include <Arduino.h>
#include "DoubleBuffer.h"
#include "GlobalDefines.h"

class TestClock
{
  private:
    byte h, m, s;
    bool hour_24;
  public:
    TestClock() {}
    TestClock(byte h_, byte m_, byte s_);
  
    void initClock(byte h_, byte m_, byte s_) volatile;
    byte getSeconds() volatile {return s;}
    byte getMinutes() volatile {return m;}
    byte getHours() volatile;
    void use24HourTime(bool h_type) volatile {hour_24 = h_type;}
    void update() volatile;
    void printClock() volatile;
    //void draw(DoubleBuffer *lBuffer) volatile;
};

TestClock::TestClock(byte h_, byte m_, byte s_)
{
  initClock(h_, m_, s_);
}
void TestClock::initClock(byte h_, byte m_, byte s_) volatile
{
  h = h_;
  m = m_;
  s = s_;
  hour_24 = true;
}
byte TestClock::getHours() volatile
{
  if (hour_24)
  {
    return h;
  }
  else
  {
    if (h >= 12)
    {
      return h-12;
    }
  }
}
void TestClock::update() volatile
{
  s++;
  if (s >= 60)
  {
    s = 0;
    m++;
  }
  if (m >= 60)
  {
    m = 0;
    h++;
  }
  if (h >= 24)
  {
    h = 0;
  }
}
void TestClock::printClock() volatile
{
  Serial.print(getHours());
  Serial.print(":");
  Serial.print(getMinutes());
  Serial.print(":");
  Serial.println(getSeconds());

}
/*
void TestClock::draw(DoubleBuffer *lBuffer) volatile
{
  int sec_, min_, hour_;

  hour_ = h;
  if (h >= 12)
  {
    hour_ -= 12;
  }
  int min_rem = ((m%5 + 1)*COLOR_RES)/5;
  min_ = m/5;
  int sec_rem = ((s%5 + 1)*COLOR_RES)/5;
  sec_ = s/5;

  lBuffer->clear();
  if (hour_ < 12)
  { 
    for (int i=0; i<=hour_; i++)
    {
      lBuffer->setColorVal(i, 0, COLOR_RES);
    }
  }
  if (min_ < 12)
  {
    for (int i=0; i<min_; i++)
      lBuffer->setColorVal(i, 1, COLOR_RES);
    lBuffer->setColorVal(min_, 1, min_rem);
  }
  if (sec_ < 12)  
  {
    for (int i=0; i<sec_; i++)
      lBuffer->setColorVal(i, 2, COLOR_RES);
    lBuffer->setColorVal(sec_, 2, sec_rem);
  }
}
*/



#endif
