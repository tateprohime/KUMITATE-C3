
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// KUMITATE-C3 GPIO
#define BUZZER_PIN 20

#define SW1_PIN 21
#define SW2_PIN 3
#define SW3_PIN 9
#define SW4_PIN 8

#define SDA_PIN 1
#define SCL_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1
);

// 周波数設定
const int MIN_FREQ = 100;
const int MAX_FREQ = 20000;
int frequency = 4000;

// ブザーON/OFF
bool buzzerEnabled = true;

// スイッチ設定
const uint8_t swPins[4] = {
  SW1_PIN, SW2_PIN, SW3_PIN, SW4_PIN
};

const int steps[4] = {
  -1000, -100, 1000, 100
};

const unsigned long DEBOUNCE_MS = 30;
const unsigned long COMBO_WINDOW_MS = 150;

bool lastReading[4];
bool stableState[4];
unsigned long lastChange[4];

// SW1・SW4の単押し判定を保留する
bool pending[4] = {false, false, false, false};
unsigned long pressTime[4] = {0, 0, 0, 0};

// 同時押しを1回だけ実行するためのフラグ
bool comboHandled = false;

// OLED更新
void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BUZZER: ");
  display.println(buzzerEnabled ? "ON" : "OFF");

  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setTextSize(2);
  String freqText = String(frequency) + " Hz";
  int x = (128 - freqText.length() * 12) / 2;
  display.setCursor(max(0, x), 23);
  display.println(freqText);

  display.setTextSize(1);
  display.setCursor(0, 46);
  display.println("1:-1k 2:-100");
  display.setCursor(0, 56);
  display.println("3:+1k 4:+100");

  display.display();
}

// ブザー出力反映
void applyBuzzer() {
  if (buzzerEnabled) {
    ledcWriteTone(BUZZER_PIN, frequency);
  } else {
    ledcWriteTone(BUZZER_PIN, 0);
  }
}

// 周波数変更
void updateFrequency(int step) {
  int newFreq = constrain(
    frequency + step,
    MIN_FREQ,
    MAX_FREQ
  );

  if (newFreq == frequency) return;

  frequency = newFreq;
  applyBuzzer();
  updateDisplay();

  Serial.printf("Frequency: %d Hz\n", frequency);
}

// ON/OFF切り替え
void toggleBuzzer() {
  buzzerEnabled = !buzzerEnabled;
  applyBuzzer();
  updateDisplay();

  Serial.printf(
    "Buzzer: %s\n",
    buzzerEnabled ? "ON" : "OFF"
  );
}

// スイッチ処理
void checkSwitches() {
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(swPins[i]);

    if (reading != lastReading[i]) {
      lastChange[i] = now;
    }

    if (now - lastChange[i] >= DEBOUNCE_MS) {
      if (reading != stableState[i]) {
        stableState[i] = reading;

        if (reading == LOW) {
          // SW1, SW4は同時押し判定のため保留
          if (i == 0 || i == 3) {
            pending[i] = true;
            pressTime[i] = now;
          } else {
            updateFrequency(steps[i]);
          }
        }
      }
    }

    lastReading[i] = reading;
  }

  // SW1 + SW4 同時押し
  if (!comboHandled &&
      stableState[0] == LOW &&
      stableState[3] == LOW &&
      (now - pressTime[0] <= COMBO_WINDOW_MS ||
       now - pressTime[3] <= COMBO_WINDOW_MS) &&
      abs((long)(pressTime[0] - pressTime[3])) <=
          (long)COMBO_WINDOW_MS) {

    pending[0] = false;
    pending[3] = false;

    toggleBuzzer();
    comboHandled = true;
  }

  // 同時押し後、両方離すまで再実行しない
  if (stableState[0] == HIGH &&
      stableState[3] == HIGH) {
    comboHandled = false;
  }

  // 単押し確定
  for (int i : {0, 3}) {
    if (pending[i] &&
        now - pressTime[i] >= COMBO_WINDOW_MS) {

      pending[i] = false;

      if (!comboHandled) {
        updateFrequency(steps[i]);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(swPins[i], INPUT_PULLUP);
    lastReading[i] = digitalRead(swPins[i]);
    stableState[i] = lastReading[i];
    lastChange[i] = 0;
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    while (true) delay(100);
  }

  ledcAttach(BUZZER_PIN, frequency, 10);
  applyBuzzer();
  updateDisplay();
}

void loop() {
  checkSwitches();
  delay(1);
}
