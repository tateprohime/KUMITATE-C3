#include <Adafruit_NeoPixel.h>

#define LED_PIN 10      // RGB LEDのデータピン
#define LED_COUNT 4     // RGB LEDの個数

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();       // RGB LEDの制御を開始
  strip.show();        // 最初は全部消灯した状態を送る

  strip.setPixelColor(0, strip.Color(255, 0, 0));  // 1個目を赤色に設定
  strip.show();        // 設定を実際のLEDに反映
}

void loop() {
  // 今回は光らせたら終わりなので、loop()では何もしない
}