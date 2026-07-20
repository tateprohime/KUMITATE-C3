const int switchPins[] = {21, 3, 9, 8};        // SW1〜SW4
const int noteFreqs[]  = {262, 294, 330, 349}; // ド レ ミ ファ
const int numButtons = 4;

int pattern[50];        // 手本の配列
int patternLength = 0;  // 手本の長さ
int playerIndex = 0;    // プレイヤーが何手目まで正解したか

bool lastSwState[4] = {HIGH, HIGH, HIGH, HIGH};  // 4つのボタンそれぞれの前回状態

void setup() {
  pinMode(20, OUTPUT);  // ブザー
  for (int i = 0; i < numButtons; i++) {
    pinMode(switchPins[i], INPUT);
  }

  randomSeed(analogRead(0));
  addNewStep();
  playPattern();
}

void loop() {
  for (int i = 0; i < numButtons; i++) {
    bool state = digitalRead(switchPins[i]);
    bool pressed = (lastSwState[i] == HIGH && state == LOW);
    lastSwState[i] = state;

    if (pressed) {
      handlePress(i);
    }
  }
}

// 手本に1音追加する
void addNewStep() {
  pattern[patternLength] = random(0, numButtons);
  patternLength++;
}

// 手本を最初から順番に再生する
void playPattern() {
  delay(500);
  for (int i = 0; i < patternLength; i++) {
    tone(20, noteFreqs[pattern[i]], 300);
    delay(400);
  }
  playerIndex = 0;
}

// ボタンが押されたときの判定
void handlePress(int index) {
  tone(20, noteFreqs[index], 150);  // 押した音を鳴らす

  if (index == pattern[playerIndex]) {
    // 正解
    playerIndex++;
    if (playerIndex >= patternLength) {
      delay(500);
      tone(20, 600, 200);  // ラウンドクリア音
      addNewStep();
      playPattern();
    }
  } else {
    // 不正解
    tone(20, 150, 500);   // 不正解音
    delay(1000);
    patternLength = 1;
    pattern[0] = random(0, numButtons);  // 手本を1音から作り直す
    playPattern();
  }
}