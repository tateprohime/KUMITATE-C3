#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// ---------- ピン設定 ----------
constexpr uint8_t DHT_PIN  = 4;
constexpr uint8_t OLED_SDA = 1;
constexpr uint8_t OLED_SCL = 2;

// ---------- DHT11設定 ----------
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ---------- OLED設定 ----------
// KUMITATE-C3内蔵 SSD1306 128×64 OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

void showMessage(const char* line1, const char* line2 = nullptr)
{
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_japanese1);

  oled.drawUTF8(0, 20, line1);

  if (line2 != nullptr) {
    oled.drawUTF8(0, 45, line2);
  }

  oled.sendBuffer();
}

void setup()
{
  Serial.begin(115200);

  // KUMITATE-C3のOLED用I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  oled.setI2CAddress(0x3C * 2);
  oled.begin();

  dht.begin();

  showMessage("DHT11 起動中...");
  delay(2000);
}

void loop()
{
  // DHT11は高速で連続取得できないため2秒間隔で測定
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  oled.clearBuffer();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT11の読み取りに失敗しました");

    oled.setFont(u8g2_font_unifont_t_japanese1);
    oled.drawUTF8(0, 22, "センサーエラー");
    oled.drawUTF8(0, 48, "配線を確認");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" C, Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");

    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(0, 11, "DHT11 SENSOR");

    oled.drawHLine(0, 15, 128);

    oled.setFont(u8g2_font_helvB14_tf);

    oled.setCursor(0, 37);
    oled.print("T: ");
    oled.print(temperature, 1);
    oled.print(" C");

    oled.setCursor(0, 61);
    oled.print("H: ");
    oled.print(humidity, 1);
    oled.print(" %");
  }

  oled.sendBuffer();
  delay(2000);
}