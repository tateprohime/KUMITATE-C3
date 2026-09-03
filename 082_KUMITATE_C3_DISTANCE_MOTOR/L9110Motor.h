#pragma once

#include <stdint.h>

void l9110_begin(uint8_t inaPin, uint8_t inbPin);
void l9110_run(uint8_t pwm);
void l9110_stop(void);
