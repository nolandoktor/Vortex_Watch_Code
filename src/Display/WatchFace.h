#pragma once

#include <Arduino.h>
#include "DoubleBuffer.h"
#include "../Time/TestClock.h"
#include "../Misc/GlobalDefines.h"

typedef enum {
  WATCH_FACE_STANDARD, 
  WATCH_FACE_CASCADE, 
  WATCH_FACE_FELIX, 
  NUM_WATCH_FACES
} watch_face_t;

class WatchFace
{
  protected:
    volatile TestClock *internalClock;
    
  public:
    WatchFace(volatile TestClock *inClk) {internalClock = inClk;}

    virtual int reset()=0;
    virtual void update()=0;
    virtual void draw(DoubleBuffer *lBuffer)=0;
    virtual const char* get_name() {return "INVALID_WATCH_FACE";}
};

//-------------------------------------------------------------------------

class StandardFace : public WatchFace
{
  private:

  public:
    StandardFace(volatile TestClock *inClk) : WatchFace(inClk) {};
    int reset() {return 0;}
    void update() {};
    void draw(DoubleBuffer *lBuffer);
    const char* get_name() {return "STANDARD_FACE";}
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
    int reset();
    void update();
    void draw(DoubleBuffer *lBuffer);
    const char* get_name() {return "CASCADE_FACE";}
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
    int reset();
    void update();
    void draw(DoubleBuffer *lBuffer);
    const char* get_name() {return "FELIX_FACE";}  
};
