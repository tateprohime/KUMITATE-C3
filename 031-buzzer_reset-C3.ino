int blinkInterval = 1000;
bool ledState = false;
unsigned long lastToggleTime = 0;

bool lastSw1State = HIGH;
bool lastSw2State = HIGH;
bool lastComboState = false;  // 同時押しの前回の状態

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
  pinMode(20, OUTPUT); // ブザー
}

void loop() {
  bool sw1State = digitalRead(21);
  bool sw2State = digitalRead(3);
  bool comboState = (sw1State == LOW && sw2State == LOW);  // 両方押されているか

  if (comboState && !lastComboState) {
    // 同時押しされた瞬間 → リセット
    blinkInterval = 1000;
    tone(20, 2000, 200);  // リセット音（高めで少し長く）

  } else if (!comboState) {
    // 片方だけの操作は、同時押し中でないときだけ受け付ける
    if (lastSw1State == HIGH && sw1State == LOW) {
      if (blinkInterval <= 100) {
        tone(20, 1000, 100);
      } else {
        blinkInterval -= 100;
      }
    }

    if (lastSw2State == HIGH && sw2State == LOW) {
      if (blinkInterval >= 2000) {
        tone(20, 1000, 100);
      } else {
        blinkInterval += 100;
      }
    }
  }

  lastSw1State = sw1State;
  lastSw2State = sw2State;
  lastComboState = comboState;

  unsigned long now = millis();
  if (now - lastToggleTime >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(0, ledState);
    lastToggleTime = now;
  }
}