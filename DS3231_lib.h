#ifndef DS3231_LIB
#define DS3231_LIB
#include <Arduino.h>
#include <Wire.h>
#include "GlobalDefines.h"

#define DS3231_I2C_ADDRESS 0x68
#define SEC_REG 0x00
#define MIN_REG 0x01
#define HOUR_REG 0x02
#define DAY_REG 0x03
#define DATE_REG 0x04
#define MONTH_REG 0x05
#define YEAR_REG 0x06

#define ALRM1_SEC_REG 0x07
#define ALRM1_MIN_REG 0x08
#define ALRM1_HOUR_REG 0x09
#define ALRM1_DAY_REG 0x0A
#define ALRM2_MIN_REG 0x0B
#define ALRM2_HOUR_REG 0x0C
#define ALRM2_DAY_REG 0x0D

#define CTRL_REG 0x0E
#define CTRL_STAT_REG 0x0F

#define AGE_REG 0x10
#define TEMP_MSB_REG 0x11
#define TEMP_LSB_REG 0x12





byte decToBcd(byte val)
{
  return ((val/10*16)+(val%10));
}
byte bcdToDec(byte val)
{
  return ((val/16*10)+(val%16));
}
void set1HzClock(byte set)
{
  int INTCN = 2;
  
  //Read value so you don't overwrite other data in register
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(CTRL_REG); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  
  byte reg_data = Wire.read();
  if (set)
    reg_data |= (1 << INTCN);
  else
    reg_data &= ~(1 << INTCN);

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(CTRL_REG); // set next input to start at the seconds register
  Wire.write(reg_data);
  Wire.endTransmission();
}

void initRTC()
{
  Wire.begin();
  set1HzClock(0);
  //TWBR = 10;
    //Not supported on M0
}
void set32KHzOut(byte set)
{
  //Read value so you don't overwrite other data in register
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(CTRL_STAT_REG); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  
  byte reg_data = Wire.read();
  if (set)
    reg_data |= (1 << 3);
  else
    reg_data &= ~(1 << 3);

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(CTRL_STAT_REG); // set next input to start at the seconds register
  Wire.write(reg_data);
  Wire.endTransmission();
}
byte checkControlStatReg()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(CTRL_STAT_REG); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  return Wire.read();
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void setDS3231time(byte second, byte minute, byte hour)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.endTransmission();
}
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  //TIMSK2 &= ~(1 << OCIE2A);
  //isrFlag = false;
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();

  //TIMSK2 |= (1 << OCIE2A);
  //delayMicroseconds(150);
  //TIMSK2 &= ~(1 << OCIE2A);
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);

  //TIMSK2 |= (1 << OCIE2A);
  //delayMicroseconds(150);
  //TIMSK2 &= ~(1 << OCIE2A);
  
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
  
  //TIMSK2 |= (1 << OCIE2A);
  //isrFlag = true;
}

void readDS3231time(byte *second,
byte *minute,
byte *hour)
{
  //TIMSK2 &= ~(1 << OCIE2A);
  //isrFlag = false;
  
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();

  //TIMSK2 |= (1 << OCIE2A);
  //delayMicroseconds(150);
  //TIMSK2 &= ~(1 << OCIE2A);
  
  Wire.requestFrom(DS3231_I2C_ADDRESS, 3);
  // request 3 bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);

  //TIMSK2 |= (1 << OCIE2A);
  //delayMicroseconds(150);
  //TIMSK2 &= ~(1 << OCIE2A);
  
  //TIMSK2 |= (1 << OCIE2A);
  //isrFlag = true;
}


byte readSeconds()
{
  int delay_ = 2;
  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    
  //TIMSK2 |= (1 << OCIE2A);
//  delay(delay_);
  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.write(SEC_REG);
    
  //TIMSK2 |= (1 << OCIE2A);
//  delay(delay_);
  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.endTransmission();
  
  //TIMSK2 |= (1 << OCIE2A);
//  delay(delay_);
  //TIMSK2 &= ~(1 << OCIE2A);

  
    Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  //TIMSK2 |= (1 << OCIE2A);  
  byte val = bcdToDec(Wire.read() & 0x7f);

  return val;
}


byte readMinutes()
{
  int delay_ = 2;
  //TIMSK2 &= ~(1 << OCIE2A);
    Wire.beginTransmission(DS3231_I2C_ADDRESS);

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.write(MIN_REG);

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);
    
    Wire.endTransmission();

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  //TIMSK2 |= (1 << OCIE2A);
  byte val = bcdToDec(Wire.read());

  return val;
}
byte readHours()
{
  int delay_ = 2;
  //TIMSK2 &= ~(1 << OCIE2A);
    Wire.beginTransmission(DS3231_I2C_ADDRESS);

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);  
    Wire.write(HOUR_REG);

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);
    
    Wire.endTransmission();

  //TIMSK2 |= (1 << OCIE2A);

  //TIMSK2 &= ~(1 << OCIE2A);
  
    Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  //TIMSK2 |= (1 << OCIE2A);
  byte val = bcdToDec(Wire.read() & 0x3f);
  
  return val;
}



void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
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
}



#endif
