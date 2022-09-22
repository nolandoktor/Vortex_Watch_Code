#pragma once

#include <Arduino.h>

int i2c_write_read(uint8_t dev_addr, uint8_t *wbuf, uint8_t wlen, uint8_t *rbuf, uint8_t rlen);
int i2c_write(uint8_t dev_addr, uint8_t *wbuf, uint8_t wlen);
int i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_val);
int i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_val);
int i2c_update_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_val, uint8_t mask);
int i2c_bus_scan(uint8_t *addr_list, uint8_t *addr_cnt, uint8_t max_addr_cnt);