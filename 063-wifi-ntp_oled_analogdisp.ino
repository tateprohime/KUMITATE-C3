#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

const char* ntpServer = "ntp.nict.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int centerX = 64;
const int centerY = 32;
const int clockRadius = 30;

void setup() {
  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeInfo;

  if (getLocalTime(&timeInfo)) {
    display.clearDisplay();
    display.drawCircle(centerX, centerY, clockRadius, SSD1306_WHITE);  // 文字盤の外枠

    float hourAngle = (timeInfo.tm_hour % 12) * 30 + timeInfo.tm_min * 0.5;  // 時針の角度
    float minAngle  = timeInfo.tm_min * 6;                                   // 分針の角度
    float secAngle  = timeInfo.tm_sec * 6;                                   // 秒針の角度

    drawHand(hourAngle, clockRadius * 0.5);  // 短い時針
    drawHand(minAngle,  clockRadius * 0.8);  // 長めの分針
    drawHand(secAngle,  clockRadius * 0.9);  // 一番長い秒針

    display.display();
  }

  delay(1000);
}

// 角度(度)と長さを指定して、時計の中心から1本の針を描く
void drawHand(float angleDeg, float length) {
  float angleRad = (angleDeg - 90) * PI / 180;  // 12時方向を基準(0度)にするための調整
  int x = centerX + cos(angleRad) * length;
  int y = centerY + sin(angleRad) * length;
  display.drawLine(centerX, centerY, x, y, SSD1306_WHITE);
}