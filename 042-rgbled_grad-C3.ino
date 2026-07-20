#include <Adafruit_NeoPixel.h>

#define LED_PIN 10
#define LED_COUNT 4

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int brightness = 0;   // 明るさ(0〜255)
int step = 5;         // 1回あたりの変化量

void setup() {
  strip.begin();
}

void loop() {
  brightness += step;  // 明るさを変化させる

  // 端まで来たら、変化の向きを反転させる
  if (brightness >= 255 || brightness <= 0) {
    step = -step;
  }

  // 4個すべてを、現在の明るさの赤色にする
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(brightness, 0, 0));
  }
  strip.show();

  delay(20);  // 変化の速さを調整
}