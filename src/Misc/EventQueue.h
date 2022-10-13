#pragma once

#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>

#define EVENT_CTX_LEN 32
#define MAX_EVENTS 10

typedef enum
{
    B0_SHORT_PRESS,
    B0_LONG_PRESS,
    B1_SHORT_PRESS,
    B1_LONG_PRESS,
    ACCEL_SINGLE_TAP,
    ACCEL_DOUBLE_TAP
} event_type_t;

#define ANY_BUTTON_PRESS ((1 << B0_SHORT_PRESS) | (1 << B1_SHORT_PRESS) | (1 << B0_LONG_PRESS) | (1 << B1_LONG_PRESS))
#define ANY_SHORT_PRESS ((1 << B0_SHORT_PRESS) | (1 << B1_SHORT_PRESS))
#define ANY_LONG_PRESS ((1 << B0_LONG_PRESS) | (1 << B1_LONG_PRESS))

struct event_message {
    event_type_t event;
    uint8_t ctx[EVENT_CTX_LEN];
};

int init_event_queue();
QueueHandle_t get_event_queue();