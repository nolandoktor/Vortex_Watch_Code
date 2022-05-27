#pragma once

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
