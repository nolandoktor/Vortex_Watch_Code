#include <LiFuelGauge.h>
#include <ArduinoLowPower.h>
#include "src/Display/DoubleBuffer.h"
#include "src/Misc/PinMapping.h"
#include "src/Misc/GlobalDefines.h"
#include "src/Time/TestClock.h"
#include "src/Display/WatchFace.h"
#include "src/Input/ButtonHandler.h"
#include "src/Games/Game.h"
#include "src/Time/RTC_API.h"
#include "src/States/StateElement.h"
#include "src/States/StateManager.h"

// Watch Objects
volatile TestClock test_clock;
DoubleBuffer *frame_buffer = NULL;
Game *watch_game = NULL;
LiFuelGauge *gauge = NULL;
WatchFace *watch_face = NULL;
WatchFace *watch_faces[NUM_WATCH_FACES] = {NULL, NULL, NULL};

// State Objects
StateManager state_manager;
SleepState *sleep_state = NULL;
AwakeState *awake_state = NULL;
SetHourState *set_hour_state = NULL;
SetMinuteState *set_minute_state = NULL;
BatteryState *battery_state = NULL;
TimingGameState *timing_game_state = NULL;

//Global Variables
volatile bool tick_ready = false;
static const byte second_init = 0;
static const byte minute_init = 0;
static const byte hour_init = 0;
static const weekday_t dayOfWeek_init = RTC_THURSDAY;
static const byte dayOfMonth_init = 2;
static const byte month_init = 6;
static const byte year_init = 22;

// Function Prototypes
void goToSleep();
void sleepNow();
void enter_sleep_mode();
void wake_from_sleep();
void tickClock();


void setup() {
  // Init Serial
  Serial.begin(115200);
  delay(2000);
  Serial.println("Bootloader running...");
  delay(4000);
  Serial.println("Exiting bootloader...");  

  // GPIO Config - Pin Direction  
  pinMode(DEBUG_LED_PIN, OUTPUT);
  pinMode(LED_MOSFET_EN_PIN, OUTPUT);
  pinMode(CLK_1HZ_PIN, INPUT_PULLUP);

  // GPIO Config - Output
  digitalWrite(LED_MOSFET_EN_PIN, HIGH);

  // Instantiate Objects
  gauge = new LiFuelGauge(MAX17043);
  frame_buffer = new DoubleBuffer();
  watch_game = new TimingGame();
  watch_faces[WATCH_FACE_STANDARD] = new StandardFace(&test_clock);
  watch_faces[WATCH_FACE_CASCADE] = new CascadeFace(&test_clock);
  watch_faces[WATCH_FACE_FELIX] = new FelixFace(&test_clock);
  watch_face = watch_faces[WATCH_FACE_CASCADE];

  // Instantiate States
  sleep_state = new SleepState(&state_manager, frame_buffer, &test_clock, gauge);
  awake_state = new AwakeState(&state_manager, frame_buffer, watch_face);
  set_hour_state = new SetHourState(&state_manager, frame_buffer, &test_clock);
  set_minute_state = new SetMinuteState(&state_manager, frame_buffer, &test_clock);
  battery_state = new BatteryState(&state_manager, frame_buffer, gauge);
  timing_game_state = new TimingGameState(&state_manager, frame_buffer, watch_game);

  // Assign States to state manager
  state_manager.assign_state(SLEEP_STATE, (StateElement*)sleep_state);
  state_manager.assign_state(AWAKE_STATE, (StateElement*)awake_state);
  state_manager.assign_state(SET_HOUR_STATE, (StateElement*)set_hour_state);
  state_manager.assign_state(SET_MIN_STATE, (StateElement*)set_minute_state);
  state_manager.assign_state(BATTERY_LEVEL_STATE, (StateElement*)battery_state);
  state_manager.assign_state(TIMING_GAME_STATE, (StateElement*)timing_game_state);

  // Init RTC
  byte sec_, min_, hour_;
  rtc_init();
  rtc_set_datetime(second_init, minute_init, hour_init, dayOfWeek_init, 
                   dayOfMonth_init, month_init, year_init);
  rtc_get_time (&sec_, &min_, &hour_);
  test_clock.initClock((int)hour_, (int)min_, (int)sec_);

  // Init Objects
  gauge->reset();
  watch_game->reset();
  watch_face->reset();
  frame_buffer->reset();

  // Init States
  if (state_manager.init(AWAKE_STATE) < 0) {
    Serial.println("State Manager init failed");
  }
  else {
    Serial.println("State Manager init successful");
  }
  
  // Setup button and 1Hz clock interrupts
  attachInterrupt(digitalPinToInterrupt(CLK_1HZ_PIN), tickClock, RISING);
  initButtonHandler();
}

void loop() 
{
  state_manager.update();
  delay(MAIN_LOOP_DELAY);
}



//-------------------------------------------

void goToSleep()
{
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //sleep_enable();
  //sleep_mode(); 
    //Not supported on M0
  
}
void sleepNow()
{
    /*
    //AVR sleep functions or IRQ enable/disable not supported on M0
    cli();
  
    // Choose our preferred sleep mode:
    //set_sleep_mode(SLEEP_MODE_IDLE);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
    // Set sleep enable (SE) bit:
    sleep_enable();

    sei();
 
    // Put the device to sleep:
    sleep_mode();
 
    // Upon waking up, sketch continues from this point.
    sleep_disable();
    */
   LowPower.deepSleep();
}
void enter_sleep_mode()
{
  detachInterrupt(digitalPinToInterrupt(CLK_1HZ_PIN));
  /*
  turnOffDrawingISR();
  turnOffSinks();
  //turnOffSources();
  for (int i=0; i<2; i++)
  {
    digitalWrite(rPWM_[i], 1);
    digitalWrite(gPWM_[i], 1);
    digitalWrite(bPWM_[i], 1);
  }
  */
  //frame_buffer->clear();
  //frame_buffer->update();
  pinMode(LED_DAT_PIN, INPUT_PULLUP);
  digitalWrite(LED_MOSFET_EN_PIN, LOW);
  //set1HzClock(1);
  rtc_set_clkout(CLK_DISABLED);
  //goToSleep();
  
  //digitalWrite(DEBUG_LED_PIN, HIGH);
  //delay(1000);
  //PCICR &= ~(1 << PCIE0);
  sleepNow();
  //PCICR |= (1 << PCIE0);
  //digitalWrite(DEBUG_LED_PIN, LOW);
}
void wake_from_sleep()
{
  
  attachInterrupt(digitalPinToInterrupt(CLK_1HZ_PIN), tickClock, RISING);
  //set1HzClock(0);
  rtc_set_clkout(CLK_1HZ);
  pinMode(LED_DAT_PIN, OUTPUT);
  digitalWrite(LED_MOSFET_EN_PIN, HIGH);
}

void tickClock()
{
  test_clock.update();
  tick_ready = true;
}




