#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>
#include <Adafruit_ADXL345_U.h>
#include "Accel_ADXL345.h"
#include "../Misc/I2C_Helper.h"
#include "../Misc/Delay.h"
#include "../Misc/PinMapping.h"

//#define ADXL345_DEFAULT_ADDRESS (0x53)

#define ADXL345_INT_DRDY (1 << 7)
#define ADXL345_INT_SINGLE_TAP (1 << 6)
#define ADXL345_INT_DOUBLE_TAP (1 << 5)
#define ADXL345_INT_ACTIVITY (1 << 4)
#define ADXL345_INT_INACTIVITY (1 << 3)
#define ADXL345_INT_FREEFALL (1 << 2)
#define ADXL345_INT_WATERMARK (1 << 1)
#define ADXL345_INT_OVERRUN (1 << 0)

#define ADXL345_DATA_FMT_INT_INVERT (1 << 5)

#define ACCEL_TASK_STACK_SIZE 2 * configMINIMAL_STACK_SIZE

typedef enum
{
    ADXL345_INT1,
    ADXL345_INT2
} adxl345_int_source_t;

TaskHandle_t xAccelTask = NULL;
static Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
static SemaphoreHandle_t xSemaphore;

static void accelTask(void *pvParameters);
static void drdy_interrupt_handler();

void init_accel_task()
{
    xTaskCreate(
        accelTask,
        (const portCHAR *)"Accel_Task", // A name just for humans
        ACCEL_TASK_STACK_SIZE,          // Stack size
        NULL,                           // No Parameters
        3,                              // priority
        &xAccelTask);
}

int enable_interrupts(uint8_t int_mask)
{
    return i2c_update_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_INT_ENABLE, 0xFF, int_mask);
}
int disable_interrupts(uint8_t int_mask)
{
    return i2c_update_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_INT_ENABLE, 0x00, int_mask);
}
int set_interrupt_mapping(uint8_t int_mask, adxl345_int_source_t src)
{
    uint8_t val = 0x00;
    if (src == ADXL345_INT2)
    {
        val = 0xFF;
    }
    return i2c_update_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_INT_MAP, val, int_mask);
}
int get_interrupt_source(uint8_t *src_mask)
{
    uint8_t src;
    int ret = i2c_read_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_INT_SOURCE, &src);
    if (ret < 0)
    {
        return ret;
    }
    *src_mask = src;
    return 0;
}

void accelTask(void *pvParameters)
{
    (void)pvParameters;
    int ret = 0;

    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
        Serial.println("Error: Could not allocate semaphore");
        vTaskDelete(xAccelTask);
    }

    if (!accel.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.println("Error: No ADXL345 detected. Check your wiring!");

        // Delete thread
        vTaskDelete(xAccelTask);
    }
    accel.setRange(ADXL345_RANGE_2_G);
    accel.setDataRate(ADXL345_DATARATE_12_5_HZ);

    // Set interrupt to be active LOW
    ret = i2c_update_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_DATA_FORMAT, ADXL345_DATA_FMT_INT_INVERT, ADXL345_DATA_FMT_INT_INVERT);
    if (ret < 0)
    {
        Serial.println("Error: Failed to set accel interrupt to active LOW");
        vTaskDelete(xAccelTask);
    }

    ret = set_interrupt_mapping(ADXL345_INT_DRDY, ADXL345_INT1);
    if (ret < 0)
    {
        Serial.println("Error: Failed to set accel interrupt mapping");
        vTaskDelete(xAccelTask);
    }
    attachInterrupt(ACCEL_INT1_PIN, drdy_interrupt_handler, FALLING);
    ret = enable_interrupts(ADXL345_INT_DRDY);
    if (ret < 0)
    {
        Serial.println("Error: Failed to enable accel interrupts");
        vTaskDelete(xAccelTask);
    }

    sensors_event_t event;
    float x, y, z;
    uint32_t ts_prev = 0;
    uint8_t int_src = 0;
    
    // Clear interrupt flags prior to entering loop
    if (get_interrupt_source(&int_src) < 0) {
        Serial.println("Error: Failed to read interrupt source");
        vTaskDelete(xAccelTask);
    }
    else if (int_src & ADXL345_INT_DRDY) {
        accel.getEvent(&event);
    }
    while (1)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdFALSE) {
            Serial.println("Error: Failed to take semaphore");
            continue;
        }
        if (get_interrupt_source(&int_src) < 0) {
            Serial.println("Error: Failed to read interrupt source");
        }

        if (int_src & ADXL345_INT_DRDY) {
            uint32_t ts = millis();
            uint32_t delta = ts - ts_prev;
            ts_prev = ts;
            accel.getEvent(&event);
            x = event.acceleration.x;
            y = event.acceleration.y;
            z = event.acceleration.z;

            Serial.print("DRDY interrupt: ");
            Serial.println(delta);
            Serial.print(x);
            Serial.print("  ");
            Serial.print(y);
            Serial.print("  ");
            Serial.print(z);
            Serial.println("\n");
        }
        if (int_src & ADXL345_INT_DOUBLE_TAP) {
            Serial.print("Double tap detected");
        }
    }
}

uint32_t ts_prev = 0;
static void drdy_interrupt_handler()
{
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}