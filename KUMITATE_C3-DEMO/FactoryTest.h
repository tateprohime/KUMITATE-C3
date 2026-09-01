#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2_for_Adafruit_GFX.h>

class FactoryTest {
 public:
  FactoryTest(Adafruit_SSD1306& display, Adafruit_NeoPixel& pixels);
  void begin();
  void update();

 private:
  enum Choice { CHOICE_OK, CHOICE_NG, CHOICE_SKIP };
  Adafruit_SSD1306& display;
  Adafruit_NeoPixel& pixels;
  U8G2_FOR_ADAFRUIT_GFX text;
  uint16_t passCount=0, failCount=0, skipCount=0;
  bool finished=false;

  void screen(const char* a,const char* b="",const char* c="",const char* d="");
  void waitRelease();
  bool waitButton(uint8_t pin,uint32_t timeout=0);
  Choice ask(const char* item,const char* detail="");
  void record(const char* item,Choice value);
  void runAll();
  void testOled();
  void testLed();
  void testBuzzer();
  void testRgb();
  void testSwitches();
  void testLoopback();
  bool pairTest(uint8_t a,uint8_t b);
  bool driveRead(uint8_t out,uint8_t in,uint8_t level);
  void summary();
};
