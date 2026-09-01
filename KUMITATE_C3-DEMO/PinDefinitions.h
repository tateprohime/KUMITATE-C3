#pragma once
#include <Arduino.h>

constexpr uint8_t KUMITATE_PIN_LED = 0;
constexpr uint8_t KUMITATE_PIN_SDA = 1;
constexpr uint8_t KUMITATE_PIN_SCL = 2;
constexpr uint8_t KUMITATE_PIN_SW2 = 3;
constexpr uint8_t KUMITATE_PIN_EXT4 = 4;
constexpr uint8_t KUMITATE_PIN_EXT5 = 5;
constexpr uint8_t KUMITATE_PIN_EXT6 = 6;
constexpr uint8_t KUMITATE_PIN_EXT7 = 7;
constexpr uint8_t KUMITATE_PIN_SW4 = 8;
constexpr uint8_t KUMITATE_PIN_SW3 = 9;
constexpr uint8_t KUMITATE_PIN_RGB = 10;
constexpr uint8_t KUMITATE_PIN_BUZZER = 20;
constexpr uint8_t KUMITATE_PIN_SW1 = 21;

constexpr uint8_t KUMITATE_SWITCH_PINS[] = {
  KUMITATE_PIN_SW1, KUMITATE_PIN_SW2, KUMITATE_PIN_SW3, KUMITATE_PIN_SW4
};
constexpr uint8_t KUMITATE_RGB_COUNT = 4;
constexpr uint8_t KUMITATE_RGB_BRIGHTNESS = 8;
constexpr uint8_t KUMITATE_OLED_ADDRESS = 0x3C;
