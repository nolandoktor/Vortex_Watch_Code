#include <Arduino.h>
#include <Wire.h>
#include "I2C_Helper.h"

//TODO: Add mutex locking to I2C commands

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
int i2c_update_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_val, uint8_t mask)
{
    uint8_t old_val;
    int ret = i2c_read_byte(dev_addr, reg_addr, &old_val);
    if (ret < 0) {
        return ret;
    }

    uint8_t new_val = (old_val & ~mask) | (reg_val & mask);
    return i2c_write_byte(dev_addr, reg_addr, new_val);
}
int i2c_bus_scan(uint8_t *addr_list, uint8_t *addr_cnt, uint8_t max_addr_cnt)
{
    int cnt = 0;
    if (addr_list == NULL) {
        return -1;
    }
    if (max_addr_cnt < 1) {
        return -1;
    }
    for (uint8_t i=0; i<128; i++) {
        if (cnt > max_addr_cnt) {
            return -2;
        }
        Wire.beginTransmission(i);
        if (Wire.endTransmission() != 0) {
            continue;
        }
        addr_list[cnt++] = i;
    }
    *addr_cnt = cnt;
    return 0;
}