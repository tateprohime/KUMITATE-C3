// 起動時に１度だけ呼ばれる
void setup() {
  // ESP32C3の0番ピンを出力設定にする
  pinMode(0, OUTPUT);
}

// setup関数の実行後に開始、電源を切るまで回り続ける
void loop() {
  digitalWrite(0, HIGH);  // 0番ピンを出力する
  delay(1000);            // 1000ミリ秒待つ
  digitalWrite(0, LOW);   // 0番ピンの出力を停止する
  delay(1000);            // 1000ミリ秒待つ
}
