bool lastRawState = HIGH;
bool debouncedState = HIGH;
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 50;

unsigned long pressStartTime = 0;
bool longPressHandled = false;
const unsigned long longPressThreshold = 1000;  // 長押しとみなす時間(ミリ秒)

bool ledState = false;

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);
}

void loop() {
  bool rawState = digitalRead(21);

  if (rawState != lastRawState) {
    lastChangeTime = millis();
  }

  if (millis() - lastChangeTime > debounceDelay) {
    if (rawState != debouncedState) {
      debouncedState = rawState;

      if (debouncedState == LOW) {
        // 押された瞬間 → タイマーをスタート
        pressStartTime = millis();
        longPressHandled = false;
      } else {
        // 離された瞬間
        if (!longPressHandled) {
          // 長押し判定がまだなら、短押しとして扱う
          ledState = !ledState;
          digitalWrite(0, ledState);
        }
      }
    }
  }

  // 押し続けている間、毎回「もう1秒経った？」を確認する
  if (debouncedState == LOW && !longPressHandled) {
    if (millis() - pressStartTime >= longPressThreshold) {
      longPressHandled = true;

      // 長押し動作: 3回速く点滅させてリセット
      for (int i = 0; i < 3; i++) {
        digitalWrite(0, HIGH);
        delay(100);
        digitalWrite(0, LOW);
        delay(100);
      }
      ledState = false;
    }
  }

  lastRawState = rawState;
}