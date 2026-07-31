#include <WiFiManager.h>

void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;

  // 【注意】保存済みのWiFi設定を毎回クリアしたい場合は、次の行を有効にしてください。
  // クリアしない場合(通常の運用時)は、この行をコメントアウトしてください。
  wifiManager.resetSettings();

  // 保存済みの設定があればそれで接続を試み、なければ
  // "KUMITATE-C3-Setup"という名前のWiFiを一時的に立ち上げる
  bool connected = wifiManager.autoConnect("KUMITATE-C3-Setup");

  if (!connected) {
    Serial.println("接続に失敗しました。リセットします");
    ESP.restart();
  }

  Serial.println("WiFiに接続できました！");
  Serial.print("IPアドレス: ");
  Serial.println(WiFi.localIP());
}

void loop() {
}