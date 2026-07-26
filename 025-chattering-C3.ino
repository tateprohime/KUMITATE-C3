bool lastRawState = HIGH;       // 生の(フィルタ前の)前回状態
bool debouncedState = HIGH;     // フィルタ後の、信頼できる状態
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 50;  // 安定を待つ時間(ミリ秒)

bool ledState = false;

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);
}

void loop() {
  bool rawState = digitalRead(21);

  // 生の状態が変わった瞬間を記録する
  if (rawState != lastRawState) {
    lastChangeTime = millis();
  }

  // 変化してから一定時間、状態が安定していたら「本物の変化」とみなす
  if (millis() - lastChangeTime > debounceDelay) {
    if (rawState != debouncedState) {
      debouncedState = rawState;

      if (debouncedState == LOW) {
        ledState = !ledState;
        digitalWrite(0, ledState);
      }
    }
  }

  lastRawState = rawState;
}