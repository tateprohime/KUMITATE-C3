const int switchPins[] = {21, 3, 9, 8};       // SW1, SW2, SW3, SW4のピン番号
const int noteFreqs[]  = {262, 294, 330, 349}; // ド、レ、ミ、ファの周波数(Hz)
const int numButtons = 4;

int lastPlayingIndex = -1;  // 今鳴らしている音の番号(-1は「何も鳴っていない」)

void setup() {
  pinMode(20, OUTPUT);  // ブザー

  for (int i = 0; i < numButtons; i++) {
    pinMode(switchPins[i], INPUT);
  }
}

void loop() {
  int pressedIndex = -1;  // 今押されているボタンの番号(-1は「何も押されていない」)

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(switchPins[i]) == LOW) {
      pressedIndex = i;
      break;
    }
  }

  // 状態が変化したときだけ、tone()/noTone()を呼ぶ
  if (pressedIndex != lastPlayingIndex) {
    if (pressedIndex == -1) {
      noTone(20);
    } else {
      tone(20, noteFreqs[pressedIndex]);
    }
    lastPlayingIndex = pressedIndex;
  }
}