int blinkInterval = 1000;
bool ledState = false;
unsigned long lastToggleTime = 0;

bool lastSw1State = HIGH;
bool lastSw2State = HIGH;

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
  pinMode(20, OUTPUT); // ブザー
}

void loop() {
  bool sw1State = digitalRead(21);
  bool sw2State = digitalRead(3);

  // SW1が押された瞬間
  if (lastSw1State == HIGH && sw1State == LOW) {
    if (blinkInterval <= 100) {
      // すでに下限にいるのに、さらに下げようとした
      tone(20, 1000, 100);  // 1000Hzで100ミリ秒鳴らす
    } else {
      blinkInterval -= 100;
    }
  }

  // SW2が押された瞬間
  if (lastSw2State == HIGH && sw2State == LOW) {
    if (blinkInterval >= 2000) {
      // すでに上限にいるのに、さらに上げようとした
      tone(20, 1000, 100);
    } else {
      blinkInterval += 100;
    }
  }

  lastSw1State = sw1State;
  lastSw2State = sw2State;

  unsigned long now = millis();
  if (now - lastToggleTime >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(0, ledState);
    lastToggleTime = now;
  }
}