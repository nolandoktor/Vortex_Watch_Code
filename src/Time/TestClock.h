#ifndef TEST_CLK_LIB
#define TEST_CLK_LIB

//#include <Arduino.h>
//#include "DoubleBuffer.h"
//#include "../Misc/GlobalDefines.h"

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
};
/*
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
  Serial.print("Clock tick: ");
  Serial.println(millis());
}
void TestClock::printClock() volatile
{
  Serial.print(getHours());
  Serial.print(":");
  Serial.print(getMinutes());
  Serial.print(":");
  Serial.println(getSeconds());

}
*/
#endif
