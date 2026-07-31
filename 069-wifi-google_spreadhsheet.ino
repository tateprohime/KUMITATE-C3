#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

const char* scriptURL = "ここに発行されたGoogle Apps ScriptのURLを入れてください";

int pressCount = 0;
bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi接続完了");
}

void loop() {
  bool sw1State = digitalRead(21);

  if (lastSw1State == HIGH && sw1State == LOW) {
    pressCount++;
    sendToSheet(pressCount);
  }

  lastSw1State = sw1State;
}

void sendToSheet(int count) {
  HTTPClient http;
  String url = String(scriptURL) + "?count=" + String(count);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.println(String(count) + "回目を記録しました");
  } else {
    Serial.println("送信に失敗しました");
  }

  http.end();
}