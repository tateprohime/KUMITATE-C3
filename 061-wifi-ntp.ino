#include <WiFi.h>
#include <time.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

const char* ntpServer = "ntp.nict.jp";  // 日本の標準時を配信しているサーバ
const long gmtOffset_sec = 9 * 3600;    // 日本時間はUTCより9時間進んでいる
const int daylightOffset_sec = 0;       // 日本にはサマータイムがないので0

void setup() {
  Serial.begin(115200);

  Serial.print("WiFi接続中");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi接続完了");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeInfo;

  if (getLocalTime(&timeInfo)) {
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
      timeInfo.tm_year + 1900,
      timeInfo.tm_mon + 1,
      timeInfo.tm_mday,
      timeInfo.tm_hour,
      timeInfo.tm_min,
      timeInfo.tm_sec);
  } else {
    Serial.println("時刻を取得できませんでした");
  }

  delay(1000);
}