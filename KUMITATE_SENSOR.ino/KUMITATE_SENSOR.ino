#include <Wire.h>

// ============================================================
// KUMITATE-SENSOR 動作確認
// ============================================================

// 使用環境に応じて変更してください（初期値：KUMITATE-C3用）
#define SDA_PIN 1
#define SCL_PIN 2

// I2C Address
#define BH1750_ADDR   0x23
#define MSA3S02_ADDR  0x62
#define SHT40_ADDR    0x44

// ============================================================
// I2Cデバイス存在確認
// ============================================================
bool checkI2CDevice(uint8_t address){
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

// ============================================================
// BH1750
// ============================================================
void initBH1750(){
  Wire.beginTransmission(BH1750_ADDR);
  Wire.write(0x10);  // Continuous High Resolution Mode
  Wire.endTransmission();
}

float readBH1750(){
  Wire.requestFrom(BH1750_ADDR, 2);
  if (Wire.available() < 2) {
    return -1;
  }

  uint16_t raw = Wire.read() << 8;
  raw |= Wire.read();

  return raw / 1.2;
}

// ============================================================
// SHT40
// ============================================================
bool readSHT40(float &temperature, float &humidity){
  Wire.beginTransmission(SHT40_ADDR);
  Wire.write(0xFD);  // High precision measurement

  if (Wire.endTransmission() != 0) {
    return false;
  }

  delay(10);

  Wire.requestFrom(SHT40_ADDR, 6);

  if (Wire.available() < 6) {
    return false;
  }

  uint16_t rawTemp = Wire.read() << 8;
  rawTemp |= Wire.read();

  Wire.read(); // Temperature CRC

  uint16_t rawHumidity = Wire.read() << 8;
  rawHumidity |= Wire.read();

  Wire.read(); // Humidity CRC

  temperature = -45.0 + 175.0 * ((float)rawTemp / 65535.0);

  humidity = -6.0 + 125.0 * ((float)rawHumidity / 65535.0);

  // 念のため表示値を0～100%に制限
  if (humidity > 100.0) humidity = 100.0;
  if (humidity < 0.0)   humidity = 0.0;

  return true;
}

// ============================================================
// I2C Scan
// ============================================================
void scanI2C(){
  Serial.println();
  Serial.println("===== I2C Scan =====");

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found : 0x");

      if (address < 16) {
        Serial.print("0");
      }

      Serial.println(address, HEX);
    }
  }

  Serial.println("====================");
}

// ============================================================
// MSA3S02
// ============================================================

void writeMSA3S02(uint8_t reg, uint8_t value){
  Wire.beginTransmission(MSA3S02_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readMSA3S02Register(uint8_t reg){
  Wire.beginTransmission(MSA3S02_ADDR);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0) {
    return 0;
  }

  Wire.requestFrom(MSA3S02_ADDR, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0;
}

// 12bit 2の補数データを読み出す
int16_t readMSA3S02Axis(uint8_t lsbReg){
  uint8_t lsb = readMSA3S02Register(lsbReg);
  uint8_t msb = readMSA3S02Register(lsbReg + 1);

  // MSB = bit11～4
  // LSB = bit3～0
  int16_t value = ((int16_t)msb << 4) | (lsb & 0x0F);

  // 12bit符号拡張
  if (value & 0x0800) {
    value |= 0xF000;
  }

  return value;
}

void initMSA3S02(){
  // ±2g
  writeMSA3S02(0x0F, 0x00);

  // Normal mode
  // PWR_MODE = 00
  uint8_t reg11 = readMSA3S02Register(0x11);
  reg11 &= ~(0xC0);
  writeMSA3S02(0x11, reg11);

  delay(10);
}

void readMSA3S02(int16_t &x, int16_t &y, int16_t &z){
  x = readMSA3S02Axis(0x02);
  y = readMSA3S02Axis(0x04);
  z = readMSA3S02Axis(0x06);
}

// ============================================================
// setup
// ============================================================
void setup(){
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" KUMITATE-SENSOR TEST");
  Serial.println("==============================");

  Wire.begin(SDA_PIN, SCL_PIN);

  scanI2C();

  Serial.println();
  Serial.println("Device check");

  Serial.print("BH1750  [0x23] : ");
  Serial.println(checkI2CDevice(BH1750_ADDR) ? "OK" : "NOT FOUND");

  initMSA3S02();
  Serial.print("MSA3S02 [0x62] : ");
  Serial.println(checkI2CDevice(MSA3S02_ADDR) ? "OK" : "NOT FOUND");

  Serial.print("SHT40-A [0x44] : ");
  Serial.println(checkI2CDevice(SHT40_ADDR) ? "OK" : "NOT FOUND");

  initBH1750();

  delay(200);
}


// ============================================================
// loop
// ============================================================
void loop(){
  float temperature;
  float humidity;
  float lux;

  Serial.println();
  Serial.println("---------- SENSOR ----------");

  // SHT40
  if (readSHT40(temperature, humidity)) {
    Serial.print("Temperature : ");
    Serial.print(temperature, 2);
    Serial.println(" degC");

    Serial.print("Humidity    : ");
    Serial.print(humidity, 2);
    Serial.println(" %");
  } else {
    Serial.println("SHT40 : ERROR");
  }

  // BH1750
  lux = readBH1750();
  if (lux >= 0) {
    Serial.print("Light       : ");
    Serial.print(lux, 1);
    Serial.println(" lx");
  } else {
    Serial.println("BH1750 : ERROR");
  }

  // MSA3S02
  int16_t accX;
  int16_t accY;
  int16_t accZ;
  if (checkI2CDevice(MSA3S02_ADDR)) {
    readMSA3S02(accX, accY, accZ);
    Serial.print("Accel X     : ");
    Serial.println(accX);
    Serial.print("Accel Y     : ");
    Serial.println(accY);
    Serial.print("Accel Z     : ");
    Serial.println(accZ);
  } else {
    Serial.println("MSA3S02 : ERROR");
  }
  Serial.println("----------------------------");

  delay(1000);
}
