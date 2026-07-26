#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int pressCount = 0;
const int maxCount = 10;
bool lastSw1State = HIGH;
bool lastSw2State = HIGH;

int targetWidth = 0;    // 目標のバーの幅
int currentWidth = 0;   // 今表示しているバーの幅
unsigned long lastStepTime = 0;

void setup() {
  pinMode(21, INPUT);  // SW1: 増やす
  pinMode(3, INPUT);   // SW2: リセット

  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
}

void loop() {
  bool sw1State = digitalRead(21);
  bool sw2State = digitalRead(3);

  if (lastSw1State == HIGH && sw1State == LOW) {
    if (pressCount < maxCount) pressCount++;
    targetWidth = map(pressCount, 0, maxCount, 0, 120);  // カウントをバーの幅に変換
  }

  if (lastSw2State == HIGH && sw2State == LOW) {
    pressCount = 0;
    targetWidth = 0;
  }

  lastSw1State = sw1State;
  lastSw2State = sw2State;

  // currentWidthを少しずつtargetWidthに近づける
  if (millis() - lastStepTime >= 15) {
    if (currentWidth < targetWidth) currentWidth++;
    else if (currentWidth > targetWidth) currentWidth--;
    lastStepTime = millis();

    drawGauge();
  }
}

void drawGauge() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Gauge: ");
  display.print(pressCount);
  display.print("/");
  display.print(maxCount);

  display.drawRect(0, 20, 122, 20, SSD1306_WHITE);          // 枠
  display.fillRect(1, 21, currentWidth, 18, SSD1306_WHITE); // 中身

  display.display();
}