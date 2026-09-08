#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <esp_arduino_version.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

constexpr uint8_t OLED_SDA = 1;
constexpr uint8_t OLED_SCL = 2;
constexpr uint8_t OLED_ADDRESS = 0x3C;

constexpr uint8_t PIN_SW1 = 21;  // ON/OFF
constexpr uint8_t PIN_SW2 = 3;   // Speed UP
constexpr uint8_t PIN_SW3 = 9;   // Speed DOWN
constexpr uint8_t PIN_SW4 = 8;   // Direction

constexpr uint8_t PIN_L9110_IA = 4;
constexpr uint8_t PIN_L9110_IB = 5;

constexpr uint32_t PWM_FREQUENCY = 20000;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint8_t MIN_PWM_DUTY = 110;  // 255 * 43% = 109.65
constexpr uint8_t SPEED_STEP = 16;
constexpr unsigned long DIRECTION_WAIT_MS = 150;
constexpr unsigned long DEBOUNCE_MS = 40;

#if ESP_ARDUINO_VERSION_MAJOR < 3
constexpr uint8_t PWM_CHANNEL_IA = 0;
constexpr uint8_t PWM_CHANNEL_IB = 1;
#endif

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

bool motorEnabled = false;
bool motorForward = true;
uint8_t logicalSpeed = 128;  // User setting: 0..255 = 0..100%
uint8_t appliedDuty = 0;     // Actual PWM: OFF=0, ON=110..255
bool pwmAttachedIA = false;
bool pwmAttachedIB = false;

constexpr char BLE_DEVICE_NAME[] = "KUMITATE-C3-CAR";
constexpr char BLE_SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char BLE_RX_UUID[] = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char BLE_TX_UUID[] = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

BLECharacteristic *bleTxCharacteristic = nullptr;
volatile bool bleConnected = false;
volatile bool restartAdvertising = false;

struct BleCommand {
  char text[32];
};

QueueHandle_t bleCommandQueue = nullptr;

enum ButtonIndex {
  BUTTON_POWER,
  BUTTON_SPEED_UP,
  BUTTON_SPEED_DOWN,
  BUTTON_DIRECTION,
  BUTTON_COUNT
};

const uint8_t buttonPins[BUTTON_COUNT] = {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4};
bool buttonStable[BUTTON_COUNT] = {HIGH, HIGH, HIGH, HIGH};
bool buttonPrevious[BUTTON_COUNT] = {HIGH, HIGH, HIGH, HIGH};
unsigned long buttonChangedAt[BUTTON_COUNT] = {0, 0, 0, 0};

uint8_t logicalToPwm(uint8_t value) {
  // Logical 0% -> physical 43%, logical 100% -> physical 100%.
  return static_cast<uint8_t>(map(value, 0, 255, MIN_PWM_DUTY, 255));
}

void initializePwm() {
#if ESP_ARDUINO_VERSION_MAJOR < 3
  ledcSetup(PWM_CHANNEL_IA, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_IB, PWM_FREQUENCY, PWM_RESOLUTION);
#endif
}

void detachOutputs() {
  if (pwmAttachedIA) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach(PIN_L9110_IA);
#else
    ledcDetachPin(PIN_L9110_IA);
#endif
    pwmAttachedIA = false;
  }

  if (pwmAttachedIB) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach(PIN_L9110_IB);
#else
    ledcDetachPin(PIN_L9110_IB);
#endif
    pwmAttachedIB = false;
  }
}

void stopMotor() {
  detachOutputs();
  pinMode(PIN_L9110_IA, OUTPUT);
  pinMode(PIN_L9110_IB, OUTPUT);
  digitalWrite(PIN_L9110_IA, LOW);
  digitalWrite(PIN_L9110_IB, LOW);
  appliedDuty = 0;
}

void startPwm(uint8_t pin, uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  if (ledcAttach(pin, PWM_FREQUENCY, PWM_RESOLUTION)) {
    ledcWrite(pin, duty);
    if (pin == PIN_L9110_IA) pwmAttachedIA = true;
    if (pin == PIN_L9110_IB) pwmAttachedIB = true;
  }
#else
  uint8_t channel = (pin == PIN_L9110_IA) ? PWM_CHANNEL_IA : PWM_CHANNEL_IB;
  ledcAttachPin(pin, channel);
  ledcWrite(channel, duty);
  if (pin == PIN_L9110_IA) pwmAttachedIA = true;
  if (pin == PIN_L9110_IB) pwmAttachedIB = true;
#endif
}

void applyMotor() {
  if (!motorEnabled) {
    stopMotor();
    return;
  }

  stopMotor();
  appliedDuty = logicalToPwm(logicalSpeed);

  if (motorForward) {
    digitalWrite(PIN_L9110_IB, LOW);
    startPwm(PIN_L9110_IA, appliedDuty);
  } else {
    digitalWrite(PIN_L9110_IA, LOW);
    startPwm(PIN_L9110_IB, appliedDuty);
  }
}

bool buttonPressed(uint8_t index) {
  bool reading = digitalRead(buttonPins[index]);

  if (reading != buttonPrevious[index]) {
    buttonPrevious[index] = reading;
    buttonChangedAt[index] = millis();
  }

  if (millis() - buttonChangedAt[index] >= DEBOUNCE_MS &&
      reading != buttonStable[index]) {
    buttonStable[index] = reading;
    return reading == LOW;
  }

  return false;
}

void changeSpeed(int amount) {
  int value = constrain(static_cast<int>(logicalSpeed) + amount, 0, 255);
  logicalSpeed = static_cast<uint8_t>(value);
  if (motorEnabled) applyMotor();
}

void reverseDirection() {
  if (motorEnabled) {
    stopMotor();
    delay(DIRECTION_WAIT_MS);
  }
  motorForward = !motorForward;
  if (motorEnabled) applyMotor();
}

void drawDisplay() {
  int settingPercent = map(logicalSpeed, 0, 255, 0, 100);
  int physicalPercent = motorEnabled ? map(appliedDuty, 0, 255, 0, 100) : 0;

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 10, "KUMITATE-C3 L9110");
  oled.drawHLine(0, 13, 128);

  oled.setCursor(0, 27);
  oled.print("POWER: ");
  oled.print(motorEnabled ? "ON" : "OFF");

  oled.setCursor(70, 27);
  oled.print(motorForward ? "FWD" : "REV");

  oled.setCursor(0, 41);
  oled.print("SPEED: ");
  oled.print(settingPercent);
  oled.print("%");

  oled.setCursor(0, 54);
  oled.print("PWM:   ");
  oled.print(physicalPercent);
  oled.print("%");

  oled.drawFrame(0, 56, 128, 8);
  int barWidth = motorEnabled ? map(logicalSpeed, 0, 255, 0, 124) : 0;
  if (barWidth > 0) oled.drawBox(2, 58, barWidth, 4);

  oled.sendBuffer();
}

void sendBle(const String &message) {
  if (!bleConnected || bleTxCharacteristic == nullptr) return;
  bleTxCharacteristic->setValue(message.c_str());
  bleTxCharacteristic->notify();
}

void sendStatus() {
  int speedPercent = map(logicalSpeed, 0, 255, 0, 100);
  int pwmPercent = motorEnabled ? map(appliedDuty, 0, 255, 0, 100) : 0;
  sendBle(String(motorEnabled ? "ON " : "OFF ") +
          (motorForward ? "FWD" : "REV"));
  delay(15);
  sendBle("S=" + String(speedPercent) + "% PWM=" + String(pwmPercent) + "%");
}

bool parsePercent(const String &text, int &percent) {
  if (text.length() == 0) return false;
  for (size_t i = 0; i < text.length(); ++i) {
    if (!isDigit(static_cast<unsigned char>(text[i]))) return false;
  }
  percent = text.toInt();
  return percent >= 0 && percent <= 100;
}

void executeBleCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command == "ON") {
    motorEnabled = true;
    applyMotor();
    sendBle("OK ON");
  } else if (command == "OFF") {
    motorEnabled = false;
    applyMotor();
    sendBle("OK OFF");
  } else if (command == "TOGGLE") {
    motorEnabled = !motorEnabled;
    applyMotor();
    sendBle(motorEnabled ? "OK ON" : "OK OFF");
  } else if (command == "FWD") {
    if (!motorForward && motorEnabled) {
      stopMotor();
      delay(DIRECTION_WAIT_MS);
    }
    motorForward = true;
    if (motorEnabled) applyMotor();
    sendBle("OK FWD");
  } else if (command == "REV") {
    if (motorForward && motorEnabled) {
      stopMotor();
      delay(DIRECTION_WAIT_MS);
    }
    motorForward = false;
    if (motorEnabled) applyMotor();
    sendBle("OK REV");
  } else if (command == "DIR") {
    reverseDirection();
    sendBle(motorForward ? "OK FWD" : "OK REV");
  } else if (command == "UP") {
    changeSpeed(SPEED_STEP);
    sendBle("OK UP");
  } else if (command == "DOWN") {
    changeSpeed(-SPEED_STEP);
    sendBle("OK DOWN");
  } else if (command.startsWith("SPEED ")) {
    int percent = 0;
    if (parsePercent(command.substring(6), percent)) {
      logicalSpeed = static_cast<uint8_t>(map(percent, 0, 100, 0, 255));
      if (motorEnabled) applyMotor();
      sendBle("OK SPEED " + String(percent));
    } else {
      sendBle("ERR SPEED 0-100");
    }
  } else if (command == "STATUS") {
    sendStatus();
  } else if (command == "HELP") {
    sendBle("ON OFF TOGGLE");
    delay(15);
    sendBle("FWD REV DIR");
    delay(15);
    sendBle("SPEED 0-100");
    delay(15);
    sendBle("UP DOWN STATUS");
  } else if (command.length() > 0) {
    sendBle("ERR TYPE HELP");
  }

  drawDisplay();
}

class MotorServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    bleConnected = true;
  }

  void onDisconnect(BLEServer *server) override {
    bleConnected = false;
    restartAdvertising = true;
  }
};

class MotorRxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    String received = characteristic->getValue().c_str();
    received.trim();
    if (received.length() == 0 || bleCommandQueue == nullptr) return;

    BleCommand command = {};
    received.substring(0, sizeof(command.text) - 1).toCharArray(
      command.text,
      sizeof(command.text)
    );
    xQueueSend(bleCommandQueue, &command, 0);
  }
};

void initializeBle() {
  bleCommandQueue = xQueueCreate(8, sizeof(BleCommand));

  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MotorServerCallbacks());
  BLEService *service = server->createService(BLE_SERVICE_UUID);

  bleTxCharacteristic = service->createCharacteristic(
    BLE_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  bleTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *rxCharacteristic = service->createCharacteristic(
    BLE_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  rxCharacteristic->setCallbacks(new MotorRxCallbacks());

  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void setup() {
  pinMode(PIN_SW1, INPUT);
  pinMode(PIN_SW2, INPUT);
  pinMode(PIN_SW3, INPUT);
  pinMode(PIN_SW4, INPUT);
  pinMode(PIN_L9110_IA, OUTPUT);
  pinMode(PIN_L9110_IB, OUTPUT);

  initializePwm();
  stopMotor();

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.setI2CAddress(OLED_ADDRESS * 2);
  oled.begin();
  initializeBle();
  drawDisplay();
}

void loop() {
  bool changed = false;

  if (restartAdvertising) {
    restartAdvertising = false;
    delay(100);
    BLEDevice::startAdvertising();
  }

  BleCommand bleCommand;
  while (bleCommandQueue != nullptr &&
         xQueueReceive(bleCommandQueue, &bleCommand, 0) == pdTRUE) {
    executeBleCommand(String(bleCommand.text));
  }

  if (buttonPressed(BUTTON_POWER)) {
    motorEnabled = !motorEnabled;
    applyMotor();
    changed = true;
  }

  if (buttonPressed(BUTTON_SPEED_UP)) {
    changeSpeed(SPEED_STEP);
    changed = true;
  }

  if (buttonPressed(BUTTON_SPEED_DOWN)) {
    changeSpeed(-SPEED_STEP);
    changed = true;
  }

  if (buttonPressed(BUTTON_DIRECTION)) {
    reverseDirection();
    changed = true;
  }

  if (changed) drawDisplay();
  delay(1);
}
