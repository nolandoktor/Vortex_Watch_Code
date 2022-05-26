#include <Arduino.h>
#include "WatchFace.h"
#include "TestClock.h"
#include "GlobalDefines.h"
#include "DoubleBuffer.h"

void StandardFace::draw(DoubleBuffer *lBuffer)
{
  int sec_, min_, hour_;

  hour_ = internalClock->getHours();
  if (internalClock->getHours() >= 12)
  {
    hour_ -= 12;
  }
  int min_rem = ((internalClock->getMinutes()%5/* + 1*/)*COLOR_RES)/5;
  min_ = internalClock->getMinutes()/5;
  int sec_rem = ((internalClock->getSeconds()%5/* + 1*/)*COLOR_RES)/5;
  sec_ = internalClock->getSeconds()/5;

  lBuffer->clear();
  if (hour_ < 12)
  { 
    //Serial.println("Hour data:");
    for (int i=0; i<hour_; i++)
    {
      lBuffer->setColorVal(i, 0, COLOR_RES);
    }
    lBuffer->setColorVal(hour_, 0, COLOR_RES);
  }
  if (min_ < 12)
  {
    for (int i=0; i<min_; i++)
      lBuffer->setColorVal(i, 1, COLOR_RES);
    lBuffer->setColorVal(min_, 1, min_rem);
  }
  if (sec_ < 12/* && sec_ticker == 1*/)  
  {
    for (int i=0; i<sec_; i++)
      lBuffer->setColorVal(i, 2, COLOR_RES);
    lBuffer->setColorVal(sec_, 2, sec_rem);
  }
}

//-------------------------------------------------------------------------

CascadeFace::CascadeFace(volatile TestClock *inClk) : WatchFace(inClk)
{
  timer_delay = 40;
  reset();
}
void CascadeFace::reset()
{
  s = internalClock->getSeconds();
  m = internalClock->getMinutes();
  h = internalClock->getHours();
  if (h >= 12)
    h -= 12;

  s_dest = s;
  m_dest = m;
  h_dest = h;

  s_change = false;
  m_change = false;
  h_change = false;

  s_idx = s/5;
  m_idx = m/5;
  h_idx = h;
 
  currentState = STATIC;
  timer = millis();

  //Serial.print(h);
  //Serial.print(":");
  //Serial.print(m);
  //Serial.print(":");
  //Serial.println(s);
}
void CascadeFace::update()
{

  switch (currentState)
  {
    case STATIC:
    {
      //Serial.println(STATIC);
      
      int temp_sec = internalClock->getSeconds();
      
      if (temp_sec != s)
      {
        s_idx = 11;
        s_dest = temp_sec;
        s_change = true;
      }
      int temp_min = internalClock->getMinutes();
      if (temp_min != m)
      {
        m_idx = 11;
        m_dest = temp_min;
        m_change = true;
      }
      int temp_hour = internalClock->getHours();
      if (temp_hour >= 12)
        temp_hour -= 12;
      if (temp_hour != h)
      {
        h_idx = 11;
        h_dest = temp_hour;
        h_change = true;
      }

      if (s_change || m_change || h_change)
      {
        currentState = CASCADE;
        //timer = millis();
      }
      break;
    }
    case CASCADE:
    {
      //Serial.println(CASCADE);
      if (millis() - timer < timer_delay)
        return;

      if (s_dest == 0)
        s_change = false;
      if (s_change)
      { 
        int dest_idx = s_dest/5;
        s_idx--;
        if (s_idx <= dest_idx)
        {
          s_idx = dest_idx;
          s_change = false;
        }
      }
      if (m_dest == 0)
        m_change = false;
      if (m_change)
      {
        int dest_idx = m_dest/5;
        m_idx--;
        if (m_idx <= dest_idx)
        {
          m_idx = dest_idx;
          m_change = false;
        }
      }
      if (h_dest == 0)
        h_change = false;
      if (h_change)
      {
        int dest_idx = h_dest;
        h_idx--;
        if (h_idx <= dest_idx)
        {
          h_idx = dest_idx;
          h_change = false;
        }
      }

      if (!s_change)
        s = s_dest;
      if (!m_change)
        m = m_dest;
      if (!h_change)
        h = h_dest;
      if (!s_change && !m_change && !h_change)
      {
        currentState = STATIC;
        //s = s_dest;
        //m = m_dest;
        //h = h_dest;
      }
    
      
        
      timer = millis();
      break;
    }
  }
}
void CascadeFace::draw(DoubleBuffer *lBuffer)
{
  lBuffer->clear();
  if (h_dest != 0)
    lBuffer->setColorVal(h_idx, DoubleBuffer::R, COLOR_RES/5);
  if (m_dest != 0)
  lBuffer->setColorVal(m_idx, DoubleBuffer::G, COLOR_RES/5);
  if (s_dest != 0)
    lBuffer->setColorVal(s_idx, DoubleBuffer::B, COLOR_RES/5);

  //int s_ = s/5;
  //int m_ = m/5;
  //int h_ = h;

  
  int sec_, min_, hour_;

  hour_ = h;
  if (h >= 12)
  {
    hour_ -= 12;
  }
  int min_rem = ((m%5)*COLOR_RES)/5;
  min_ = m/5;
  int sec_rem = ((s%5)*COLOR_RES)/5;
  sec_ = s/5;

  //lBuffer->clear();
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
  
  //Serial.print(hour_);
  //Serial.print(":");
  //Serial.print(m);
  //Serial.print(":");
  //Serial.println(s);
  
}

//------------------------------------------


FelixFace::FelixFace(volatile TestClock *inClk) : WatchFace(inClk)
{
  reset();
}
void FelixFace::reset()
{
  toggle_count = TOGGLE_MAX;
}
void FelixFace::update()
{
  toggle_count++;
  if (toggle_count >= TOGGLE_MAX)
    toggle_count = 0;

  noDelta = false;

  int hour_idx = internalClock->getHours();
  if (internalClock->getHours() >= 12)
  {
    hour_idx -= 12;
  }
  
  int min_val = internalClock->getMinutes();
  int min_idx = 0;
  if (min_val > 0)
  {
    min_val--;
    min_idx = min_val/5;
    min_val = min_val%5 + 1;
    min_val = (min_val*COLOR_RES)/5;
  }


  min_start = min_idx;
  min_end = hour_idx;
  hour_start = hour_idx;
  hour_end = min_idx;

  min_delta = min_end - min_start;
  if (min_delta == 0)
  {
    noDelta = true;
    return;
  }
  if (min_delta > 0)
    min_inc = 1;
  else
    min_inc = -1;
  if (min_delta > 6 || min_delta < -6)
    min_inc *= -1;
    
  hour_delta = hour_end - hour_start;
  if (hour_delta == 0)
  {
    noDelta = true;
    return;
  }
  if (hour_delta > 0)
    hour_inc = 1;
  else
    hour_inc = -1;
  if (hour_delta > 6 || hour_delta < -6)
    hour_inc *= -1;
    
}
void FelixFace::draw(DoubleBuffer *lBuffer)
{
  lBuffer->clear();

  if (noDelta)
  {
    if (getToggleState())
    {
      lBuffer->setColorVal(min_start, DoubleBuffer::G, COLOR_RES);
    }
    else
    {
      lBuffer->setColorVal(hour_start, DoubleBuffer::R, COLOR_RES);
    }
  }
  else
  {
    int min_val = COLOR_RES;
    int min_idx = min_start;
    while (min_idx != min_end)
    {
      lBuffer->setColorVal(min_idx, DoubleBuffer::G, min_val);
      min_val -= COLOR_RES/(min_delta*min_inc);//Get's absolute value of min_delta
      min_idx += min_inc;
      if (min_idx >= N_LEDS)
      {
        min_idx = 0;
      }
      if (min_idx < 0)
      {
        min_idx = N_LEDS - 1;
      }
    }

    int hour_val = COLOR_RES;
    int hour_idx = hour_start;
    while (hour_idx != hour_end)
    {
      lBuffer->setColorVal(hour_idx, DoubleBuffer::R, hour_val);
      hour_val -= COLOR_RES/(hour_delta*hour_inc);//Get's absolute value of hour_delta
      hour_idx += hour_inc;
      if (hour_idx >= N_LEDS)
      {
        hour_idx = 0;
      }
      if (hour_idx < 0)
      {
        hour_idx = N_LEDS - 1;
      }
    }
  }

  int sec_val = internalClock->getSeconds();
  int sec_idx = 0;
  if (sec_val > 0)
  {
    sec_val--;
    sec_idx = sec_val/5;
    sec_val = sec_val%5 + 1;
    sec_val = (sec_val*COLOR_RES)/5;
  }
  lBuffer->setColorVal(sec_idx, 0, 0, sec_val);
}