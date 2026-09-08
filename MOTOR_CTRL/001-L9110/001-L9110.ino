#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <esp_arduino_version.h>

// ============================================================
// KUMITATE-C3 pin assignments
// ============================================================

// Built-in OLED
constexpr uint8_t OLED_SDA = 1;
constexpr uint8_t OLED_SCL = 2;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Built-in switches（押すとLOW）
constexpr uint8_t PIN_SW1 = 21;  // ON / OFF
constexpr uint8_t PIN_SW2 = 3;   // Speed UP
constexpr uint8_t PIN_SW3 = 9;   // Speed DOWN
constexpr uint8_t PIN_SW4 = 8;   // Direction

// L9110
constexpr uint8_t PIN_L9110_IA = 4;
constexpr uint8_t PIN_L9110_IB = 5;

// ============================================================
// OLED
// ============================================================

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// ============================================================
// PWM settings
// ============================================================

constexpr uint32_t PWM_FREQUENCY = 20000;
constexpr uint8_t PWM_RESOLUTION = 8;

#if ESP_ARDUINO_VERSION_MAJOR < 3
constexpr uint8_t PWM_CHANNEL_IA = 0;
constexpr uint8_t PWM_CHANNEL_IB = 1;
#endif

bool pwmAttachedIA = false;
bool pwmAttachedIB = false;

// ============================================================
// Motor state
// ============================================================

constexpr uint8_t SPEED_STEP = 16;
constexpr unsigned long DIRECTION_WAIT_MS = 150;

bool motorEnabled = false;
bool motorForward = true;

// 設定速度
uint8_t motorSpeed = 128;

// 実際に出力中の速度
uint8_t appliedSpeed = 0;

// ============================================================
// Button state
// ============================================================

enum ButtonIndex {
  BUTTON_POWER = 0,
  BUTTON_SPEED_UP,
  BUTTON_SPEED_DOWN,
  BUTTON_DIRECTION,
  BUTTON_COUNT
};

const uint8_t buttonPins[BUTTON_COUNT] = {
  PIN_SW1,
  PIN_SW2,
  PIN_SW3,
  PIN_SW4
};

bool buttonStableState[BUTTON_COUNT] = {
  HIGH,
  HIGH,
  HIGH,
  HIGH
};

bool buttonPreviousReading[BUTTON_COUNT] = {
  HIGH,
  HIGH,
  HIGH,
  HIGH
};

unsigned long buttonChangedAt[BUTTON_COUNT] = {
  0,
  0,
  0,
  0
};

constexpr unsigned long DEBOUNCE_MS = 40;

// ============================================================
// PWM functions
// ============================================================

void initializePwm() {
#if ESP_ARDUINO_VERSION_MAJOR < 3

  ledcSetup(
    PWM_CHANNEL_IA,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  ledcSetup(
    PWM_CHANNEL_IB,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

#endif
}

void detachPwmIA() {
  if (!pwmAttachedIA) {
    return;
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcDetach(PIN_L9110_IA);
#else
  ledcDetachPin(PIN_L9110_IA);
#endif

  pwmAttachedIA = false;
}

void detachPwmIB() {
  if (!pwmAttachedIB) {
    return;
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcDetach(PIN_L9110_IB);
#else
  ledcDetachPin(PIN_L9110_IB);
#endif

  pwmAttachedIB = false;
}

void startPwmIA(uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3

  bool attached = ledcAttach(
    PIN_L9110_IA,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  if (attached) {
    pwmAttachedIA = true;
    ledcWrite(PIN_L9110_IA, duty);
  }

#else

  ledcAttachPin(PIN_L9110_IA, PWM_CHANNEL_IA);
  pwmAttachedIA = true;
  ledcWrite(PWM_CHANNEL_IA, duty);

#endif
}

void startPwmIB(uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3

  bool attached = ledcAttach(
    PIN_L9110_IB,
    PWM_FREQUENCY,
    PWM_RESOLUTION
  );

  if (attached) {
    pwmAttachedIB = true;
    ledcWrite(PIN_L9110_IB, duty);
  }

#else

  ledcAttachPin(PIN_L9110_IB, PWM_CHANNEL_IB);
  pwmAttachedIB = true;
  ledcWrite(PWM_CHANNEL_IB, duty);

#endif
}

// ============================================================
// Hard motor stop
// ============================================================

void stopMotorHard() {
  // PWMとの接続を解除
  detachPwmIA();
  detachPwmIB();

  // 通常のGPIO出力に戻す
  pinMode(PIN_L9110_IA, OUTPUT);
  pinMode(PIN_L9110_IB, OUTPUT);

  // L9110の両入力を確実にLOWへ固定
  digitalWrite(PIN_L9110_IA, LOW);
  digitalWrite(PIN_L9110_IB, LOW);

  appliedSpeed = 0;
}

// ============================================================
// Motor drive
// ============================================================

void startMotor() {
  // 無効または速度0なら必ず停止
  if (!motorEnabled || motorSpeed == 0) {
    stopMotorHard();
    return;
  }

  // 前のPWM状態を一度完全に解除
  stopMotorHard();

  if (motorForward) {
    // 正転
    // IA = PWM、IB = LOW
    digitalWrite(PIN_L9110_IB, LOW);
    startPwmIA(motorSpeed);
  } else {
    // 逆転
    // IA = LOW、IB = PWM
    digitalWrite(PIN_L9110_IA, LOW);
    startPwmIB(motorSpeed);
  }

  appliedSpeed = motorSpeed;
}

void turnMotorOn() {
  motorEnabled = true;
  startMotor();
}

void turnMotorOff() {
  motorEnabled = false;
  stopMotorHard();
}

void toggleMotorPower() {
  if (motorEnabled) {
    turnMotorOff();
  } else {
    turnMotorOn();
  }
}

void increaseSpeed() {
  int newSpeed = static_cast<int>(motorSpeed) + SPEED_STEP;

  if (newSpeed > 255) {
    newSpeed = 255;
  }

  motorSpeed = static_cast<uint8_t>(newSpeed);

  // ONのときだけ実出力へ反映
  if (motorEnabled) {
    startMotor();
  }
}

void decreaseSpeed() {
  int newSpeed = static_cast<int>(motorSpeed) - SPEED_STEP;

  if (newSpeed < 0) {
    newSpeed = 0;
  }

  motorSpeed = static_cast<uint8_t>(newSpeed);

  // OFFのときはモーター出力に一切触らない
  if (motorEnabled) {
    startMotor();
  }
}

void reverseDirection() {
  bool wasRunning = motorEnabled && motorSpeed > 0;

  // 回転中なら一度確実に停止
  if (wasRunning) {
    stopMotorHard();
    delay(DIRECTION_WAIT_MS);
  }

  motorForward = !motorForward;

  // ONだった場合のみ新しい方向で再始動
  if (motorEnabled) {
    startMotor();
  }
}

// ============================================================
// Button debounce
// ============================================================

bool buttonPressed(uint8_t buttonIndex) {
  bool reading = digitalRead(buttonPins[buttonIndex]);

  if (reading != buttonPreviousReading[buttonIndex]) {
    buttonPreviousReading[buttonIndex] = reading;
    buttonChangedAt[buttonIndex] = millis();
  }

  if ((millis() - buttonChangedAt[buttonIndex]) >= DEBOUNCE_MS) {
    if (reading != buttonStableState[buttonIndex]) {
      buttonStableState[buttonIndex] = reading;

      if (buttonStableState[buttonIndex] == LOW) {
        return true;
      }
    }
  }

  return false;
}

// ============================================================
// OLED
// ============================================================

void drawSpeedBar(uint8_t value) {
  constexpr int x = 0;
  constexpr int y = 54;
  constexpr int width = 128;
  constexpr int height = 10;

  oled.drawFrame(x, y, width, height);

  int barWidth = map(value, 0, 255, 0, width - 4);

  if (barWidth > 0) {
    oled.drawBox(
      x + 2,
      y + 2,
      barWidth,
      height - 4
    );
  }
}

void updateDisplay() {
  int settingPercent = map(motorSpeed, 0, 255, 0, 100);
  int outputPercent = map(appliedSpeed, 0, 255, 0, 100);

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tf);

  oled.drawStr(0, 10, "KUMITATE-C3 L9110");
  oled.drawHLine(0, 13, 128);

  oled.setCursor(0, 26);
  oled.print("POWER:");
  oled.print(motorEnabled ? "ON " : "OFF");

  oled.setCursor(68, 26);
  oled.print("DIR:");
  oled.print(motorForward ? "FWD" : "REV");

  oled.setCursor(0, 39);
  oled.print("SET:");
  oled.print(settingPercent);
  oled.print("% ");
  oled.print(motorSpeed);

  oled.setCursor(68, 39);
  oled.print("OUT:");
  oled.print(outputPercent);
  oled.print("%");

  if (!motorEnabled) {
    oled.drawStr(0, 51, "OUTPUT DISABLED");
  } else if (motorSpeed == 0) {
    oled.drawStr(0, 51, "SPEED ZERO");
  } else {
    oled.drawStr(0, 51, "RUNNING");
  }

  drawSpeedBar(appliedSpeed);

  oled.sendBuffer();
}

// ============================================================
// Serial status
// ============================================================

void printStatus() {
  Serial.print("POWER=");
  Serial.print(motorEnabled ? "ON" : "OFF");

  Serial.print(" DIR=");
  Serial.print(motorForward ? "FORWARD" : "REVERSE");

  Serial.print(" SET=");
  Serial.print(motorSpeed);

  Serial.print(" OUT=");
  Serial.println(appliedSpeed);
}

// ============================================================
// Setup
// ============================================================

void setup() {
  Serial.begin(115200);

  // KUMITATE-C3のスイッチには外部プルアップがあります
  pinMode(PIN_SW1, INPUT);
  pinMode(PIN_SW2, INPUT);
  pinMode(PIN_SW3, INPUT);
  pinMode(PIN_SW4, INPUT);

  pinMode(PIN_L9110_IA, OUTPUT);
  pinMode(PIN_L9110_IB, OUTPUT);

  initializePwm();

  // 起動時は必ず停止
  motorEnabled = false;
  stopMotorHard();

  // Built-in OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // U8g2は8bit形式でアドレスを指定
  oled.setI2CAddress(OLED_ADDRESS * 2);
  oled.begin();

  updateDisplay();
  printStatus();

  Serial.println("SW1: ON/OFF");
  Serial.println("SW2: SPEED UP");
  Serial.println("SW3: SPEED DOWN");
  Serial.println("SW4: DIRECTION");
}

// ============================================================
// Main loop
// ============================================================

void loop() {
  bool displayNeedsUpdate = false;

  // SW1：ON/OFF
  if (buttonPressed(BUTTON_POWER)) {
    toggleMotorPower();
    displayNeedsUpdate = true;
  }

  // SW2：速度を上げる
  if (buttonPressed(BUTTON_SPEED_UP)) {
    increaseSpeed();
    displayNeedsUpdate = true;
  }

  // SW3：速度を下げる
  if (buttonPressed(BUTTON_SPEED_DOWN)) {
    decreaseSpeed();
    displayNeedsUpdate = true;
  }

  // SW4：方向反転
  if (buttonPressed(BUTTON_DIRECTION)) {
    reverseDirection();
    displayNeedsUpdate = true;
  }

  if (displayNeedsUpdate) {
    updateDisplay();
    printStatus();
  }

  delay(1);
}