#include <Adafruit_NeoPixel.h>

#define LED_PIN 10
#define LED_COUNT 4

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();

  strip.setPixelColor(0, strip.Color(255, 0, 0));    // 1個目: 赤
  strip.setPixelColor(1, strip.Color(0, 255, 0));    // 2個目: 緑
  strip.setPixelColor(2, strip.Color(0, 0, 255));    // 3個目: 青
  strip.setPixelColor(3, strip.Color(255, 255, 0));  // 4個目: 黄色

  strip.show();  // 4つ分の予約を、まとめて一気に送信
}

void loop() {
}