#include "OledDisplay.h"
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(
    U8G2_R0, U8X8_PIN_NONE);

void oled_begin(uint8_t sdaPin, uint8_t sclPin)
{
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000);
  display.begin();
  display.enableUTF8Print();
}

void oled_show_startup(void)
{
  display.clearBuffer();
  display.setFont(u8g2_font_unifont_t_japanese1);
  display.drawUTF8(0, 20, "距離センサー");
  display.drawUTF8(0, 45, "起動しています...");
  display.sendBuffer();
}

void oled_show_measurement(float distanceCm, uint8_t motorPwm)
{
  display.clearBuffer();
  display.setFont(u8g2_font_unifont_t_japanese1);

  if (distanceCm < 0.0f) {
    display.drawUTF8(0, 15, "測定できません");
    display.drawUTF8(0, 38, "モーター：停止");
    display.sendBuffer();
    return;
  }

  char distanceText[32];
  char speedText[32];
  const int speedPercent = map(motorPwm, 0, 255, 0, 100);

  snprintf(distanceText, sizeof(distanceText), "距離：%.1f cm", distanceCm);
  snprintf(speedText, sizeof(speedText), "速度：%d%%", speedPercent);

  display.drawUTF8(0, 15, distanceText);
  display.drawUTF8(0, 37, motorPwm > 0 ? "モーター：回転中" : "モーター：停止");
  display.drawUTF8(0, 59, speedText);
  display.sendBuffer();
}
