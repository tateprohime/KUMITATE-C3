#pragma once

#include <stdint.h>

void oled_begin(uint8_t sdaPin, uint8_t sclPin);
void oled_show_startup(void);
void oled_show_measurement(float distanceCm, uint8_t motorPwm);
