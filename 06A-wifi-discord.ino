#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

//クミタテのDiscordウェブフックを初期値として代入しています
const char* webhookURL = "https://discord.com/api/webhooks/1531817931623633006/wvB-zVwZeQ0jcG2ESNWfN3Az1v4EyGzf2Ms_gX2DAUFPKl9x9m__vdezSRw4AKq0816t";

WiFiClientSecure client;

bool lastSw1State = HIGH;

void setup() {
  pinMode(21, INPUT);  // SW1
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi接続完了");

  client.setInsecure();  // HTTPS通信のため
}

void loop() {
  bool sw1State = digitalRead(21);

  if (lastSw1State == HIGH && sw1State == LOW) {
    sendDiscordMessage("ボタンが押されました！");
  }

  lastSw1State = sw1State;
}

void sendDiscordMessage(String message) {
  HTTPClient http;

  http.begin(client, webhookURL);
  http.addHeader("Content-Type", "application/json");  // 送るデータの種類を伝える

  String jsonBody = "{\"content\": \"" + message + "\"}";
  int httpCode = http.POST(jsonBody);

  Serial.print("httpCode: ");
  Serial.println(httpCode);

  http.end();
}
