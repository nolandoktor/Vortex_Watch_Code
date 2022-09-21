#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>
#include "FreeTouch/Adafruit_FreeTouch_alt.h"
#include "ButtonHandler.h"
#include "../Misc/Delay.h"

#define TOUCH_STACK_SIZE 512

#define TOUCH_TIME_DELTA 33
#define HYST_HIGH 400
#define HYST_LOW 365
typedef enum
{
    TOUCH_SW2,
    NUM_TOUCH_PADS
} touch_pad_t;

TaskHandle_t xTouchTask = NULL;

static Adafruit_FreeTouch_alt qt[NUM_TOUCH_PADS] = {
    Adafruit_FreeTouch_alt(PORTA, 3, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE),
    // TODO: Add another definition for switch 1
};
static uint16_t prev_touch[NUM_TOUCH_PADS];
bool button_pressed[NUM_TOUCH_PADS] = {false};
static volatile uint32_t timers[NUM_TOUCH_PADS] = {0};

static void touchTask(void *pvParameters);

void init_touch_task()
{
    xTaskCreate(
        touchTask,
        (const portCHAR *)"Touch_Task", // A name just for humans
        TOUCH_STACK_SIZE,               // Stack size
        NULL,                           // No Parameters
        1,                              // priority
        &xTouchTask);
}

static void touchTask(void *pvParameters)
{
    (void)pvParameters;
    for (uint8_t pad = 0; pad < NUM_TOUCH_PADS; pad++)
    {
        qt[pad].begin();
        prev_touch[pad] = qt[pad].measure();
    }

    while (1)
    {
        for (uint8_t pad = 0; pad < NUM_TOUCH_PADS; pad++)
        {
            uint16_t current_touch = qt[pad].measure();
            int delta = (int)current_touch - (int)prev_touch[pad];
            prev_touch[pad] = current_touch;

            uint32_t event_ts = millis();
            if (current_touch > HYST_HIGH && !button_pressed[pad])
            {
                Serial.print(event_ts);
                Serial.print(": Button ");
                Serial.print(pad);
                Serial.println(" Pressed\n");
                button_pressed[pad] = true;

                timers[pad] = event_ts;
            }
            else if (current_touch < HYST_LOW && button_pressed[pad])
            {
                Serial.print(event_ts);
                Serial.print(": Button ");
                Serial.print(pad);
                Serial.println(" Released\n");
                button_pressed[pad] = false;

                int32_t delta = event_ts - timers[pad];
                if (delta < 500) {
                    Serial.println("Short Press");
                    shortPresses[1] = true;
                }
                else{
                    Serial.println("Long Press");
                    longPresses[1] = true;
                }
            }
        }
        k_msleep(TOUCH_TIME_DELTA);
    }
}