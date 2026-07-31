#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int switchPins[] = {21, 3, 9, 8, 4, 5, 6, 7};  // SW1〜SW8(それぞれ1ビットに対応)
const int numSwitches = 8;

int targetNumber = 0;
bool cleared = false;

void setup() {
  pinMode(20, OUTPUT);  // ブザー
  for (int i = 0; i < numSwitches; i++) {
    pinMode(switchPins[i], INPUT);
  }

  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  randomSeed(analogRead(0));
  newTarget();
}

void loop() {
  int currentValue = readSwitchesAsByte();

  drawScreen(currentValue);

  if (currentValue == targetNumber && !cleared) {
    cleared = true;
    tone(20, 1000, 300);
    delay(1000);
    newTarget();
  }

  if (currentValue != targetNumber) {
    cleared = false;
  }
}

// 8個のスイッチの状態を、1つの数値(0〜255)にまとめる
int readSwitchesAsByte() {
  int value = 0;

  for (int i = 0; i < numSwitches; i++) {
    if (digitalRead(switchPins[i]) == LOW) {
      value = value | (1 << i);  // i番目のビットを1にする
    }
  }

  return value;
}

void newTarget() {
  targetNumber = random(0, 256);  // 0〜255のランダムな目標
}

void drawScreen(int currentValue) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Target: ");
  display.print(targetNumber);

  display.setCursor(0, 15);
  display.print("Now:    ");
  display.print(currentValue);

  // 2進数(0と1の並び)でも表示する
  display.setCursor(0, 35);
  for (int i = numSwitches - 1; i >= 0; i--) {
    display.print(bitRead(currentValue, i));
  }

  display.display();
}