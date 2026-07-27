struct Button {
  int pin;       // ピン番号
  int noteFreq;  // 鳴らす音の周波数
};

Button buttons[] = {
  {21, 262},  // SW1: ド
  {3,  294},  // SW2: レ
  {9,  330},  // SW3: ミ
  {8,  349},  // SW4: ファ
};
const int numButtons = 4;

int lastPlayingIndex = -1;

void setup() {
  pinMode(20, OUTPUT);
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT);
  }
}

void loop() {
  int pressedIndex = -1;

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttons[i].pin) == LOW) {
      pressedIndex = i;
      break;
    }
  }

  if (pressedIndex != lastPlayingIndex) {
    if (pressedIndex == -1) {
      noTone(20);
    } else {
      tone(20, buttons[pressedIndex].noteFreq);
    }
    lastPlayingIndex = pressedIndex;
  }
}