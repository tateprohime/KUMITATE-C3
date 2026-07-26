#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int pressCount = 0;         // ボタンを押した回数
bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1

  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  updateDisplay();  // 最初に0回の状態を表示しておく
}

void loop() {
  bool sw1State = digitalRead(21);

  if (lastSw1State == HIGH && sw1State == LOW) {
    pressCount++;         // 1回押されるたびにカウントを増やす
    updateDisplay();      // 画面を更新
  }

  lastSw1State = sw1State;
}

// 現在のpressCountを画面に表示する関数
void updateDisplay() {
  display.clearDisplay();      // 前の表示を消す
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Button Count:");

  display.setTextSize(3);      // 数字だけ大きく表示
  display.setCursor(0, 20);
  display.print(pressCount);

  display.display();           // まとめて画面に反映
}