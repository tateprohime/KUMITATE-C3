#include <WiFi.h>
#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(1000);

  preferences.begin("wifi-config", false);  // "wifi-config"という名前の保存領域を開く

  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");

  if (ssid == "") {
    // 保存された設定がなければ、シリアルモニタから入力してもらう
    Serial.println("WiFiのSSIDを入力してEnterを押してください:");
    while (Serial.available() == 0) { }
    ssid = Serial.readStringUntil('\n');
    ssid.trim();  // 改行や余分な空白を取り除く

    Serial.println("WiFiのパスワードを入力してEnterを押してください:");
    while (Serial.available() == 0) { }
    password = Serial.readStringUntil('\n');
    password.trim();

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    Serial.println("設定を保存しました");
  } else {
    Serial.println("保存済みの設定を使用します: " + ssid);
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("接続中");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFiに接続できました！");
}

void loop() {
}