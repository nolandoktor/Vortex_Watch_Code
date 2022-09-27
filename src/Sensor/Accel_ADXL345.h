#pragma once

void init_accel_task();

int accel_set_odr(float odr);
int accel_get_odr(float *odr);

int accel_set_tap_thresh(float thresh_mg);
int accel_get_tap_thresh(float *thresh_mg);

int accel_set_tap_duration(uint16_t ms);
int accel_get_tap_duration(uint16_t *ms);

int accel_set_tap_latent(uint16_t ms);
int accel_get_tap_latent(uint16_t *ms);

int accel_set_tap_window(uint16_t ms);
int accel_get_tap_window(uint16_t *ms);
