#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Preferences.h>
#include "PinDefinitions.h"
#include "FactoryTest.h"
#include "DemoProgram.h"

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Adafruit_NeoPixel rgb(KUMITATE_RGB_COUNT, KUMITATE_PIN_RGB, NEO_GRB + NEO_KHZ800);
FactoryTest factoryTest(oled, rgb);
DemoProgram demoProgram(oled, rgb);
U8G2_FOR_ADAFRUIT_GFX bootText;

enum AppMode { MODE_DEMO, MODE_FACTORY };
AppMode appMode = MODE_DEMO;

enum BootAction { BOOT_NORMAL, BOOT_FACTORY, BOOT_RESET_RECORDS };

BootAction waitForBootAction() {
  constexpr uint32_t menuTimeMs = 5000;
  constexpr uint32_t holdTimeMs = 900;
  uint32_t menuStart = millis();
  uint32_t comboStart = 0;
  BootAction candidate = BOOT_NORMAL;

  while (millis() - menuStart < menuTimeMs) {
    BootAction current = BOOT_NORMAL;
    if (digitalRead(KUMITATE_PIN_SW1) == LOW && digitalRead(KUMITATE_PIN_SW4) == LOW)
      current = BOOT_FACTORY;
    else if (digitalRead(KUMITATE_PIN_SW2) == LOW && digitalRead(KUMITATE_PIN_SW3) == LOW)
      current = BOOT_RESET_RECORDS;

    if (current == BOOT_NORMAL) {
      candidate = BOOT_NORMAL;
      comboStart = 0;
    } else if (current != candidate) {
      candidate = current;
      comboStart = millis();
    } else if (millis() - comboStart >= holdTimeMs) {
      return candidate;
    }
    delay(10);
  }
  return BOOT_NORMAL;
}

void clearGameRecords() {
  Preferences records;
  records.begin("kumitate", false);
  records.clear();
  records.end();

  oled.clearDisplay();
  bootText.setCursor(0, 20); bootText.print("ゲーム記録を");
  bootText.setCursor(0, 42); bootText.print("消去しました");
  oled.display();
  tone(KUMITATE_PIN_BUZZER, 900, 100); delay(140);
  tone(KUMITATE_PIN_BUZZER, 1400, 180); delay(1600);
}

bool confirmGameRecordReset() {
  // 長押し成立直後に、まず確認画面へ遷移する。
  oled.clearDisplay();
  bootText.setCursor(0, 14); bootText.print("記録を消去？");
  bootText.setCursor(0, 36); bootText.print("SW1:はい");
  bootText.setCursor(0, 58); bootText.print("SW2:いいえ");
  oled.display();

  // 画面遷移後、起動メニューで押していたSW2とSW3が
  // 離されてから確認入力を受け付ける。これにより長押し中の
  // SW2が、そのまま「いいえ」と判定されることを防ぐ。
  while (digitalRead(KUMITATE_PIN_SW1) == LOW ||
         digitalRead(KUMITATE_PIN_SW2) == LOW ||
         digitalRead(KUMITATE_PIN_SW3) == LOW ||
         digitalRead(KUMITATE_PIN_SW4) == LOW) {
    delay(10);
  }
  delay(80);

  for (;;) {
    if (digitalRead(KUMITATE_PIN_SW1) == LOW) {
      delay(30);
      if (digitalRead(KUMITATE_PIN_SW1) == LOW) {
        while (digitalRead(KUMITATE_PIN_SW1) == LOW) delay(5);
        return true;
      }
    }
    if (digitalRead(KUMITATE_PIN_SW2) == LOW) {
      delay(30);
      if (digitalRead(KUMITATE_PIN_SW2) == LOW) {
        while (digitalRead(KUMITATE_PIN_SW2) == LOW) delay(5);
        return false;
      }
    }
    delay(5);
  }
}

void showResetCancelled() {
  oled.clearDisplay();
  bootText.setCursor(0, 26); bootText.print("消去を");
  bootText.setCursor(0, 48); bootText.print("取り消しました");
  oled.display();
  tone(KUMITATE_PIN_BUZZER, 500, 100);
  delay(1200);
}

void fatalOledError() {
  pinMode(KUMITATE_PIN_LED, OUTPUT);
  Serial.println("OLED 0x3C NOT FOUND");
  for (;;) {
    digitalWrite(KUMITATE_PIN_LED, HIGH); delay(150);
    digitalWrite(KUMITATE_PIN_LED, LOW);  delay(850);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(KUMITATE_PIN_SW1, INPUT_PULLUP);
  pinMode(KUMITATE_PIN_SW2, INPUT_PULLUP);
  pinMode(KUMITATE_PIN_SW3, INPUT_PULLUP);
  pinMode(KUMITATE_PIN_SW4, INPUT_PULLUP);

  Wire.begin(KUMITATE_PIN_SDA, KUMITATE_PIN_SCL);
  Wire.setClock(100000);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, KUMITATE_OLED_ADDRESS)) fatalOledError();

  rgb.begin();
  rgb.setBrightness(KUMITATE_RGB_BRIGHTNESS);
  rgb.clear();
  rgb.show();

  bootText.begin(oled);
  bootText.setFont(u8g2_font_unifont_t_japanese3);
  bootText.setForegroundColor(SSD1306_WHITE);
  oled.clearDisplay();
  bootText.setCursor(0, 14); bootText.print("起動モード");
  bootText.setCursor(0, 30); bootText.print("1+4長押し:検査");
  bootText.setCursor(0, 46); bootText.print("2+3長押し:消去");
  bootText.setCursor(0, 62); bootText.print("5秒後:通常起動");
  oled.display();

  BootAction bootAction = waitForBootAction();
  if (bootAction == BOOT_RESET_RECORDS) {
    if (confirmGameRecordReset()) clearGameRecords();
    else showResetCancelled();
    bootAction = BOOT_NORMAL;
  }

  if (bootAction == BOOT_FACTORY) {
    appMode = MODE_FACTORY;
    factoryTest.begin();
  } else {
    appMode = MODE_DEMO;
    demoProgram.begin();
  }
}

void loop() {
  if (appMode == MODE_FACTORY) factoryTest.update();
  else demoProgram.update();
}
