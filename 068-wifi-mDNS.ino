#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

WebServer server(80);

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("IPアドレス: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("kumitate-c3")) {
    Serial.println("mDNSを開始しました: http://kumitate-c3.local");
  } else {
    Serial.println("mDNSの開始に失敗しました");
  }

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
}

void loop() {
  server.handleClient();
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
  handleRoot();
}

void handleOff() {
  digitalWrite(0, LOW);
  handleRoot();
}