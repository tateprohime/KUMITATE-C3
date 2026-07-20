// 起動時に１度だけ呼ばれる
void setup() {
  // LED（0番ピン）を出力に設定
  pinMode(0, OUTPUT);

  // ボタン（21番ピン）を入力に設定
  pinMode(21, INPUT);
}

// setup関数の実行後に開始、電源を切るまで回り続ける
void loop() {
  // ボタンが押されているか確認（押すとLOW）
  if (digitalRead(21) == LOW) {
    // LEDを点灯
    digitalWrite(0, HIGH);
  } else {
    // LEDを消灯
    digitalWrite(0, LOW);
  }
}