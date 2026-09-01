#include "DemoProgram.h"
#include "PinDefinitions.h"

DemoProgram::DemoProgram(Adafruit_SSD1306& d, Adafruit_NeoPixel& p):display(d),pixels(p){}

void DemoProgram::screen(const char* a,const char* b,const char* c,const char* d){
  display.clearDisplay();
  text.setCursor(0,14);text.print(a);text.setCursor(0,30);text.print(b);
  text.setCursor(0,46);text.print(c);text.setCursor(0,62);text.print(d);display.display();
}

void DemoProgram::waitRelease(){
  while(!digitalRead(KUMITATE_PIN_SW1)||!digitalRead(KUMITATE_PIN_SW2)||!digitalRead(KUMITATE_PIN_SW3)||!digitalRead(KUMITATE_PIN_SW4))delay(5);
  delay(35);
}

int8_t DemoProgram::readButton(uint32_t timeout){
  uint32_t start=millis();
  while(!timeout||millis()-start<timeout){
    for(uint8_t i=0;i<4;i++)if(!digitalRead(KUMITATE_SWITCH_PINS[i])){
      delay(25);if(!digitalRead(KUMITATE_SWITCH_PINS[i])){while(!digitalRead(KUMITATE_SWITCH_PINS[i]))delay(5);delay(25);return i;}
    }
    delay(2);
  }
  return -1;
}

void DemoProgram::lightOne(uint8_t index,uint32_t color){pixels.clear();pixels.setPixelColor(index,color);pixels.show();}

void DemoProgram::showMenu(){screen("KUMITATE-C3","1:反射 2:記憶","3:暗号解除","4:10秒ぴったり");}

void DemoProgram::begin(){
  text.begin(display);text.setFont(u8g2_font_unifont_t_japanese3);text.setForegroundColor(SSD1306_WHITE);
  // NVSから、電源断前の直前得点と最高得点を復元する。
  preferences.begin("kumitate", false);
  lastReactionScore = preferences.getShort("lastScore", 0);
  reactionBestScore = preferences.getUShort("bestScore", 0);
  memoryBestLevel = preferences.getUChar("memoryBest", 0);
  timingBestError = preferences.getUInt("timingBest", UINT32_MAX);
  randomSeed(analogRead(KUMITATE_PIN_EXT4)+micros());
  showMenu();waitRelease();lastAnimation=millis();
}

void DemoProgram::update(){
  for(uint8_t i=0;i<4;i++)if(!digitalRead(KUMITATE_SWITCH_PINS[i])){
    while(!digitalRead(KUMITATE_SWITCH_PINS[i]))delay(5);delay(30);

    // 待機アニメーションを完全に消してから各モードへ入る。
    pixels.clear();
    pixels.show();
    noTone(KUMITATE_PIN_BUZZER);
    delay(80);

    if(i==0)reactionGame();else if(i==1)memoryGame();else if(i==2)codeBreaker();else timingGame();
    pixels.clear();pixels.show();showMenu();waitRelease();return;
  }
  if(millis()-lastAnimation>=140){
    lastAnimation=millis();pixels.clear();
    pixels.setPixelColor(animationStep%4,pixels.Color(0,32,90));
    pixels.setPixelColor((animationStep+3)%4,pixels.Color(18,0,35));pixels.show();animationStep++;
  }
}

void DemoProgram::reactionGame(){
  constexpr uint8_t rounds = 10;
  constexpr uint8_t fakeOutCount = 2;
  int16_t totalScore = 0;

  // 全10ラウンドのうち、重複しない2ラウンドだけをフェイントにする。
  bool fakeOutRounds[rounds] = {};
  for (uint8_t i = 0; i < fakeOutCount; i++) {
    uint8_t position;
    do {
      position = random(0, rounds);
    } while (fakeOutRounds[position]);
    fakeOutRounds[position] = true;
  }

  pixels.clear(); pixels.show();
  char currentBestText[32];
  snprintf(currentBestText, sizeof(currentBestText), "最高:%u点", reactionBestScore);
  screen("反射ゲーム", "緑:押す 赤:待つ", currentBestText, "SW1:開始");
  if (readButton() != 0) return;

  // カウントダウン
  for (int count = 3; count >= 1; count--) {
    char number[8];
    snprintf(number, sizeof(number), "%d", count);
    screen("準備してね", "", number, "");
    tone(KUMITATE_PIN_BUZZER, 700 + (3 - count) * 200, 80);
    delay(650);
  }
  screen("開始！", "全10回", "", "");
  tone(KUMITATE_PIN_BUZZER, 1400, 120);
  delay(500);

  for (uint8_t round = 1; round <= rounds; round++) {
    char roundText[40];
    snprintf(roundText, sizeof(roundText), "第%u回 / 全%u回", round, rounds);
    screen(roundText, "待って...", "", "");
    pixels.clear(); pixels.show();
    delay(random(900, 2300));

    uint8_t target = random(0, 4);
    bool fakeOut = fakeOutRounds[round - 1];
    uint32_t color = fakeOut ? pixels.Color(255, 0, 0) : pixels.Color(0, 255, 0);
    lightOne(target, color);
    tone(KUMITATE_PIN_BUZZER, fakeOut ? 350 : 1500, 60);

    // 後半ほど制限時間が短くなる（1900ms → 1000ms）。
    uint32_t timeLimit = 2000 - round * 100;
    uint32_t start = millis();
    int8_t pressed = readButton(timeLimit);
    uint32_t elapsed = millis() - start;
    int16_t roundScore = 0;
    const char* judgement = "";

    if (fakeOut) {
      if (pressed < 0) {
        roundScore = 75;
        judgement = "フェイント成功！";
        tone(KUMITATE_PIN_BUZZER, 2100, 120);
      } else {
        roundScore = -100;
        judgement = "赤は押さない！";
        tone(KUMITATE_PIN_BUZZER, 220, 400);
      }
    } else if (pressed == target) {
      // 200msまでは100点。以後10msごとに1点減点し、最低10点。
      if (elapsed <= 200) {
        roundScore = 100;
      } else {
        int calculatedScore = 100 - (int)((elapsed - 200) / 10);
        roundScore = calculatedScore < 10 ? 10 : calculatedScore;
      }
      judgement = "正解！";
      tone(KUMITATE_PIN_BUZZER, 2200, 100);
    } else if (pressed < 0) {
      judgement = "時間切れ！";
      tone(KUMITATE_PIN_BUZZER, 220, 400);
    } else {
      roundScore = -50;
      judgement = "押し間違い！";
      tone(KUMITATE_PIN_BUZZER, 220, 400);
    }

    totalScore += roundScore;
    pixels.clear(); pixels.show();
    char scoreText[48];
    if (!fakeOut && pressed == target)
      snprintf(scoreText, sizeof(scoreText), "反応:%lums %+d", (unsigned long)elapsed, roundScore);
    else
      snprintf(scoreText, sizeof(scoreText), "%+d 合計:%d", roundScore, totalScore);

    // 現在平均と、NVSに保存された最高得点の1ラウンド平均を比較する。
    // 除算による丸めを避けるため、totalScore/round > bestScore/10 を
    // totalScore*10 > bestScore*round の形で判定する。
    bool onRecordPace = (int32_t)totalScore * rounds >
                        (int32_t)reactionBestScore * round;
    screen(roundText, judgement, scoreText,
           onRecordPace ? "好調！記録ペース" : "");

    if (onRecordPace) {
      // 記録ペース中は水色の全灯と短い上昇音で知らせる。
      pixels.fill(pixels.Color(0, 100, 180));
      pixels.show();
      tone(KUMITATE_PIN_BUZZER, 2300, 70); delay(90);
      tone(KUMITATE_PIN_BUZZER, 2800, 100);
    }
    delay(1100);
    pixels.clear();
    pixels.show();
  }

  // 直前の合計得点は毎回保存し、最高得点は更新時だけ保存する。
  lastReactionScore = totalScore;
  preferences.putShort("lastScore", lastReactionScore);
  bool newRecord = totalScore > reactionBestScore;
  if (newRecord) {
    reactionBestScore = totalScore;
    preferences.putUShort("bestScore", reactionBestScore);
  }
  // 最高得点は950点（通常8回×100点＋フェイント2回×75点）。
  const char* rank = totalScore >= 850 ? "S" : totalScore >= 700 ? "A" : totalScore >= 550 ? "B" : totalScore >= 350 ? "C" : "D";
  char totalText[32], bestText[32], rankText[24];
  snprintf(totalText, sizeof(totalText), "合計:%d点", totalScore);
  snprintf(bestText, sizeof(bestText), "最高:%u点", reactionBestScore);
  snprintf(rankText, sizeof(rankText), "ランク:%s", rank);
  screen(newRecord ? "新記録！" : "ゲーム終了", totalText, bestText, rankText);
  if (newRecord) {
    celebrateNewRecord();
  } else {
    tone(KUMITATE_PIN_BUZZER, 1200, 100); delay(140);
    tone(KUMITATE_PIN_BUZZER, 1600, 100); delay(140);
    tone(KUMITATE_PIN_BUZZER, 2200, 180); delay(1800);
  }
  screen("ゲーム終了", totalText, bestText, "どれか押す");
  readButton();
}

void DemoProgram::celebrateNewRecord() {
  // Cメジャーの短いファンファーレ。音に合わせて4灯を順番に光らせる。
  const uint16_t notes[] = {523, 659, 784, 1047, 784, 1047, 1319};
  const uint16_t durations[] = {140, 140, 140, 260, 120, 180, 500};
  const uint32_t colors[] = {
    pixels.Color(255, 0, 0), pixels.Color(255, 80, 0),
    pixels.Color(0, 255, 0), pixels.Color(0, 120, 255),
    pixels.Color(100, 0, 255), pixels.Color(255, 0, 100),
    pixels.Color(255, 255, 255)
  };

  for (uint8_t i = 0; i < 7; i++) {
    pixels.clear();
    pixels.setPixelColor(i % KUMITATE_RGB_COUNT, colors[i]);
    pixels.show();
    tone(KUMITATE_PIN_BUZZER, notes[i], durations[i]);
    delay(durations[i] + 45);
  }
  pixels.fill(pixels.Color(255, 255, 255));
  pixels.show();
  tone(KUMITATE_PIN_BUZZER, 1568, 650);
  delay(700);
  noTone(KUMITATE_PIN_BUZZER);
  pixels.clear();
  pixels.show();
  delay(500);
}

void DemoProgram::memoryGame(){
  uint8_t sequence[16];uint8_t length=1;for(uint8_t&i:sequence)i=random(0,4);
  char bestText[32];
  snprintf(bestText,sizeof(bestText),"最高:レベル%u",memoryBestLevel);
  screen("記憶ゲーム","光を覚える",bestText,"SW1:開始");if(readButton()!=0)return;
  while(length<=16){
    for(uint8_t i=0;i<length;i++){lightOne(sequence[i],pixels.Color(0,80,180));tone(KUMITATE_PIN_BUZZER,500+sequence[i]*250,130);delay(260);pixels.clear();pixels.show();delay(100);}
    screen("あなたの番","順番に押す","","");
    for(uint8_t i=0;i<length;i++){int8_t b=readButton(5000);if(b!=sequence[i]){tone(KUMITATE_PIN_BUZZER,220,500);finishMemoryGame(length-1,false);return;}lightOne(b,pixels.Color(0,180,40));tone(KUMITATE_PIN_BUZZER,500+b*250,100);delay(130);pixels.clear();pixels.show();}
    length++;char line[32];snprintf(line,sizeof(line),"レベル%uクリア",length-1);screen("正解！",line,"次へ...","");tone(KUMITATE_PIN_BUZZER,2000,120);delay(700);
  }
  finishMemoryGame(16,true);
}

void DemoProgram::finishMemoryGame(uint8_t clearedLevel,bool perfect){
  bool newRecord=clearedLevel>memoryBestLevel;
  if(newRecord){
    memoryBestLevel=clearedLevel;
    preferences.putUChar("memoryBest",memoryBestLevel);
  }

  char levelText[32],bestText[32];
  snprintf(levelText,sizeof(levelText),"到達:レベル%u",clearedLevel);
  snprintf(bestText,sizeof(bestText),"最高:レベル%u",memoryBestLevel);
  screen(newRecord?"新記録！":(perfect?"完全クリア！":"ゲーム終了"),levelText,bestText,newRecord?"おめでとう！":"どれか押す");

  if(newRecord){
    celebrateNewRecord();
    screen("記憶ゲーム終了",levelText,bestText,"どれか押す");
  }
  readButton();
}

void DemoProgram::codeBreaker(){
  constexpr uint8_t codeLength=4;
  constexpr uint8_t maxAttempts=10;
  uint8_t secret[codeLength];
  for(uint8_t i=0;i<codeLength;i++)secret[i]=random(0,4);

  screen("暗号解除ゲーム","4桁を推理","同じ数字もあり","SW1:開始");
  if(readButton()!=0)return;

  for(uint8_t attempt=1;attempt<=maxAttempts;attempt++){
    uint8_t guess[codeLength];
    char attemptText[32],inputText[32];
    snprintf(attemptText,sizeof(attemptText),"第%u回 / 全%u回",attempt,maxAttempts);

    for(uint8_t position=0;position<codeLength;position++){
      strcpy(inputText,"入力:");
      for(uint8_t i=0;i<codeLength;i++){
        char symbol[5];
        snprintf(symbol,sizeof(symbol)," %c",i<position?('1'+guess[i]):'_');
        strcat(inputText,symbol);
      }
      screen("暗号解除",attemptText,inputText,"SW1～SW4で入力");
      int8_t button=readButton();
      guess[position]=button;
      lightOne(button,pixels.Color(0,100,200));
      tone(KUMITATE_PIN_BUZZER,500+button*220,90);
      delay(120);
      pixels.clear();pixels.show();
    }

    uint8_t exact=0,misplaced=0;
    uint8_t secretCounts[4]={0,0,0,0};
    uint8_t guessCounts[4]={0,0,0,0};
    for(uint8_t i=0;i<codeLength;i++){
      if(guess[i]==secret[i])exact++;
      else{secretCounts[secret[i]]++;guessCounts[guess[i]]++;}
    }
    for(uint8_t value=0;value<4;value++)misplaced+=min(secretCounts[value],guessCounts[value]);

    if(exact==codeLength){
      char tries[24];snprintf(tries,sizeof(tries),"%u回で成功",attempt);
      screen("暗号解除成功！",tries,"","どれか押す");
      pixels.fill(pixels.Color(0,255,0));pixels.show();
      tone(KUMITATE_PIN_BUZZER,1047,120);delay(150);
      tone(KUMITATE_PIN_BUZZER,1319,120);delay(150);
      tone(KUMITATE_PIN_BUZZER,1568,350);delay(400);
      pixels.clear();pixels.show();readButton();return;
    }

    char exactText[32],misplacedText[32];
    snprintf(exactText,sizeof(exactText),"位置も一致:%u",exact);
    snprintf(misplacedText,sizeof(misplacedText),"数字だけ一致:%u",misplaced);
    screen("ヒント",exactText,misplacedText,"どれかで次へ");
    tone(KUMITATE_PIN_BUZZER,700+exact*250,120);
    readButton();
  }

  char answer[24];
  snprintf(answer,sizeof(answer),"正解:%u%u%u%u",secret[0]+1,secret[1]+1,secret[2]+1,secret[3]+1);
  screen("解除失敗",answer,"","どれか押す");
  pixels.fill(pixels.Color(255,0,0));pixels.show();
  for(uint8_t i=0;i<3;i++){tone(KUMITATE_PIN_BUZZER,220,180);delay(260);}
  pixels.clear();pixels.show();readButton();
}

void DemoProgram::timingGame(){
  char bestText[32];
  if(timingBestError==UINT32_MAX)strcpy(bestText,"最高:未記録");
  else snprintf(bestText,sizeof(bestText),"最高差:%lums",(unsigned long)timingBestError);

  screen("10秒ぴったり","体感で10秒",bestText,"SW1:開始");
  if(readButton()!=0)return;

  for(int count=3;count>=1;count--){
    char number[8];snprintf(number,sizeof(number),"%d",count);
    screen("準備",number,"","SW1で止める");
    tone(KUMITATE_PIN_BUZZER,700+(3-count)*180,70);delay(650);
  }

  pixels.clear();pixels.show();
  screen("計測中...","10秒と思ったら","SW1を押す","");
  tone(KUMITATE_PIN_BUZZER,1400,100);
  uint32_t start=millis();
  int8_t stopButton=-1;
  uint32_t stopTime=start;
  uint32_t lastTimeDisplay=0;
  bool timeDisplayHidden=false;
  for(;;){
    // 表示更新より先にスイッチを確認し、停止時刻を優先して取得する。
    for(uint8_t i=0;i<4;i++){
      if(!digitalRead(KUMITATE_SWITCH_PINS[i])){
        // 押下を検出した最初の瞬間を記録し、ボタンを離す時間は含めない。
        stopTime=millis();
        delay(25);
        if(!digitalRead(KUMITATE_SWITCH_PINS[i])){
          stopButton=i;
          while(!digitalRead(KUMITATE_SWITCH_PINS[i]))delay(5);
          delay(25);
          break;
        }
      }
    }
    if(stopButton>=0)break;

    uint32_t previewElapsed=millis()-start;
    if(previewElapsed<=2000){
      if(millis()-lastTimeDisplay>=50){
        lastTimeDisplay=millis();
        char elapsedText[32];
        snprintf(elapsedText,sizeof(elapsedText),"経過:%lu.%03lu秒",
                 (unsigned long)(previewElapsed/1000),
                 (unsigned long)(previewElapsed%1000));
        screen("計測中...",elapsedText,"10秒を狙う","SW1で停止");
      }
    }else if(!timeDisplayHidden){
      // 2秒を過ぎたら時間の数値を消し、以後は更新しない。
      timeDisplayHidden=true;
      screen("計測中...","時間は非表示","","SW1で停止");
    }
    delay(1);
  }
  uint32_t elapsed=stopTime-start;

  // SW1以外で停止した場合も時間は表示するが、記録対象にはしない。
  int32_t signedDifference=(int32_t)elapsed-10000;
  uint32_t error=signedDifference<0?-signedDifference:signedDifference;
  char timeText[32],differenceText[32];
  snprintf(timeText,sizeof(timeText),"時間:%lu.%03lu秒",(unsigned long)(elapsed/1000),(unsigned long)(elapsed%1000));
  snprintf(differenceText,sizeof(differenceText),"差:%+ldms",(long)signedDifference);

  bool valid=stopButton==0;
  bool newRecord=valid&&error<timingBestError;
  if(newRecord){
    timingBestError=error;
    preferences.putUInt("timingBest",timingBestError);
  }

  const char* rank=error<=10?"神！":error<=50?"ランクS":error<=100?"ランクA":error<=250?"ランクB":error<=500?"ランクC":"ランクD";
  screen(!valid?"SW1で止めてね":(newRecord?"新記録！":"計測結果"),timeText,differenceText,valid?rank:"記録対象外");

  if(!valid){
    tone(KUMITATE_PIN_BUZZER,220,350);delay(1300);
  }else if(newRecord){
    celebrateNewRecord();
    screen("10秒ゲーム終了",timeText,differenceText,"どれか押す");
  }else{
    uint32_t resultColor=error<=100?pixels.Color(0,255,0):error<=500?pixels.Color(255,100,0):pixels.Color(255,0,0);
    pixels.fill(resultColor);pixels.show();
    tone(KUMITATE_PIN_BUZZER,error<=100?2200:500,220);delay(1400);
    pixels.clear();pixels.show();
    screen("10秒ゲーム終了",timeText,differenceText,"どれか押す");
  }
  readButton();
}

// 現在はメニューから呼び出していないが、将来再利用できるよう残している。
void DemoProgram::sequencer(){
  uint8_t notes[8];screen("8音シーケンサ","8回押して","音を記録","");
  for(uint8_t i=0;i<8;i++){int8_t b=readButton();notes[i]=b;lightOne(b,pixels.Color(120,30*b,100));tone(KUMITATE_PIN_BUZZER,440+b*220,100);char line[24];snprintf(line,sizeof(line),"記録 %u / 8",i+1);screen("8音シーケンサ",line,"記録中...","");}
  screen("再生中","光と音の演奏","","見てください");delay(400);
  for(uint8_t repeat=0;repeat<2;repeat++)for(uint8_t i=0;i<8;i++){lightOne(notes[i],pixels.Color(120,30*notes[i],100));tone(KUMITATE_PIN_BUZZER,440+notes[i]*220,160);delay(230);pixels.clear();pixels.show();delay(40);}
  screen("再生終了","","","どれか押す");readButton();
}

// 現在はメニューから呼び出していないが、将来再利用できるよう残している。
void DemoProgram::lightShow(){
  screen("光と音ショー","12秒デモ","","見てください");
  uint32_t start=millis();uint16_t step=0;
  while(millis()-start<12000){
    for(uint8_t i=0;i<4;i++){uint8_t phase=(step+i*48)&255;pixels.setPixelColor(i,pixels.Color((sin(phase*0.02454)+1)*80,(sin((phase+85)*0.02454)+1)*80,(sin((phase+170)*0.02454)+1)*80));}
    pixels.show();tone(KUMITATE_PIN_BUZZER,300+(step%160)*18);delay(24);step+=3;
    if(!digitalRead(KUMITATE_PIN_SW4))break;
  }
  noTone(KUMITATE_PIN_BUZZER);pixels.clear();pixels.show();
}
