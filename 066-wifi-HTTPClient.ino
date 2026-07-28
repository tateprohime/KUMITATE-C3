#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "ここに自分のSSIDを入れてください";
const char* password = "ここに自分のパスワードを入れてください";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// お住まいの地域の緯度・経度に書き換えてください(例は東京)
const String latitude = "35.6895";
const String longitude = "139.6917";

void setup() {
  Wire.begin(1, 2);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  fetchWeather();
  delay(600000);  // 10分ごとに更新(APIへの負荷を抑えるため)
}

void fetchWeather() {
  HTTPClient http;
  String url = "http://api.open-meteo.com/v1/forecast?latitude=" + latitude +
               "&longitude=" + longitude + "&current_weather=true";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    JsonDocument doc;
    deserializeJson(doc, payload);

    float temperature = doc["current_weather"]["temperature"];
    float windspeed = doc["current_weather"]["windspeed"];

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Weather Now");

    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(temperature, 1);
    display.print(" C");

    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Wind: ");
    display.print(windspeed, 1);
    display.print(" km/h");

    display.display();
  } else {
    Serial.println("天気情報の取得に失敗しました");
  }

  http.end();
}