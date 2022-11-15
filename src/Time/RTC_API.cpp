#include <Arduino.h>
#include "RTC_API.h"

#ifdef USE_RV8803
static RV8803 rtc_rv8803;
#else

#endif


int rtc_init()
{
#ifdef USE_DS2331
    initRTC();
#elif USE_RV8803
    if (rtc_rv8803.begin() == false) 
        return -1;
    rtc_rv8803.setClockOutTimerFrequency(CLK_1HZ);
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_set_clkout(clock_freq_t clk_freq)
{
#ifdef USE_DS2331
    switch(clk_freq) {
        case CLK_32KHZ:
            set32KHzOut(1);
            break;
        case CLK_1024HZ:
            return -1;
            break;
        case CLK_1HZ:
            set1HzClock(0);
            break;
        case CLK_DISABLED:
            set32KHzOut(1);
            set1HzClock(1);
            break;
        default:
            return -1;
    }
#elif USE_RV8803
    rtc_rv8803.setClockOutTimerFrequency(clk_freq);
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_set_datetime(byte second, byte minute, byte hour, weekday_t dayOfWeek, 
                     byte dayOfMonth, byte month, byte year)
{
    if (dayOfWeek > SATURDAY)
        return -1;
#ifdef USE_DS2331
    setDS3231time(second, minute, hour, dayOfWeek + 1, dayOfMonth, month, year);
#elif USE_RV8803
    byte day = (1 << dayOfWeek);
    int year_ = year + 2000;
    if (rtc_rv8803.setTime(second, minute, hour, day, dayOfMonth, month, year_) == false)
        return -1;
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_get_datetime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, 
                     byte *dayOfMonth, byte *month, byte *year)
{
#ifdef USE_DS2331
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
#elif USE_RV8803
    if (rtc_rv8803.updateTime() == false)
        return -1;
    *second = rtc_rv8803.getSeconds();
    *minute = rtc_rv8803.getMinutes();
    *hour = rtc_rv8803.getHours();
    *dayOfMonth = rtc_rv8803.getDate();
    *dayOfWeek = rtc_rv8803.getWeekday();
    *month = rtc_rv8803.getMonth();
    int year_ = rtc_rv8803.getYear();
    *year = (byte)(year_ - 2000);
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_set_time(byte second, byte minute, byte hour)
{
#ifdef USE_DS2331
    setDS3231time(second, minute, hour);
#elif USE_RV8803
    uint8_t time[3];
    time[0] = rtc_rv8803.DECtoBCD(second);
    time[1] = rtc_rv8803.DECtoBCD(minute);
    time[2] = rtc_rv8803.DECtoBCD(hour);
    rtc_rv8803.setTime(time, 3);
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_get_time(byte *second, byte *minute, byte *hour)
{
#ifdef USE_DS2331
    readDS3231time(second, minute, hour);
#elif USE_RV8803
    if (rtc_rv8803.updateTime() == false)
        return -1;
    *second = rtc_rv8803.getSeconds();
    *minute = rtc_rv8803.getMinutes();
    *hour = rtc_rv8803.getHours();
#else
    Serial.println("Error: No valid RTC selected");
    return -1;
#endif
    return 0;
}
int rtc_display_time()
{
    Serial.println("rtc_display_time");
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  if (rtc_get_datetime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year) < 0)
    return -1;
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }

  String currentDate = rtc_rv8803.stringDateUSA();
  Serial.println(currentDate);
  return 0;
}

