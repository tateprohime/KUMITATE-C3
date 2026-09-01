#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Preferences.h>

class DemoProgram {
 public:
  DemoProgram(Adafruit_SSD1306& display, Adafruit_NeoPixel& pixels);
  void begin();
  void update();

 private:
  Adafruit_SSD1306& display;
  Adafruit_NeoPixel& pixels;
  U8G2_FOR_ADAFRUIT_GFX text;
  Preferences preferences;
  uint32_t lastAnimation = 0;
  uint8_t animationStep = 0;
  uint16_t reactionBestScore = 0;
  int16_t lastReactionScore = 0;
  uint8_t memoryBestLevel = 0;
  uint32_t timingBestError = UINT32_MAX;

  void showMenu();
  void screen(const char* a, const char* b="", const char* c="", const char* d="");
  void waitRelease();
  int8_t readButton(uint32_t timeout=0);
  void lightOne(uint8_t index, uint32_t color);
  void reactionGame();
  void celebrateNewRecord();
  void memoryGame();
  void finishMemoryGame(uint8_t clearedLevel, bool perfect);
  void codeBreaker();
  void timingGame();
  void sequencer();
  void lightShow();
};
