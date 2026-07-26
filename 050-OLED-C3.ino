#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(1, 2);  // SDA=IO1, SCL=IO2

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    // 初期化失敗時はここで止める(後のレッスンで対処法を扱います)
    while (true);
  }

  display.clearDisplay();       // 画面をまっさらにする
  display.setTextSize(1);       // 文字の大きさ
  display.setTextColor(SSD1306_WHITE);  // 文字の色(白)
  display.setCursor(0, 0);      // 文字を書き始める位置(左上)
  display.print("Hello, KUMITATE-C3!");
  display.display();            // 実際の画面に反映
}

void loop() {
}