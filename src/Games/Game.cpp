#include <Arduino.h>
#include "../Display/DoubleBuffer.h"
#include "../Input/ButtonHandler.h"
#include "../Misc/GlobalDefines.h"
#include "../Misc/EventQueue.h"
#include "Game.h"

TG_Node::TG_Node()
{
  active = false;
  ranOff = false;
}

TG_Node::TG_Node(int pos_, int dir_, int type_)
{
  pos = pos_;
  dir = dir_;
  type = type_;
  ranOff = false;
  active = true;
}

void TG_Node::update()
{
  switch(dir)
  {
    case CW:
    {
      pos++;
      if (pos >= N_LEDS)
      {
        ranOff = true;
      }
      break;
    }
    case CCW:
    {
      pos--;
      if (pos < 0)
      {
        pos = N_LEDS - 1;
      }
      if (pos == N_LEDS - 2)
      {
        ranOff = true;
      }
      break;
    }
  }
}
void TG_Node::draw(DoubleBuffer *lBuffer)
{
  if (!active)
    return;
  
  switch (type)
  {
    case NORMAL:
    {
      lBuffer->setColorVal(pos, COLOR_RES, COLOR_RES, COLOR_RES);
      break;
    }
    case UP:
    {
      lBuffer->setColorVal(pos, 0, COLOR_RES, COLOR_RES);
      break;
    }
    case DOWN:
    {
      lBuffer->setColorVal(pos, COLOR_RES, 0, COLOR_RES);
      break; 
    }
    case NO_HIT:
    {
      lBuffer->setColorVal(pos, COLOR_RES, 0, 0);
      break;
    }
  }
}

TimingGame::TimingGame()
{
  reset();
}
void TimingGame::reset()
{
  for (int i=0; i<NUM_NODES; i++)
  {
    nodes[i].setActive(false);
  }
  for (int i=0; i<TG_Node::NUM_TYPES; i++)
  {
    typeCounters[i] = 0;
  }
  pos = 5;
  score = 0;
  lives = 10;
  goodFlash = 0;
  badFlash = 0;
  spawnTimer = 0;
  spawnSpread = 0;
  spawnOffset = 6;
  
  //state = GAMEPLAY;
  state = INTRO;
  gameOver = false;
  endGame = false;
  tally = 0;
  ridx = 0;
  gidx = 0;
  bidx = 0;

  difficulty = EASY;
  level_speed = INITIAL_LEVEL_SPEEDS[EASY];
  
  breath_val = 1.0;
  breath_dir = -1;
  //spawnNode(5, rand()%TG_Node::NUM_DIRECTIONS, /*TG_Node::NORMAL*/rand()%TG_Node::NUM_TYPES);
  //update_timer = millis();
}
void TimingGame::update(uint16_t events)
{
  if (events & ANY_LONG_PRESS)
  {
    Serial.println("Manual exit");
    resetButtonStates();
    endGame = true;
    return;
  }
  switch(state)
  {
    case INTRO:
    {
      Serial.println("Game Intro");
      intro(events);
      break;
    }
    case GAMEPLAY:
    {
      Serial.println("Gameplay");
      gameplay(events);
      break;
    }
    case END:
    {
      Serial.println("Game End");
      end(events);
      break;
    }
  }
}
void TimingGame::draw(DoubleBuffer *lBuffer)
{
  lBuffer->clear();
  switch(state)
  {
    case INTRO:
    {
      //break;
    }
    case GAMEPLAY:
    { 
      //lBuffer->clear();
      for (int i=0; i<N_LEDS; i++)
      {
        if (goodFlash)
        {
          lBuffer->setColorVal(i, DoubleBuffer::G, goodFlash);  
        }
        if (badFlash)
        {
          lBuffer->setColorVal(i, DoubleBuffer::R, badFlash);
        }
      }
    
      lBuffer->setColorVal(N_LEDS-1, 3*COLOR_RES/5, 3*COLOR_RES/5, 0);
      for (int i=0; i<NUM_NODES; i++)
        nodes[i].draw(lBuffer);
      //lBuffer->update();
      break;
    }
    case END:
    {
      for (int i=0; i<ridx; i++)
      {
        lBuffer->setColorVal(i, DoubleBuffer::R, (int)(COLOR_RES*breath_val));
      }
      for (int i=0; i<gidx; i++)
      {
        lBuffer->setColorVal(i, DoubleBuffer::G, (int)(COLOR_RES*breath_val));
      }
      for (int i=0; i<bidx; i++)
      {
        lBuffer->setColorVal(i, DoubleBuffer::B, (int)(COLOR_RES*breath_val));
      }
      break;
    }
  }
  lBuffer->update();
}
void TimingGame::intro(uint16_t events)
{
  if (goodFlash)
  {
    goodFlash -= FLASH_FALLOFF;
    if (goodFlash < 0)
      goodFlash = 0;
  }
  if (badFlash)
  {
    badFlash -= FLASH_FALLOFF;
    if (badFlash < 0)
      badFlash = 0;
  }
  
  if (events & ANY_BUTTON_PRESS)
  {
    bool atZeroPos = false;
    for (int i=0; i<NUM_NODES; i++)
    {
      if (nodes[i].isActive() && nodes[i].getPos() == N_LEDS-1)
      {
        atZeroPos = true;
        
        switch (nodes[i].getType())
        {
          case TG_Node::NORMAL:
          {
            goodTrigger(false);
            typeCounters[TG_Node::NORMAL]++;
            break;
          }
          case TG_Node::UP:
          {
            if (events & (1 << B0_SHORT_PRESS))
            {
              goodTrigger(false);
              typeCounters[TG_Node::UP]++;
            }
            else
            {
              badTrigger(false);
            }
            break;
          }
          case TG_Node::DOWN:
          {
            if (events & (1 << B1_SHORT_PRESS))
            {
              goodTrigger(false);
              typeCounters[TG_Node::DOWN]++;
            }
            else
            {
              badTrigger(false);
            }
            break;
          }
          case TG_Node::NO_HIT:
          {
            badTrigger(false);
            break;
          }
        }
        nodes[i].setActive(false);
      }
    }
    if (!atZeroPos)
    {
      badTrigger();
    }
  }
  resetButtonStates();

  if (millis() - update_timer < /*300*/ level_speed)
  {
    return;
  }
  update_timer = millis();
  for (int i=0; i<NUM_NODES; i++)
  {
    if (nodes[i].isActive())
    {
      nodes[i].update();
      if (nodes[i].getRanOff())
      {
        if (nodes[i].getType() != TG_Node::NO_HIT)
        {
          badTrigger(false);
        }
        else
        {
          goodTrigger(false); 
        }
        nodes[i].setActive(false);
      }
    }
  }
  if (typeCounters[TG_Node::NORMAL] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CCW, TG_Node::NORMAL);
  }
  else if (typeCounters[TG_Node::UP] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CCW, TG_Node::UP);
  }
  else if (typeCounters[TG_Node::DOWN] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CW, TG_Node::DOWN);
  }
  else
  {
    spawnSpread = 3;
    spawnOffset = 2;
    spawnNode(5, rand()%TG_Node::NUM_DIRECTIONS, /*TG_Node::NORMAL*/rand()%TG_Node::NUM_TYPES);
    update_timer = millis();
    state = GAMEPLAY;
  }
  
  //spawnNode(5, rand()%TG_Node::NUM_DIRECTIONS, /*TG_Node::NORMAL*/rand()%TG_Node::NUM_TYPES);
  if (gameOver)
    state = END;
}
void TimingGame::gameplay(uint16_t events)
{
  if (goodFlash)
  {
    goodFlash -= FLASH_FALLOFF;
    if (goodFlash < 0)
      goodFlash = 0;
  }
  if (badFlash)
  {
    badFlash -= FLASH_FALLOFF;
    if (badFlash < 0)
      badFlash = 0;
  }
  
  if (events & ANY_BUTTON_PRESS)
  {
    bool atZeroPos = false;
    for (int i=0; i<NUM_NODES; i++)
    {
      if (nodes[i].isActive() && nodes[i].getPos() == N_LEDS-1)
      {
        atZeroPos = true;
        
        switch (nodes[i].getType())
        {
          case TG_Node::NORMAL:
          {
            goodTrigger();
            break;
          }
          case TG_Node::UP:
          {
            if (events & (1 << B0_SHORT_PRESS))
            {
              goodTrigger();
            }
            else
            {
              badTrigger();
            }
            break;
          }
          case TG_Node::DOWN:
          {
            if (events & (1 << B1_SHORT_PRESS))
            {
              goodTrigger();
            }
            else
            {
              badTrigger();
            }
            break;
          }
          case TG_Node::NO_HIT:
          {
            badTrigger();
            break;
          }
        }
        nodes[i].setActive(false);
      }
    }
    if (!atZeroPos)
    {
      badTrigger();
    }
  }
  resetButtonStates();

  if (millis() - update_timer < /*300*/ level_speed)
  {
    return;
  }
  update_timer = millis();
  for (int i=0; i<NUM_NODES; i++)
  {
    if (nodes[i].isActive())
    {
      nodes[i].update();
      if (nodes[i].getRanOff())
      {
        if (nodes[i].getType() != TG_Node::NO_HIT)
        {
          badTrigger();
        }
        else
        {
          goodTrigger(); 
        }
        nodes[i].setActive(false);
      }
    }
  }
/*
if (typeCounters[TG_Node::NORMAL] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CCW, TG_Node::NORMAL);
  }
  else if (typeCounters[TG_Node::UP] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CCW, TG_Node::UP);
  }
  else if (typeCounters[TG_Node::DOWN] < TUTORIAL_VAL)
  {
    spawnNode(5, TG_Node::CW, TG_Node::DOWN);
  }
*/
  switch (difficulty)
  {
    case EASY:
    {
      spawnSpread = 2;
      spawnOffset = 3;
      spawnNode(5, TG_Node::CCW, TG_Node::NORMAL);
      if (score >= SCORE_DIFFICULTY_TRIGGERS[EASY])
      {
        spawnSpread = 3;
        spawnOffset = 2;
        difficulty = MEDIUM;
      }
      break;
    }
    case MEDIUM:
    {
      spawnSpread = 3;
      spawnOffset = 2;

      
      int type = rand()%(TG_Node::NUM_TYPES-1);
      int dir;
      if (type == TG_Node::NORMAL)
      {
        dir = rand()%TG_Node::NUM_DIRECTIONS;
      }
      else if (type == TG_Node::UP)
      {
        dir = TG_Node::CCW;
      }
      else
      {
        dir = TG_Node::CW;
      }
      
      spawnNode(5, dir, type);
      if (score >= SCORE_DIFFICULTY_TRIGGERS[MEDIUM])
      {
        spawnSpread = 3;
        spawnOffset = 1;
        difficulty = HARD;
      }
      break;
    }
    case HARD:
    {
      spawnSpread = 3;
      spawnOffset = 1;
      int type = rand()%TG_Node::NUM_TYPES;
      int dir = rand()%TG_Node::NUM_DIRECTIONS;     
      spawnNode(5, dir, type);
      break;
    }
  }
  //spawnNode(5, rand()%TG_Node::NUM_DIRECTIONS, /*TG_Node::NORMAL*/rand()%TG_Node::NUM_TYPES);
  if (gameOver)
    state = END;
}
void TimingGame::end(uint16_t events)
{
  if (millis() - update_timer < 50)
    return;
  update_timer = millis();
  
  if (tally < score)
  {
    tally++;
    ridx++;
    if (ridx > N_LEDS)
    {
      ridx = 0;
      gidx++;
    }
    if (gidx > N_LEDS)
    {
      gidx = 0;
      bidx++;
    }
  }
  else
  {
    if (breath_dir < 0)
    {
      breath_val -= 0.05;
    }
    if (breath_dir > 0)
    {
      breath_val += 0.05;
    }
    if (breath_val < BREATH_MIN)
    {
      breath_val = BREATH_MIN;
      breath_dir *= -1;
    }
    if (breath_val > 1.0)
    {
      breath_val = 1.0;
      breath_dir *= -1;
    }
    
    if (events & ANY_BUTTON_PRESS)
    {
      endGame = true; 
    }
  }
  resetButtonStates();
}
void TimingGame::spawnNode(int pos, int dir, int type)
{
  if ((pos < N_LEDS) && (pos >=0) && (dir < TG_Node::NUM_DIRECTIONS) && (dir >= 0) && (type < TG_Node::NUM_TYPES) && (type >= 0))
  {
    if (spawnTimer == 0)
    {
      for (int i=0; i<NUM_NODES; i++)
      {
        if (!nodes[i].isActive())
        {
          //TG_Node newNode(5, rand()%2, /*TG_Node::NORMAL*/rand()%4);
          TG_Node newNode(pos, dir, type);
          nodes[i] = newNode;
          break;
        }
      }
    }
  }
  spawnTimer--;
  if (spawnTimer < 0)
  {
    if (spawnSpread > 0)
      spawnTimer = rand()%spawnSpread + spawnOffset;
    else
      spawnTimer = spawnOffset;
  }
}
void TimingGame::goodTrigger(bool real)
{
  if (real)
  {
    score++;
    level_speed -= LEVEL_SPEED_DELTA;
    if (level_speed < MAX_LEVEL_SPEED)
    {
      level_speed = MAX_LEVEL_SPEED;
    }
  }
  goodFlash = FLASH_VAL;
  badFlash = 0;
}
void TimingGame::badTrigger(bool real)
{
  badFlash = FLASH_VAL;
  goodFlash = 0;
  if (real)
  {
    lives--;
    if (lives < 0)
    {
      lives = 0;
      gameOver = true;
    }
  }
}
