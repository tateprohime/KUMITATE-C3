int blinkInterval = 1000;  // 点滅の間隔（ミリ秒）。最初は1秒

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
}

void loop() {
  // SW1が押されていたら間隔を短く（速く点滅）
  if (digitalRead(21) == LOW) {
    blinkInterval -= 100;
    if (blinkInterval < 100) blinkInterval = 100;  // 短くなりすぎないようにする
  }

  // SW2が押されていたら間隔を長く（遅く点滅）
  if (digitalRead(3) == LOW) {
    blinkInterval += 100;
    if (blinkInterval > 2000) blinkInterval = 2000;  // 長くなりすぎないようにする
  }

  digitalWrite(0, HIGH);
  delay(blinkInterval);
  digitalWrite(0, LOW);
  delay(blinkInterval);
}