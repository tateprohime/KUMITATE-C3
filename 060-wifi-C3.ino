#include <WiFi.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

void setup() {
  Serial.begin(115200);

  Serial.print("接続中");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFiに接続できました！");
  Serial.print("IPアドレス: ");
  Serial.println(WiFi.localIP());
}

void loop() {
}