#include <Adafruit_NeoPixel.h>

#define LED_PIN 10
#define LED_COUNT 4

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 切り替える色の一覧
uint32_t colors[] = {
  0,  // 一時的な値。setup()の中で本物の色に置き換える
};
const int numColors = 4;
int colorIndex = 0;  // 今何番目の色を表示しているか

bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  strip.begin();

  // strip.Color()はsetup()より前(グローバル)では使えないため、ここで配列を作る
  static uint32_t colorList[] = {
    strip.Color(255, 0, 0),   // 赤
    strip.Color(0, 255, 0),   // 緑
    strip.Color(0, 0, 255),   // 青
    strip.Color(255, 255, 0), // 黄色
  };

  showColor(colorList[colorIndex]);
}

void loop() {
  bool sw1State = digitalRead(21);

  if (lastSw1State == HIGH && sw1State == LOW) {
    colorIndex = (colorIndex + 1) % numColors;  // 次の色へ(最後まで行ったら0に戻る)

    static uint32_t colorList[] = {
      strip.Color(255, 0, 0),
      strip.Color(0, 255, 0),
      strip.Color(0, 0, 255),
      strip.Color(255, 255, 0),
    };
    showColor(colorList[colorIndex]);
  }

  lastSw1State = sw1State;
}

// 4個すべてを指定した色にする関数
void showColor(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}