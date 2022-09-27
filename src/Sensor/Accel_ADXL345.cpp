#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>
#include <Adafruit_ADXL345_U.h>
#include "Accel_ADXL345.h"
#include "../Misc/I2C_Helper.h"
#include "../Misc/Delay.h"
#include "../Misc/PinMapping.h"

#define ADXL345_INT_DRDY (1 << 7)
#define ADXL345_INT_SINGLE_TAP (1 << 6)
#define ADXL345_INT_DOUBLE_TAP (1 << 5)
#define ADXL345_INT_ACTIVITY (1 << 4)
#define ADXL345_INT_INACTIVITY (1 << 3)
#define ADXL345_INT_FREEFALL (1 << 2)
#define ADXL345_INT_WATERMARK (1 << 1)
#define ADXL345_INT_OVERRUN (1 << 0)

#define ADXL345_DATA_FMT_INT_INVERT (1 << 5)

#define ACCEL_TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)
#define ODR_MAP_SIZE 16
#define ACCEL_THRESH_LSB (62.5)
#define ACCEL_DURATION_LSB (0.625)
#define ACCEL_WINDOW_LSB (1.25)
/**********************************************
 *               Type Definitions 
 **********************************************/ 
typedef enum
{
    ADXL345_INT1,
    ADXL345_INT2
} adxl345_int_source_t;


/**********************************************
 *            Static/Global Variables 
 **********************************************/ 
TaskHandle_t xAccelTask = NULL;
static Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
static SemaphoreHandle_t xSemaphore;
static const float ODR_MAP[ODR_MAP_SIZE] = {
    0.1,
    0.2,
    0.39,
    0.78,
    1.56,
    3.13,
    6.25,
    12.5,
    25.0,
    50.0,
    100.0,
    200.0,
    400.0,
    800.0,
    1600.0,
    3200.0,
};

/**********************************************
 *              Function Prototypes 
 **********************************************/ 
static void accelTask(void *pvParameters);
static void accel_int1_handler();


/**********************************************
 *               Public Functions 
 **********************************************/ 
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

int accel_set_odr(float odr)
{
    dataRate_t data_rate;
    if (odr <= ODR_MAP[0]) {
        data_rate = ADXL345_DATARATE_0_10_HZ;
    }
    if (odr >= ODR_MAP[15]) {
        data_rate = ADXL345_DATARATE_3200_HZ;
    }

    int i;
    for (i=1; i<ODR_MAP_SIZE; i++) {
        if (odr < ODR_MAP[i]) {
            break;
        }
    }
    Serial.print("i = ");
    Serial.println(i);
    float delta_low = odr - ODR_MAP[i-1];
    float delta_high = ODR_MAP[i] - odr;

    if (delta_low < delta_high) {
        data_rate = (dataRate_t)(i-1);
    }
    else {
        data_rate = (dataRate_t)i;
    }

    accel.setDataRate(data_rate);
    return 0;
}
int accel_get_odr(float *odr)
{
    dataRate_t data_rate = accel.getDataRate();
    *odr = ODR_MAP[data_rate];
    return 0;
}
int accel_set_tap_thresh(float thresh_mg)
{
    uint8_t reg;
    if (thresh_mg <= 0) {
        reg = 0;
    }
    int raw_val = thresh_mg / ACCEL_THRESH_LSB;
    Serial.println(raw_val);
    if (raw_val > 0xFF) {
        reg = 0xFF;
    }
    else {
        reg = (uint8_t)raw_val;
    }
    Serial.println(reg);
    return i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_THRESH_TAP, reg);
}
int accel_get_tap_thresh(float *thresh_mg)
{
    int ret;
    uint8_t reg;
    ret = i2c_read_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_THRESH_TAP, &reg);
    if (ret < 0) {
        return ret;
    }
    Serial.println(reg);
    *thresh_mg = reg * ACCEL_THRESH_LSB;
    return 0;
}
int accel_set_tap_duration(uint16_t ms)
{
    uint8_t reg;
    int raw_val = (int)(ms / ACCEL_DURATION_LSB);
    if (raw_val > 0xFF) {
        reg = 0xFF;
    }
    else {
        reg = (uint8_t)raw_val;
    }
    return i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_DUR, reg);
}
int accel_get_tap_duration(uint16_t *ms)
{
    int ret;
    uint8_t reg;
    ret = i2c_read_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_DUR, &reg);
    if (ret < 0) {
        return ret;
    }
    *ms = (uint16_t)(reg * ACCEL_DURATION_LSB);
    return 0;
}
int accel_set_tap_latent(uint16_t ms)
{
    uint8_t reg;
    int raw_val = (int)(ms / ACCEL_WINDOW_LSB);
    if (raw_val > 0xFF) {
        reg = 0xFF;
    }
    else {
        reg = (uint8_t)raw_val;
    }
    return i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_LATENT, reg);
}
int accel_get_tap_latent(uint16_t *ms)
{
    int ret;
    uint8_t reg;
    ret = i2c_read_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_LATENT, &reg);
    if (ret < 0) {
        return ret;
    }
    *ms = (uint16_t)(reg * ACCEL_WINDOW_LSB);
    return 0;
}
int accel_set_tap_window(uint16_t ms)
{
    uint8_t reg;
    int raw_val = (int)(ms / ACCEL_WINDOW_LSB);
    if (raw_val > 0xFF) {
        reg = 0xFF;
    }
    else {
        reg = (uint8_t)raw_val;
    }
    return i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_WINDOW, reg);
}
int accel_get_tap_window(uint16_t *ms)
{
    int ret;
    uint8_t reg;
    ret = i2c_read_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_WINDOW, &reg);
    if (ret < 0) {
        return ret;
    }
    *ms = (uint16_t)(reg * ACCEL_WINDOW_LSB);
    return 0;
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

/**********************************************
 *               Private Functions 
 **********************************************/ 
// TODO: Use message queue instead of semaphore to pass type of interrupt to main thread
static void accel_int1_handler()
{
    xSemaphoreGiveFromISR(xSemaphore, NULL);
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
    accel.setDataRate(ADXL345_DATARATE_400_HZ/*ADXL345_DATARATE_12_5_HZ*/);

    // Set interrupt to be active LOW
    ret = i2c_update_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_DATA_FORMAT, ADXL345_DATA_FMT_INT_INVERT, ADXL345_DATA_FMT_INT_INVERT);
    if (ret < 0)
    {
        Serial.println("Error: Failed to set accel interrupt to active LOW");
        vTaskDelete(xAccelTask);
    }

    //Clear interrupt pins to init IMU
    i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_INT_ENABLE, 0x00);

    uint8_t int_mask = ADXL345_INT_SINGLE_TAP;//ADXL345_INT_DRDY;

    //Configure TAP settings
    i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_THRESH_TAP, 0x40);
    i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_DUR, 0x08);
    i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_LATENT, 0x30); 
    i2c_write_byte(ADXL345_DEFAULT_ADDRESS, ADXL345_REG_TAP_AXES, 0x01); 

    //accel_set_odr(300);
    accel_set_tap_thresh(12000.0);
    //accel_set_tap_duration(1);
    accel_set_tap_latent(500);
    accel_set_tap_window(72);

    float odr, thresh_mg;
    uint16_t dur_ms, latent_ms, window_ms;
    accel_get_odr(&odr);
    accel_get_tap_thresh(&thresh_mg);
    accel_get_tap_duration(&dur_ms);
    accel_get_tap_latent(&latent_ms);
    accel_get_tap_window(&window_ms);

    Serial.print("ODR: ");
    Serial.println(odr);
    Serial.print("Thresh (mg): ");
    Serial.println(thresh_mg);
    Serial.print("Duration (ms): ");
    Serial.println(dur_ms);
    Serial.print("Latent (ms): ");
    Serial.println(latent_ms);
    Serial.print("Window (ms): ");
    Serial.println(window_ms);

    ret = set_interrupt_mapping(int_mask, ADXL345_INT1);
    if (ret < 0)
    {
        Serial.println("Error: Failed to set accel interrupt mapping");
        vTaskDelete(xAccelTask);
    }
    attachInterrupt(ACCEL_INT1_PIN, accel_int1_handler, FALLING);
    ret = enable_interrupts(int_mask);
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

        if ((int_src & ADXL345_INT_DRDY) && false) {
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
            Serial.println("Double tap detected");
        }
        if (int_src & ADXL345_INT_SINGLE_TAP) {
            uint32_t ts = millis();
            uint32_t delta = ts - ts_prev;
            ts_prev = ts;
            if (delta >= 25) {
                Serial.print("Single tap detected: ");
                Serial.println(delta);
            }
            else {
                Serial.println("Tap Echo");
            }
        }
    }
}
