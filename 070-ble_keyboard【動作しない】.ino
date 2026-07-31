#include <BleKeyboard.h>

BleKeyboard bleKeyboard("KUMITATE-C3", "Tatepro", 100);

bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  Serial.begin(115200);

  bleKeyboard.begin();
  Serial.println("BLEキーボードを開始しました。ペアリングしてください");
}

void loop() {
  bool sw1State = digitalRead(21);

  if (lastSw1State == HIGH && sw1State == LOW) {
    if (bleKeyboard.isConnected()) {
      bleKeyboard.print("Hello from KUMITATE-C3!");
    } else {
      Serial.println("まだ接続されていません");
    }
  }

  lastSw1State = sw1State;
}