#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int currentScreen = 0;       // 0:カウンター 1:ゲージ 2:説明
const int numScreens = 3;

int counter = 0;
const int maxCount = 10;

// SW1(画面送り・長押しでリセット)用
bool lastRawSw1 = HIGH;
bool debouncedSw1 = HIGH;
unsigned long lastChangeSw1 = 0;
const unsigned long debounceDelay = 50;
unsigned long pressStartSw1 = 0;
bool longPressHandled = false;
const unsigned long longPressThreshold = 1000;

// SW2(決定)用
bool lastSw2State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2

  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  drawScreen();
}

void loop() {
  handleSw1();
  handleSw2();
}

// SW1: デバウンス + 長押し判定
void handleSw1() {
  bool rawState = digitalRead(21);

  if (rawState != lastRawSw1) {
    lastChangeSw1 = millis();
  }

  if (millis() - lastChangeSw1 > debounceDelay) {
    if (rawState != debouncedSw1) {
      debouncedSw1 = rawState;

      if (debouncedSw1 == LOW) {
        pressStartSw1 = millis();
        longPressHandled = false;
      } else {
        if (!longPressHandled) {
          // 短押し → 次の画面へ
          currentScreen = (currentScreen + 1) % numScreens;
          drawScreen();
        }
      }
    }
  }

  if (debouncedSw1 == LOW && !longPressHandled) {
    if (millis() - pressStartSw1 >= longPressThreshold) {
      longPressHandled = true;
      counter = 0;  // 長押し → リセット
      drawScreen();
    }
  }

  lastRawSw1 = rawState;
}

// SW2: 決定操作(今はカウンターを増やすだけ)
void handleSw2() {
  bool sw2State = digitalRead(3);

  if (lastSw2State == HIGH && sw2State == LOW) {
    if (currentScreen == 0 && counter < maxCount) {
      counter++;
      drawScreen();
    }
  }

  lastSw2State = sw2State;
}

// 今の画面(currentScreen)に応じて描き分ける
// 今の画面(currentScreen)に応じて描き分ける(switch文版)
void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (currentScreen) {
    case 0:
      display.setCursor(0, 0);
      display.print("1: Counter");
      display.setTextSize(3);
      display.setCursor(0, 25);
      display.print(counter);
      break;

    case 1:
      display.setCursor(0, 0);
      display.print("2: Gauge");
      {
        int barWidth = map(counter, 0, maxCount, 0, 120);
        display.drawRect(0, 25, 122, 20, SSD1306_WHITE);
        display.fillRect(1, 26, barWidth, 18, SSD1306_WHITE);
      }
      break;

    case 2:
      display.setCursor(0, 0);
      display.print("3: Info");
      display.setCursor(0, 25);
      display.print("Short: next menu");
      display.setCursor(0, 40);
      display.print("Long: reset");
      break;

    default:
      display.setCursor(0, 0);
      display.print("Unknown screen");
      break;
  }

  display.display();
}