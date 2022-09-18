#include <Arduino.h>
#include <FreeRTOS_SAMD21.h>


void k_usleep(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void k_msleep(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}