#pragma once

#define USE_RV8803 true
#if USE_RV8803
#include <SparkFun_RV8803.h>
#else
#include "DS3231_lib.h"
#endif

typedef enum {
    CLK_32KHZ = 0,
    CLK_1024HZ = 1, 
    CLK_1HZ = 2,
    CLK_DISABLED = 4,
} clock_freq_t;
typedef enum {
    RTC_SUNDAY = 0,
    RTC_MONDAY,
    RTC_TUESDAY,
    RTC_WEDNESDAY,
    RTC_THURSDAY,
    RTC_FRIDAY,
    RTC_SATURDAY,
} weekday_t;

int rtc_init();
int rtc_set_clkout(clock_freq_t clk_freq);
int rtc_set_datetime(byte second, byte minute, byte hour, weekday_t dayOfWeek, 
                     byte dayOfMonth, byte month, byte year);
int rtc_get_datetime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, 
                     byte *dayOfMonth, byte *month, byte *year);
int rtc_set_time(byte second, byte minute, byte hour);
int rtc_get_time(byte *second, byte *minute, byte *hour);
int rtc_display_time();
