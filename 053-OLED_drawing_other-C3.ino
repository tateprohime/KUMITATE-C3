#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

#define LED_PIN 10
#define LED_COUNT 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int pressCount = 0;
const int maxCount = 10;
bool lastSw1State = HIGH;
bool lastSw2State = HIGH;

int targetWidth = 0;
int currentWidth = 0;
unsigned long lastStepTime = 0;

bool alarmActive = false;   // 今、満タン状態かどうか
bool alarmHandled = false;  // 満タンになった瞬間の通知を、もう出したかどうか

void setup() {
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
  pinMode(20, OUTPUT); // ブザー

  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  strip.begin();
}

void loop() {
  bool sw1State = digitalRead(21);
  bool sw2State = digitalRead(3);

  if (lastSw1State == HIGH && sw1State == LOW) {
    if (pressCount < maxCount) pressCount++;
    targetWidth = map(pressCount, 0, maxCount, 0, 120);
  }

  if (lastSw2State == HIGH && sw2State == LOW) {
    pressCount = 0;
    targetWidth = 0;
    alarmActive = false;
    alarmHandled = false;
    setAllLED(strip.Color(0, 0, 0));  // LEDを消灯
  }

  lastSw1State = sw1State;
  lastSw2State = sw2State;

  // 満タンかどうかを毎回判定する
  if (pressCount >= maxCount) {
    alarmActive = true;
  } else {
    alarmActive = false;
    alarmHandled = false;
  }

  // 満タンに「なった瞬間」だけ、ブザーとLEDで通知する
  if (alarmActive && !alarmHandled) {
    tone(20, 1000, 300);
    setAllLED(strip.Color(255, 0, 0));
    alarmHandled = true;
  }

  if (millis() - lastStepTime >= 15) {
    if (currentWidth < targetWidth) currentWidth++;
    else if (currentWidth > targetWidth) currentWidth--;
    lastStepTime = millis();

    drawGauge();
  }
}

void setAllLED(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
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

  display.drawRect(0, 20, 122, 20, SSD1306_WHITE);
  display.fillRect(1, 21, currentWidth, 18, SSD1306_WHITE);

  if (alarmActive) {
    display.setTextSize(2);
    display.setCursor(30, 45);
    display.print("MAX!");
  }

  display.display();
}