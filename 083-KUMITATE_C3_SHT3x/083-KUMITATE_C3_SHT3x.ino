#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_SHT31.h>

// ==================================================
// ピン設定
// ==================================================

#define I2C_SDA_PIN  1
#define I2C_SCL_PIN  2
#define SWITCH_PIN   8

// ==================================================
// 測定間隔
// ==================================================

// SHT30の読み取り間隔
const unsigned long SENSOR_INTERVAL_MS = 1000;

// グラフへ記録する間隔
const unsigned long GRAPH_INTERVAL_MS = 10000;

// 10秒間隔で5分間を表示
// 5分 = 300秒
// 0秒から300秒までなので31点
const int HISTORY_SIZE = 31;

// ==================================================
// OLED・SHT30
// ==================================================

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

Adafruit_SHT31 sht30 = Adafruit_SHT31();

// ==================================================
// 測定データ
// ==================================================

float currentTemperature = 0.0f;
float currentHumidity = 0.0f;

bool sensorDataValid = false;

// 温度履歴を保存するリングバッファ
float temperatureHistory[HISTORY_SIZE];

int historyCount = 0;
int historyWritePosition = 0;

// ==================================================
// 画面切り替え
// ==================================================

enum DisplayMode {
  MODE_REALTIME,
  MODE_GRAPH
};

DisplayMode displayMode = MODE_REALTIME;

// ==================================================
// タイマー
// ==================================================

unsigned long lastSensorTime = 0;
unsigned long lastGraphTime = 0;

// ==================================================
// スイッチのチャタリング対策
// ==================================================

bool lastSwitchReading = HIGH;
bool stableSwitchState = HIGH;

unsigned long switchChangeTime = 0;

const unsigned long DEBOUNCE_TIME_MS = 30;

// ==================================================
// 温度履歴へ追加
// ==================================================

void addTemperatureHistory(float temperature)
{
  temperatureHistory[historyWritePosition] = temperature;

  historyWritePosition++;

  if (historyWritePosition >= HISTORY_SIZE) {
    historyWritePosition = 0;
  }

  if (historyCount < HISTORY_SIZE) {
    historyCount++;
  }
}

// ==================================================
// 古い順に温度を取得
// ==================================================

float getHistoryTemperature(int position)
{
  int index;

  if (historyCount < HISTORY_SIZE) {
    index = position;
  } else {
    index = historyWritePosition + position;

    if (index >= HISTORY_SIZE) {
      index -= HISTORY_SIZE;
    }
  }

  return temperatureHistory[index];
}

// ==================================================
// スイッチ処理
// ==================================================

void updateSwitch()
{
  bool reading = digitalRead(SWITCH_PIN);

  if (reading != lastSwitchReading) {
    switchChangeTime = millis();
    lastSwitchReading = reading;
  }

  if ((millis() - switchChangeTime) >= DEBOUNCE_TIME_MS) {
    if (reading != stableSwitchState) {
      stableSwitchState = reading;

      // スイッチを押した瞬間
      if (stableSwitchState == LOW) {
        if (displayMode == MODE_REALTIME) {
          displayMode = MODE_GRAPH;
        } else {
          displayMode = MODE_REALTIME;
        }
      }
    }
  }
}

// ==================================================
// SHT30の測定
// ==================================================

void updateSensor()
{
  unsigned long now = millis();

  if (
    lastSensorTime != 0 &&
    now - lastSensorTime < SENSOR_INTERVAL_MS
  ) {
    return;
  }

  lastSensorTime = now;

  float temperature = sht30.readTemperature();
  float humidity = sht30.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    sensorDataValid = false;
    Serial.println("SHT30の測定に失敗しました");
    return;
  }

  currentTemperature = temperature;
  currentHumidity = humidity;
  sensorDataValid = true;

  Serial.print("温度: ");
  Serial.print(currentTemperature, 1);
  Serial.print(" C  湿度: ");
  Serial.print(currentHumidity, 1);
  Serial.println(" %");

  // 最初の測定値をすぐに履歴へ追加
  if (historyCount == 0) {
    addTemperatureHistory(currentTemperature);
    lastGraphTime = now;
  }
}

// ==================================================
// 10秒ごとの履歴記録
// ==================================================

void updateTemperatureHistory()
{
  if (!sensorDataValid || historyCount == 0) {
    return;
  }

  unsigned long now = millis();

  if (now - lastGraphTime >= GRAPH_INTERVAL_MS) {
    lastGraphTime += GRAPH_INTERVAL_MS;

    addTemperatureHistory(currentTemperature);
  }
}

// ==================================================
// センサーエラー表示
// ==================================================

void displaySensorError()
{
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_japanese1);
  oled.drawUTF8(0, 23, "センサーの");
  oled.drawUTF8(0, 48, "測定エラー");

  oled.sendBuffer();
}

// ==================================================
// リアルタイムモニタ
// ==================================================

void displayRealtimeMonitor()
{
  char temperatureText[12];
  char humidityText[12];

  snprintf(
    temperatureText,
    sizeof(temperatureText),
    "%.1f",
    currentTemperature
  );

  snprintf(
    humidityText,
    sizeof(humidityText),
    "%.1f",
    currentHumidity
  );

  oled.clearBuffer();

  // 温度見出し
  oled.setFont(u8g2_font_unifont_t_japanese1);
  oled.drawUTF8(0, 14, "TEMP");

  // 温度の数値
  oled.setFont(u8g2_font_logisoso24_tf);
  oled.drawStr(4, 40, temperatureText);

  int temperatureWidth = oled.getStrWidth(temperatureText);
  int unitX = 4 + temperatureWidth + 3;

  // ℃を円とCで描画
  oled.drawCircle(unitX + 3, 21, 2);

  oled.setFont(u8g2_font_helvB12_tf);
  oled.drawStr(unitX + 8, 39, "C");

  // 湿度
  oled.setFont(u8g2_font_unifont_t_japanese1);
  oled.drawUTF8(0, 62, "HMD");

  oled.setFont(u8g2_font_helvB14_tf);
  oled.drawStr(53, 61, humidityText);

  int humidityWidth = oled.getStrWidth(humidityText);
  oled.drawStr(53 + humidityWidth + 5, 61, "%");

  oled.sendBuffer();
}

// ==================================================
// 温度グラフ
// ==================================================

void displayTemperatureGraph()
{
  oled.clearBuffer();

  // グラフの表示領域
  const int graphLeft = 22;
  const int graphRight = 127;
  const int graphTop = 15;
  const int graphBottom = 52;

  // タイトル
  oled.setFont(u8g2_font_unifont_t_japanese1);
  oled.drawUTF8(0, 12, "温度");

  oled.setFont(u8g2_font_5x7_tf);
  oled.drawStr(38, 10, "5min");

  if (historyCount == 0) {
    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(28, 38, "NO DATA");
    oled.sendBuffer();
    return;
  }

  // 最小値・最大値を調べる
  float minimumTemperature = getHistoryTemperature(0);
  float maximumTemperature = getHistoryTemperature(0);

  for (int i = 1; i < historyCount; i++) {
    float temperature = getHistoryTemperature(i);

    if (temperature < minimumTemperature) {
      minimumTemperature = temperature;
    }

    if (temperature > maximumTemperature) {
      maximumTemperature = temperature;
    }
  }

  // グラフが平らになりすぎないよう最低2℃の幅を確保
  float centerTemperature =
    (minimumTemperature + maximumTemperature) / 2.0f;

  if (
    maximumTemperature - minimumTemperature < 2.0f
  ) {
    minimumTemperature = centerTemperature - 1.0f;
    maximumTemperature = centerTemperature + 1.0f;
  } else {
    // 上下に少し余白を追加
    minimumTemperature -= 0.5f;
    maximumTemperature += 0.5f;
  }

  // グラフ枠
  oled.drawFrame(
    graphLeft,
    graphTop,
    graphRight - graphLeft,
    graphBottom - graphTop
  );

  // 最小値・最大値
  char maximumText[8];
  char minimumText[8];

  snprintf(
    maximumText,
    sizeof(maximumText),
    "%.0f",
    maximumTemperature
  );

  snprintf(
    minimumText,
    sizeof(minimumText),
    "%.0f",
    minimumTemperature
  );

  oled.setFont(u8g2_font_5x7_tf);
  oled.drawStr(0, graphTop + 6, maximumText);
  oled.drawStr(0, graphBottom, minimumText);

  // グラフを描画
  int previousX = 0;
  int previousY = 0;

  for (int i = 0; i < historyCount; i++) {
    float temperature = getHistoryTemperature(i);

    int x;

    // 31点分の位置を固定し、データがたまる様子も表示
    x = graphLeft +
        ((graphRight - graphLeft - 1) * i) /
        (HISTORY_SIZE - 1);

    float normalized =
      (temperature - minimumTemperature) /
      (maximumTemperature - minimumTemperature);

    int y = graphBottom - 1 -
            (int)(
              normalized *
              (graphBottom - graphTop - 2)
            );

    y = constrain(
      y,
      graphTop + 1,
      graphBottom - 1
    );

    // 測定点
    oled.drawPixel(x, y);

    // 前の測定点と線でつなぐ
    if (i > 0) {
      oled.drawLine(
        previousX,
        previousY,
        x,
        y
      );
    }

    previousX = x;
    previousY = y;
  }

  // 時間表示
  oled.setFont(u8g2_font_5x7_tf);
  oled.drawStr(graphLeft, 63, "-5m");
  oled.drawStr(108, 63, "now");

  oled.sendBuffer();
}

// ==================================================
// setup
// ==================================================

void setup()
{
  Serial.begin(115200);
  delay(500);

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // OLED開始
  oled.setI2CAddress(0x3C * 2);
  oled.begin();
  oled.enableUTF8Print();

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_japanese1);
  oled.drawUTF8(0, 24, "温湿度モニター");
  oled.drawUTF8(0, 50, "起動中...");
  oled.sendBuffer();

  // SHT30開始
  if (!sht30.begin(0x44)) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(0, 25, "SHT30 ERROR");
    oled.drawStr(0, 48, "ADDRESS: 0x44");
    oled.sendBuffer();

    Serial.println("SHT30が見つかりません");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("SHT30を認識しました");

  // 最初の測定を実行
  updateSensor();

  delay(500);
}

// ==================================================
// loop
// ==================================================

void loop()
{
  updateSwitch();
  updateSensor();
  updateTemperatureHistory();

  if (!sensorDataValid) {
    displaySensorError();
  } else if (displayMode == MODE_REALTIME) {
    displayRealtimeMonitor();
  } else {
    displayTemperatureGraph();
  }

  delay(20);
}