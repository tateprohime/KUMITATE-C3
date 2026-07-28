#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

unsigned long lastToggle = 0;
bool ledState = false;

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi接続完了");

  ArduinoOTA.setHostname("kumitate-c3");  // ネットワーク上での名前

  ArduinoOTA.onStart([]() {
    Serial.println("OTA更新を開始します");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA更新が完了しました");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("進捗: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("エラーが発生しました[%u]\n", error);
  });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();  // OTA書き込み要求がないか、毎回確認する

  // 通常の動作(LED点滅)は、これまで通りmillis()で非ブロッキングに
  if (millis() - lastToggle >= 500) {
    ledState = !ledState;
    digitalWrite(0, ledState);
    lastToggle = millis();
  }
}