int blinkInterval = 1000;
bool ledState = false;
unsigned long lastToggleTime = 0;

bool lastSw1State = HIGH;  // SW1の前回の状態
bool lastSw2State = HIGH;  // SW2の前回の状態

void setup() {
  pinMode(0, OUTPUT);
  pinMode(21, INPUT);  // SW1
  pinMode(3, INPUT);   // SW2
}

void loop() {
  bool sw1State = digitalRead(21);
  bool sw2State = digitalRead(3);

  // SW1が「離れていた→押された」瞬間だけ反応
  if (lastSw1State == HIGH && sw1State == LOW) {
    blinkInterval -= 100;
    if (blinkInterval < 100) blinkInterval = 100;
  }

  // SW2が「離れていた→押された」瞬間だけ反応
  if (lastSw2State == HIGH && sw2State == LOW) {
    blinkInterval += 100;
    if (blinkInterval > 2000) blinkInterval = 2000;
  }

  lastSw1State = sw1State;  // 今回の状態を「前回」として保存
  lastSw2State = sw2State;

  unsigned long now = millis();
  if (now - lastToggleTime >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(0, ledState);
    lastToggleTime = now;
  }
}