#pragma once

#include <stdint.h>

void hcsr04_begin(uint8_t trigPin, uint8_t echoPin);
float hcsr04_measure_cm(void);
