#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

const char* ntpServer = "ntp.nict.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi connecting...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeInfo;

  if (getLocalTime(&timeInfo)) {
    char dateStr[11];  // "YYYY/MM/DD" + 終端文字
    char timeStr[9];   // "HH:MM:SS" + 終端文字

    sprintf(dateStr, "%04d/%02d/%02d",
      timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    sprintf(timeStr, "%02d:%02d:%02d",
      timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(dateStr);

    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(timeStr);

    display.display();
  }

  delay(1000);  // 1秒ごとに更新
}