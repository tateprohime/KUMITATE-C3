#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

WebServer server(80);  // 80番はWeb通信で標準的に使われるポート番号

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("ブラウザでこのアドレスにアクセスしてください: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);   // トップページ
  server.on("/on", handleOn);   // LED点灯
  server.on("/off", handleOff); // LED消灯
  server.begin();
}

void loop() {
  server.handleClient();  // アクセスがないか、毎回確認する
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>KUMITATE-C3 LED Control</h1>";
  html += "<a href=\"/on\">LED ON</a><br>";
  html += "<a href=\"/off\">LED OFF</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(0, HIGH);
  handleRoot();  // 操作後、トップページの内容を返す
}

void handleOff() {
  digitalWrite(0, LOW);
  handleRoot();
}