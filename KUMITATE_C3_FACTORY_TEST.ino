#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

namespace Pin {
constexpr uint8_t LED=0, SDA=1, SCL=2, SW2=3, EXT4=4, EXT5=5;
constexpr uint8_t EXT6=6, EXT7=7, SW4=8, SW3=9, RGB=10, BUZZER=20, SW1=21;
}

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel pixels(4, Pin::RGB, NEO_GRB + NEO_KHZ800);
const uint8_t switches[] = {Pin::SW1, Pin::SW2, Pin::SW3, Pin::SW4};
enum Choice { CHOICE_OK, CHOICE_NG, CHOICE_SKIP };
uint16_t passCount, failCount, skipCount;

void screen(const char* a, const char* b="", const char* c="", const char* d="") {
  display.clearDisplay(); display.setTextColor(SSD1306_WHITE); display.setTextSize(1);
  display.setCursor(0,0); display.println(a); display.setCursor(0,16); display.println(b);
  display.setCursor(0,32); display.println(c); display.setCursor(0,48); display.println(d);
  display.display();
}

void waitRelease() {
  while (!digitalRead(Pin::SW1)||!digitalRead(Pin::SW2)||!digitalRead(Pin::SW3)||!digitalRead(Pin::SW4)) delay(10);
  delay(40);
}

bool waitButton(uint8_t pin, uint32_t timeout=0) {
  waitRelease(); uint32_t start=millis();
  while (!timeout || millis()-start<timeout) {
    if (!digitalRead(pin)) { delay(30); if (!digitalRead(pin)) { while(!digitalRead(pin)) delay(5); delay(30); return true; } }
    delay(5);
  }
  return false;
}

Choice ask(const char* item, const char* detail="") {
  screen(item, detail, "SW1:OK  SW2:NG", "SW3:SKIP"); waitRelease();
  for (;;) {
    if (!digitalRead(Pin::SW1)) { while(!digitalRead(Pin::SW1)) delay(5); delay(30); return CHOICE_OK; }
    if (!digitalRead(Pin::SW2)) { while(!digitalRead(Pin::SW2)) delay(5); delay(30); return CHOICE_NG; }
    if (!digitalRead(Pin::SW3)) { while(!digitalRead(Pin::SW3)) delay(5); delay(30); return CHOICE_SKIP; }
  }
}

void record(const char* item, Choice value) {
  if (value==CHOICE_OK) { passCount++; Serial.printf("[PASS] %s\n",item); }
  else if (value==CHOICE_NG) { failCount++; Serial.printf("[FAIL] %s\n",item); }
  else { skipCount++; Serial.printf("[SKIP] %s\n",item); }
}

void testOled() {
  screen("TEST: OLED", "Filling screen...", "", "Please watch");
  delay(500);

  // 左上から右下へ、横1列ずつ白く塗りつぶす。
  display.clearDisplay();
  for (int y = 0; y < 64; y++) {
    display.drawFastHLine(0, y, 128, SSD1306_WHITE);
    display.display();
    delay(20);
  }
  delay(800);

  // 右下から左上へ、横1列ずつ黒く戻す。
  for (int y = 63; y >= 0; y--) {
    display.drawFastHLine(0, y, 128, SSD1306_BLACK);
    display.display();
    delay(20);
  }
  delay(300);

  record("OLED", ask("OLED DISPLAY", "Fill / clear was OK?"));
}

void testLed() {
  screen("TEST: BLUE LED","Blinking 3 times","","Watch D8"); pinMode(Pin::LED,OUTPUT);
  for(int i=0;i<3;i++){digitalWrite(Pin::LED,HIGH);delay(350);digitalWrite(Pin::LED,LOW);delay(250);}
  record("BLUE LED",ask("BLUE LED D8","Did it blink x3?"));
}

void testBuzzer() {
  screen("TEST: BUZZER", "Frequency sweep", "300Hz - 3500Hz", "Please listen");

  // 低音から高音、高音から低音へ連続的に変化させる。
  // この往復スイープを2回実行する。
  for (int cycle = 0; cycle < 2; cycle++) {
    for (int frequency = 300; frequency <= 3500; frequency += 40) {
      tone(Pin::BUZZER, frequency);
      delay(6);
    }
    for (int frequency = 3500; frequency >= 300; frequency -= 40) {
      tone(Pin::BUZZER, frequency);
      delay(6);
    }
  }

  noTone(Pin::BUZZER);
  digitalWrite(Pin::BUZZER, LOW);
  record("BUZZER", ask("BUZZER BZ1", "Sweep sound was OK?"));
}

void testRgb() {
  screen("TEST: RGB LED","RED GREEN BLUE WHITE","","Check all 4 LEDs");
  pixels.begin();
  // 0～255のうち8に制限し、検査時のまぶしさを抑える。
  pixels.setBrightness(8);
  uint32_t colors[]={pixels.Color(255,0,0),pixels.Color(0,255,0),pixels.Color(0,0,255),pixels.Color(255,255,255)};
  for(uint32_t color:colors){pixels.fill(color);pixels.show();delay(800);} pixels.clear();pixels.show();
  record("RGB LED",ask("RGB LED D1-D4","All colors / 4 LEDs?"));
}

void testSwitches() {
  const char* names[]={"SW1","SW2","SW3","SW4"};
  for(int i=0;i<4;i++){
    // 検査対象スイッチの真上にあるRGB LEDだけを白く点灯する。
    pixels.clear();
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    pixels.show();

    char prompt[20]; snprintf(prompt,sizeof(prompt),"PRESS %s",names[i]);
    screen("TEST: SWITCH",prompt,"Waiting...","15 sec timeout"); bool good=waitButton(switches[i],15000);
    good?passCount++:failCount++; Serial.printf("[%s] %s\n",good?"PASS":"FAIL",names[i]);

    // 押下を検出した場合は緑、タイムアウトした場合は赤で結果を示す。
    pixels.clear();
    pixels.setPixelColor(i, good ? pixels.Color(0, 255, 0) : pixels.Color(255, 0, 0));
    pixels.show();
    screen(good?"SWITCH: PASS":"SWITCH: FAIL",names[i]);delay(500);
  }
  pixels.clear();
  pixels.show();
}

bool driveRead(uint8_t out,uint8_t in,uint8_t level){pinMode(out,OUTPUT);digitalWrite(out,level);pinMode(in,INPUT_PULLUP);delay(3);return digitalRead(in)==level;}
bool pairTest(uint8_t a,uint8_t b){bool good=driveRead(a,b,LOW)&&driveRead(a,b,HIGH)&&driveRead(b,a,LOW)&&driveRead(b,a,HIGH);pinMode(a,INPUT);pinMode(b,INPUT);return good;}
void showAuto(const char* item,bool good){good?passCount++:failCount++;screen(good?"AUTO TEST: PASS":"AUTO TEST: FAIL",item,"","SW1:NEXT");waitButton(Pin::SW1);}

void testLoopback(){
  screen("GPIO LOOPBACK?","J4: 4-5 and 6-7","SW1:TEST","SW3:SKIP");waitRelease();
  while(digitalRead(Pin::SW1)&&digitalRead(Pin::SW3))delay(5);
  if(!digitalRead(Pin::SW3)){while(!digitalRead(Pin::SW3))delay(5);skipCount++;return;}
  while(!digitalRead(Pin::SW1))delay(5);delay(30);
  showAuto("GPIO4-GPIO5",pairTest(Pin::EXT4,Pin::EXT5)); showAuto("GPIO6-GPIO7",pairTest(Pin::EXT6,Pin::EXT7));
}

void summary(){char s[30];snprintf(s,sizeof(s),"OK:%u NG:%u SKIP:%u",passCount,failCount,skipCount);screen(failCount?"RESULT: FAIL":"RESULT: PASS",s,"","SW4:RETEST");}

void runTests(){
  passCount=failCount=skipCount=0; screen("KUMITATE-C3","FACTORY TEST","","SW1:START");waitButton(Pin::SW1);
  testOled();testLed();testBuzzer();testRgb();testSwitches();testLoopback();summary();
}

void setup(){
  Serial.begin(115200);for(uint8_t pin:switches)pinMode(pin,INPUT_PULLUP);Wire.begin(Pin::SDA,Pin::SCL);Wire.setClock(100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){Serial.println("OLED 0x3C NOT FOUND");pinMode(Pin::LED,OUTPUT);for(;;){digitalWrite(Pin::LED,HIGH);delay(150);digitalWrite(Pin::LED,LOW);delay(850);}}
  runTests();
}

void loop(){if(!digitalRead(Pin::SW4)){while(!digitalRead(Pin::SW4))delay(5);delay(30);runTests();}delay(10);}
