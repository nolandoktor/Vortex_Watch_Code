#pragma once

#include <Arduino.h>
#include "DoubleBuffer.h"
#include "../Time/TestClock.h"
#include "../Misc/GlobalDefines.h"


class WatchFace
{
  protected:
    volatile TestClock *internalClock;
    
  public:
    WatchFace(volatile TestClock *inClk) {internalClock = inClk;}

    virtual void reset()=0;
    virtual void update()=0;
    virtual void draw(DoubleBuffer *lBuffer)=0;
};

//-------------------------------------------------------------------------

class StandardFace : public WatchFace
{
  private:

  public:
    StandardFace(volatile TestClock *inClk) : WatchFace(inClk) {};
    void reset() {};
    void update() {};
    void draw(DoubleBuffer *lBuffer);
    
};

//-------------------------------------------------------------------------

class CascadeFace : public WatchFace
{
  private:
    int s, m, h;
    int s_idx, m_idx, h_idx;
    int s_cas, m_cas, h_cas;
    int s_dest, m_dest, h_dest;
    bool s_change, m_change, h_change;

    int timer_delay;
    long timer;

    int currentState;
    enum States{STATIC,CASCADE};
  public:
    CascadeFace(volatile TestClock *inClk);
    void reset();
    void update();
    void draw(DoubleBuffer *lBuffer);
};

//-------------------------------------------------------------------------

class FelixFace : public WatchFace
{
  private:
    int min_start, min_end, min_inc;
    int hour_start, hour_end, hour_inc;
    int min_delta, hour_delta;
    bool noDelta;
  
    int toggle_count;
    const int TOGGLE_MAX = 150;

    bool getToggleState() {return (toggle_count < TOGGLE_MAX/2);}
  public:
    FelixFace(volatile TestClock *inClk);
    void reset();
    void update();
    void draw(DoubleBuffer *lBuffer);  
};
