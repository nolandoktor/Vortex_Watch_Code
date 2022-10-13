#pragma once

#include <Arduino.h>
#include "../Display/DoubleBuffer.h"
#include "../Misc/GlobalDefines.h"

class Game
{
  protected:
    int N;
  public:
    Game() {}
    virtual void reset()=0;
    virtual void update(uint16_t events)=0;
    virtual void draw(DoubleBuffer *lBuffer)=0;
    virtual bool gameIsOver()=0;
};

class TG_Node
{
  private:
    int pos;
    int dir;
    int type;
    bool ranOff;
    bool active;
    
  public:
    enum TYPES{NORMAL, UP, DOWN, NO_HIT, NUM_TYPES};
    enum DIRECTIONS{CW, CCW, NUM_DIRECTIONS};

    TG_Node();
    TG_Node(int pos_, int dir_, int type_);
    void update();
    void draw(DoubleBuffer *lBuffer);
    
    int getPos() {return pos;}
    int getType() {return type;}
    bool getRanOff() {return ranOff;}
    void setActive(bool active_) {active = active_;}
    bool isActive() {return active;}
};

class TimingGame : public Game
{
  private:
    long update_timer;
    int pos;
    int score;
    int lives;
    int goodFlash;
    int badFlash;
    int spawnTimer;
    int spawnSpread;
    int spawnOffset;
    
    int state;
    bool gameOver;
    bool endGame;
    int tally;
    int ridx, gidx, bidx;
    int typeCounters[TG_Node::NUM_TYPES];
    int difficulty;
    int level_speed;

    const double BREATH_MIN = 0.5;
    double breath_val;
    int breath_dir;

    const static int NUM_NODES = 6;
    TG_Node nodes[NUM_NODES];
    
    const static int FLASH_VAL = COLOR_RES;
    const static int FLASH_FALLOFF = COLOR_RES/10;
    const static int TUTORIAL_VAL = 1;
    const static int MAX_LEVEL_SPEED = 150;
    
    const static int LEVEL_SPEED_DELTA = 3;
    enum STATES{INTRO, GAMEPLAY, END, NUM_STATES};
    enum DIFFICULTY{EASY, MEDIUM, HARD, NUM_DIFFICULTIES};
    const int INITIAL_LEVEL_SPEEDS[NUM_DIFFICULTIES] = {350, 325, 275};
    const int SCORE_DIFFICULTY_TRIGGERS[NUM_DIFFICULTIES] = {12, 36, -1};
  public:
    TimingGame();
    void reset();
    void update(uint16_t events);
    void draw(DoubleBuffer *lBuffer);
    bool gameIsOver() {return endGame;}


    void intro();
    void gameplay();
    void end();

    void spawnNode(int pos, int dir, int type);
    void goodTrigger(bool real=true);
    void badTrigger(bool real=true);
};

