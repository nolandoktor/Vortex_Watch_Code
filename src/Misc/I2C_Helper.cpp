#include <Arduino.h>
#include <Wire.h>
#include "I2C_Helper.h"

int i2c_write_read(uint8_t dev_addr, uint8_t *wbuf, uint8_t wlen, uint8_t *rbuf, uint8_t rlen)
{
    int ret;
    ret = i2c_write(dev_addr, wbuf, wlen);
    if (ret < 0) {
        return ret;
    }
    Wire.requestFrom(dev_addr, rlen);
    for (int i=0; i<rlen; i++) {
        if (Wire.available()) {
            rbuf[i] = Wire.read();
        }
        else {
            return -1;
        }
    }
    return 0;
}
int i2c_write(uint8_t dev_addr, uint8_t *wbuf, uint8_t wlen)
{
    Wire.beginTransmission(dev_addr);
    Wire.write(wbuf, wlen);
    if (Wire.endTransmission() != 0) {
        return -1;
    }
    return 0;
}
int i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_val)
{
    return i2c_write_read(dev_addr, &reg_addr, 1, reg_val, 1);
}
int i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_val)
{
    uint8_t wbuf[2] = {
        reg_addr,
        reg_val
    };
    return i2c_write(dev_addr, wbuf, sizeof(wbuf));
}