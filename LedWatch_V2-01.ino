#include <FreeRTOS_SAMD21.h>
#include <LiFuelGauge.h>
#include <ArduinoLowPower.h>
#include "src/Display/DoubleBuffer.h"
#include "src/Misc/PinMapping.h"
#include "src/Misc/GlobalDefines.h"
#include "src/Misc/Delay.h"
#include "src/Time/TestClock.h"
#include "src/Display/WatchFace.h"
#include "src/Display/WatchFaceManager.h"
#include "src/Input/ButtonHandler.h"
#include "src/Games/Game.h"
#include "src/Time/RTC_API.h"
#include "src/States/StateElement.h"
#include "src/States/StateManager.h"
#include "src/Input/CLI_Input.h"
#include "src/Input/TouchInput.h"
#include "src/Sensor/Accel_ADXL345.h"

#define MAIN_TASK_STACK_SIZE 2*configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

// Watch Objects
volatile TestClock test_clock;
DoubleBuffer *frame_buffer = NULL;
Game *watch_game = NULL;
LiFuelGauge *gauge = NULL;
WatchFaceManager watch_face_manager;
StandardFace *standard_face = NULL;
CascadeFace *cascade_face = NULL;
FelixFace *felix_face = NULL;

// State Objects
StateManager state_manager;
SleepState *sleep_state = NULL;
AwakeState *awake_state = NULL;
SetHourState *set_hour_state = NULL;
SetMinuteState *set_minute_state = NULL;
BatteryState *battery_state = NULL;
TimingGameState *timing_game_state = NULL;
FaceSelectState *face_select_state = NULL;
FlashLightState *flash_light_state = NULL;

//Global Variables
volatile bool tick_ready = false;
static const byte second_init = 0;
static const byte minute_init = 0;
static const byte hour_init = 0;
static const weekday_t dayOfWeek_init = RTC_THURSDAY;
static const byte dayOfMonth_init = 2;
static const byte month_init = 6;
static const byte year_init = 22;

//Created Tasks
TaskHandle_t xMainTask = NULL;
TaskHandle_t xBlinkTask = NULL;
extern TaskHandle_t xCLITask;
extern TaskHandle_t xTouchTask;
extern TaskHandle_t xAccelTask;

// Function Prototypes
void goToSleep();
void sleepNow();
void enter_sleep_mode();
void wake_from_sleep();
void tickClock();

void TaskWatchMain(void *pvParameters);
void TaskBlink(void *pvParameters);

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
  pinMode(CLK_OE_PIN, OUTPUT);

  // GPIO Config - Output
  digitalWrite(LED_MOSFET_EN_PIN, HIGH);
  digitalWrite(CLK_OE_PIN, HIGH);

  // Instantiate Objects
  gauge = new LiFuelGauge(MAX17043);
  frame_buffer = new DoubleBuffer();
  watch_game = new TimingGame();
  standard_face = new StandardFace(&test_clock);
  cascade_face = new CascadeFace(&test_clock);
  felix_face = new FelixFace(&test_clock);

  // Assign Watch Faces
  watch_face_manager.assign_face(WATCH_FACE_STANDARD, (WatchFace*)standard_face);
  watch_face_manager.assign_face(WATCH_FACE_CASCADE, (WatchFace*)cascade_face);
  watch_face_manager.assign_face(WATCH_FACE_FELIX, (WatchFace*)felix_face);

  // Instantiate States
  sleep_state = new SleepState(&state_manager, frame_buffer, &test_clock, gauge);
  awake_state = new AwakeState(&state_manager, frame_buffer, &watch_face_manager);
  set_hour_state = new SetHourState(&state_manager, frame_buffer, &test_clock);
  set_minute_state = new SetMinuteState(&state_manager, frame_buffer, &test_clock);
  battery_state = new BatteryState(&state_manager, frame_buffer, gauge);
  timing_game_state = new TimingGameState(&state_manager, frame_buffer, watch_game);
  face_select_state = new FaceSelectState(&state_manager, frame_buffer, &watch_face_manager);
  flash_light_state = new FlashLightState(&state_manager, frame_buffer);

  // Assign States to state manager
  state_manager.assign_state(SLEEP_STATE, (StateElement*)sleep_state);
  state_manager.assign_state(AWAKE_STATE, (StateElement*)awake_state);
  state_manager.assign_state(SET_HOUR_STATE, (StateElement*)set_hour_state);
  state_manager.assign_state(SET_MIN_STATE, (StateElement*)set_minute_state);
  state_manager.assign_state(BATTERY_LEVEL_STATE, (StateElement*)battery_state);
  state_manager.assign_state(TIMING_GAME_STATE, (StateElement*)timing_game_state);
  state_manager.assign_state(FACE_SELECT_STATE, (StateElement*)face_select_state);
  state_manager.assign_state(FLASH_LIGHT_STATE, (StateElement*)flash_light_state);

  // Init RTC
  byte sec_, min_, hour_;
  if (rtc_init() < 0) {
    Serial.println("Error: Failed to initialize RTC");
  }
  rtc_set_datetime(second_init, minute_init, hour_init, dayOfWeek_init, 
                   dayOfMonth_init, month_init, year_init);
  rtc_get_time(&sec_, &min_, &hour_);
  test_clock.initClock((int)hour_, (int)min_, (int)sec_);

  // Init Objects
  gauge->reset();
  watch_game->reset();
  frame_buffer->reset();

  // Init Watch Faces
  if (watch_face_manager.init(WATCH_FACE_CASCADE) < 0) {
    Serial.println("WatchFace Manager init failed");
  }
  else {
    Serial.println("WatchFace Manager init successful");
  }

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


  // Create FreeRTOS Tasks
  xTaskCreate(
    TaskWatchMain,
    (const portCHAR *)"Watch_Main",   // A name just for humans
    MAIN_TASK_STACK_SIZE,        // Stack size
    NULL,
    2,          // priority
    &xMainTask
  );

  
  xTaskCreate(
    TaskBlink,
    (const portCHAR *)"Blink",   // A name just for humans
    BLINK_TASK_STACK_SIZE,        // Stack size
    NULL,
    1,          // priority
    &xBlinkTask
  );
  
  init_cli_task();
  init_touch_task();
  init_accel_task();
  
  vTaskStartScheduler();
}

void loop() 
{
  /*
  state_manager.update();
  delay(MAIN_LOOP_DELAY);
  */
}

//-------------------------------------------

void TaskBlink(void *pvParameters) 
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(DEBUG_LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    k_msleep(1000);          // wait for one second
    digitalWrite(DEBUG_LED_PIN, LOW);    // turn the LED off by making the voltage LOW
    k_msleep(1000);          // wait for one second

    UBaseType_t wm_blink = uxTaskGetStackHighWaterMark(xBlinkTask);
    UBaseType_t wm_main = uxTaskGetStackHighWaterMark(xMainTask);
    UBaseType_t wm_cli = uxTaskGetStackHighWaterMark(xCLITask);
    UBaseType_t wm_touch = uxTaskGetStackHighWaterMark(xTouchTask);
    UBaseType_t wm_accel = uxTaskGetStackHighWaterMark(xAccelTask);

    Serial.print("Main: ");
    Serial.println(wm_main);
    Serial.print("Blink: ");
    Serial.println(wm_blink);
    Serial.print("CLI: ");
    Serial.println(wm_cli);
    Serial.print("Touch: ");
    Serial.println(wm_touch);
    Serial.print("Accel: ");
    Serial.println(wm_accel);
    Serial.println();
  }
}

void TaskWatchMain(void *pvParameters)
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    state_manager.update();
    k_msleep(MAIN_LOOP_DELAY);

    //Serial.println("TaskWatchMain");
    //k_msleep(1000);
  }
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




