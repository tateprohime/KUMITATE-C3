int state = 0;  // 0:待機中(合図待ち) 1:合図済み(反応待ち) 2:結果表示中
unsigned long stateStartTime = 0;
unsigned long waitTime = 0;
unsigned long reactionStartTime = 0;

bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  pinMode(20, OUTPUT); // ブザー

  Serial.begin(115200);
  randomSeed(analogRead(0));  // 乱数の種をばらけさせる

  waitTime = random(2000, 5000);  // 2〜5秒のランダムな待ち時間
  stateStartTime = millis();

  Serial.println("準備...ボタンは押さずに待っていてください");
}

void loop() {
  bool sw1State = digitalRead(21);
  bool pressed = (lastSw1State == HIGH && sw1State == LOW);
  unsigned long now = millis();

  if (state == 0) {
    // 合図を待っているフェーズ
    if (pressed) {
      Serial.println("フライング！音が鳴る前に押してしまいました");
      tone(20, 200, 300);
      state = 2;
      stateStartTime = now;
    } else if (now - stateStartTime >= waitTime) {
      tone(20, 1000, 100);       // 合図の音
      reactionStartTime = now;
      state = 1;
    }

  } else if (state == 1) {
    // 反応を待っているフェーズ
    if (pressed) {
      unsigned long reactionTime = now - reactionStartTime;
      Serial.print("反応時間: ");
      Serial.print(reactionTime);
      Serial.println("ミリ秒");
      state = 2;
      stateStartTime = now;
    }

  } else if (state == 2) {
    // 結果を表示してから、少し待って次のラウンドへ
    if (now - stateStartTime >= 3000) {
      waitTime = random(2000, 5000);
      stateStartTime = now;
      state = 0;
      Serial.println("準備...ボタンは押さずに待っていてください");
    }
  }

  lastSw1State = sw1State;
}