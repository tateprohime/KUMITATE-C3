bool ledState = false;      // LEDが今光っているかどうかを覚えておく
bool lastButtonState = HIGH; // 前回読み取ったボタンの状態を覚えておく

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);
}

void loop() {
  bool buttonState = digitalRead(21);  // 今のボタンの状態を読む

  // 「離れていた→押された」瞬間だけ反応する
  if (lastButtonState == HIGH && buttonState == LOW) {
    ledState = !ledState;           // 状態を反転させる
    digitalWrite(0, ledState);      // LEDに反映する
    delay(50);                      // チャタリング対策の小休止
  }

  lastButtonState = buttonState;    // 今回の状態を「前回」として保存
}