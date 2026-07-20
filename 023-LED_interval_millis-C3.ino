int blinkInterval = 1000;      // 点滅の間隔（ミリ秒）
bool ledState = false;         // LEDが今光っているかどうか
unsigned long lastToggleTime = 0;  // 最後にLEDを切り替えた時刻

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
}

void loop() {
  // SW1が押されていたら間隔を短く（速く点滅）
  if (digitalRead(21) == LOW) {
    blinkInterval -= 100;
    if (blinkInterval < 100) blinkInterval = 100;
  }

  // SW2が押されていたら間隔を長く（遅く点滅）
  if (digitalRead(3) == LOW) {
    blinkInterval += 100;
    if (blinkInterval > 2000) blinkInterval = 2000;
  }

  // 現在時刻をチェックして、切り替えタイミングか判断する
  unsigned long now = millis();
  if (now - lastToggleTime >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(0, ledState);
    lastToggleTime = now;  // 切り替えた時刻を記録
  }
}