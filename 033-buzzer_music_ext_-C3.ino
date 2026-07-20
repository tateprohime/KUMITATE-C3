const int switchPins[] = {21, 3, 9, 8, 4, 5, 6, 7};  // SW1〜SW8のピン番号
const int noteFreqs[]  = {262, 294, 330, 349, 392, 440, 494, 523};  // ド レ ミ ファ ソ ラ シ ド
const int numButtons = 8;

int lastPlayingIndex = -1;  // 今鳴らしている音の番号(-1は「何も鳴っていない」)

void setup() {
  pinMode(20, OUTPUT);  // ブザー

  for (int i = 0; i < numButtons; i++) {
    pinMode(switchPins[i], INPUT);
  }
}

void loop() {
  int pressedIndex = -1;

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(switchPins[i]) == LOW) {
      pressedIndex = i;
      break;
    }
  }

  if (pressedIndex != lastPlayingIndex) {
    if (pressedIndex == -1) {
      noTone(20);
    } else {
      tone(20, noteFreqs[pressedIndex]);
    }
    lastPlayingIndex = pressedIndex;
  }
}