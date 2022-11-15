#pragma once

#include <Arduino.h>
#include "Revision.h"

#define BUTTON0_PIN 4
#define BUTTON1_PIN 3
#define LED_MOSFET_EN_PIN 5
#define LED_DAT_PIN 6
#define CLK_OE_PIN 8
#define CLK_1HZ_PIN 18
#define DEBUG_LED_PIN 13

#define ACCEL_INT1_PIN A1
#define ACCEL_INT2_PIN A2

#if BOARD_REVISION == REV_6V0
#error "Board rev = REV_6V0"
#elif BOARD_REVISION == REV_6V1
#define BATT_TEST_PIN A5
#else
#error "Invalid Board Revision"
#endif