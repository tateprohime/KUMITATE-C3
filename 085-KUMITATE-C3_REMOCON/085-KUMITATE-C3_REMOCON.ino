//フルバージョン
#if 1

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include <string.h>

#define NO_LED_FEEDBACK_CODE
#define MAX_RAW_LENGTH  400
#include <IRremote.hpp>

// ======================================================
// KUMITATE-C3 PIN
// ======================================================

#define OLED_SDA        1
#define OLED_SCL        2

#define SW1_PIN        21
#define SW2_PIN         3
#define SW3_PIN         9
#define SW4_PIN         8

#define IR_SEND_PIN      6
#define IR_RECEIVE_PIN   7

// ======================================================
// OLED
// ======================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// ======================================================
// IR SLOT
// ======================================================
#define SLOT_COUNT       20

struct IRSlot{
  bool used;
  uint16_t length;
  uint16_t raw[MAX_RAW_LENGTH];
  uint64_t rawData;
  uint16_t bits;
  uint16_t address;
  uint16_t command;
};

IRSlot slots[SLOT_COUNT];
IRSlot receivedData;

const size_t SLOT_NAME_MAX = 4;
char slotNames[SLOT_COUNT][SLOT_NAME_MAX + 1] = {};

int selectedSlot = 0;


// ======================================================
// SCREEN STATE
// ======================================================
enum ScreenState{
  SCREEN_MAIN,
  SCREEN_RECEIVE_WAIT,
  SCREEN_RECEIVE_SELECT,
  SCREEN_SEND_SELECT
};
ScreenState screen = SCREEN_MAIN;


// ======================================================
// BUTTON
// ======================================================
struct Button{
  uint8_t pin;
  bool lastState;
  unsigned long lastTime;
};

Button sw1 = {SW1_PIN, HIGH, 0};
Button sw2 = {SW2_PIN, HIGH, 0};
Button sw3 = {SW3_PIN, HIGH, 0};
Button sw4 = {SW4_PIN, HIGH, 0};


// ======================================================
// BUTTON PRESS DETECT
// ======================================================

bool buttonPressed(Button &button){
  bool now = digitalRead(button.pin);
  bool pressed = false;
  if (button.lastState == HIGH && now == LOW){
    if (millis() - button.lastTime > 50){
      pressed = true;
      button.lastTime = millis();
    }
  }
  button.lastState = now;
  return pressed;
}

// ======================================================
// OLED MAIN
// ======================================================
void showMain(){
  oled.clearBuffer();
  oled.setFont(u8g2_font_helvB12_tf);
  oled.drawStr(0, 16, "IR REMOTE");
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 38, "SW1 : RECEIVE");
  oled.drawStr(0, 55, "SW4 : SEND");
  oled.sendBuffer();
}

// ======================================================
// RECEIVE WAIT
// ======================================================
void showReceiveWait(){
  oled.clearBuffer();
  oled.setFont(u8g2_font_helvB12_tf);
  oled.drawStr(0, 16, "RECEIVE");
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 38, "Waiting IR...");
  oled.drawStr(0, 58, "SW1 : Back");
  oled.sendBuffer();
}

// ======================================================
// RECEIVE SELECT
// ======================================================
void showReceiveSelect(){
  char buf[32];
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 10, "IR RECEIVED");
  snprintf(buf, sizeof(buf), "RAW:%012llX", (unsigned long long)receivedData.rawData);
  oled.drawStr(0, 22, buf);
  snprintf(buf, sizeof(buf), "Bits:%d Cmd:%02X", receivedData.bits, receivedData.command);
  oled.drawStr(0, 34, buf);
  snprintf(buf, sizeof(buf), "<%02d> %s", selectedSlot + 1, slotNames[selectedSlot]);

  oled.setFont(u8g2_font_helvB10_tf);
  oled.drawStr(0, 50, buf);

  oled.setFont(u8g2_font_5x8_tf);
  oled.drawStr(0, 63, "SW3:- SW4:+ SW2:Save");

  oled.sendBuffer();
}

// ======================================================
// SEND SELECT
// ======================================================
void showSendSelect(){
  char buf[32];
  oled.clearBuffer();
  oled.setFont(u8g2_font_helvB10_tf);
  oled.drawStr(0, 14, "IR SEND");
  snprintf(buf, sizeof(buf), "<%02d> %s", selectedSlot + 1, slotNames[selectedSlot]);
  oled.drawStr(0, 31, buf);
  oled.setFont(u8g2_font_6x12_tf);

  if (slots[selectedSlot].used){
    snprintf(buf, sizeof(buf), "RAW:%012llX", (unsigned long long) slots[selectedSlot].rawData);

    oled.drawStr(0, 46, buf);
  } else {
    oled.drawStr(0, 46, "--- EMPTY ---");
  }

  oled.setFont(u8g2_font_5x8_tf);
  oled.drawStr(0, 63, "SW3:- SW4:+ SW2:Send");
  oled.sendBuffer();
}


// ======================================================
// MESSAGE
// ======================================================
void showMessage(const char *msg){
  oled.clearBuffer();
  oled.setFont(u8g2_font_helvB12_tf);
  oled.drawStr(0, 32, msg);
  oled.sendBuffer();
}

// ======================================================
// COPY RECEIVED IR
// ======================================================
bool copyReceivedIR(){
  uint16_t rawLength = IrReceiver.irparams.rawlen;
  if (rawLength <= 1){
    return false;
  }

  uint16_t copyLength = rawLength - 1;

  if (copyLength > MAX_RAW_LENGTH){
    copyLength = MAX_RAW_LENGTH;
  }

  receivedData.used = true;
  receivedData.length = copyLength;

  // rawbuf[0] はGap
  for (uint16_t i = 0; i < copyLength; i++){
    uint32_t us = (uint32_t)IrReceiver.irparams.rawbuf[i + 1] * MICROS_PER_TICK;

    if (us > 65535){
      us = 65535;
    }
    receivedData.raw[i] = (uint16_t)us;}

  receivedData.rawData = IrReceiver.decodedIRData.decodedRawData;
  receivedData.bits = IrReceiver.decodedIRData.numberOfBits;
  receivedData.address = IrReceiver.decodedIRData.address;
  receivedData.command = IrReceiver.decodedIRData.command;

  Serial.println();
  Serial.println("IR RECEIVED");

  IrReceiver.printIRResultShort(&Serial);

  Serial.print("RAW length = ");
  Serial.println(receivedData.length);

  Serial.print("RAW data = 0x");
  Serial.println((unsigned long long) receivedData.rawData, HEX);

  return true;
}

// ======================================================
// SLOT SAVE
// ======================================================
Preferences slotPreferences;
bool nvsReady = false;

// Version 1: uint16_t words, independent of IRSlot struct padding.
// magic, version, length, rawData(4 words), bits, address, command, raw timings.
const size_t NVS_HEADER_WORDS = 10;

void loadSlots(){
  for (int i = 0; i < SLOT_COUNT; i++) slots[i] = {};
  nvsReady = slotPreferences.begin("ir-remote", false);
  if (!nvsReady){
    Serial.println("NVS OPEN FAILED");
    return;
  }
  int loaded = 0;
  for (int i = 0; i < SLOT_COUNT; i++){
    char key[12];
    snprintf(key, sizeof(key), "slot%02d", i);
    if (!slotPreferences.isKey(key)) continue;
    uint16_t data[NVS_HEADER_WORDS + MAX_RAW_LENGTH] = {};
    size_t bytes = slotPreferences.getBytesLength(key);
    if (bytes < (NVS_HEADER_WORDS + 1) * sizeof(uint16_t) ||
        bytes > sizeof(data) || bytes % sizeof(uint16_t) != 0){
      Serial.printf("NVS INVALID SLOT %d\n", i + 1);
      continue;
    }
    if (slotPreferences.getBytes(key, data, bytes) != bytes ||
        data[0] != 0x4952 || data[1] != 1 ||
        data[2] == 0 || data[2] > MAX_RAW_LENGTH ||
        bytes != (NVS_HEADER_WORDS + data[2]) * sizeof(uint16_t)){
      Serial.printf("NVS INVALID SLOT %d\n", i + 1);
      continue;
    }
    slots[i].length = data[2];
    for (int j = 0; j < 4; j++){
      slots[i].rawData |= (uint64_t)data[3 + j] << (16 * j);
    }
    slots[i].bits = data[7];
    slots[i].address = data[8];
    slots[i].command = data[9];
    for (uint16_t j = 0; j < slots[i].length; j++){
      slots[i].raw[j] = data[NVS_HEADER_WORDS + j];
    }
    slots[i].used = true;
    loaded++;
  }
  Serial.printf("NVS LOADED %d SLOTS\n", loaded);
}

bool saveSlot(int slot){
  if (!nvsReady || slot < 0 || slot >= SLOT_COUNT ||
      !receivedData.used || receivedData.length == 0 ||
      receivedData.length > MAX_RAW_LENGTH){
    Serial.println("NVS SAVE FAILED: storage unavailable or invalid data");
    return false;
  }
  uint16_t data[NVS_HEADER_WORDS + MAX_RAW_LENGTH] = {};
  data[0] = 0x4952;
  data[1] = 1;
  data[2] = receivedData.length;
  for (int i = 0; i < 4; i++){
    data[3 + i] = (uint16_t)(receivedData.rawData >> (16 * i));
  }
  data[7] = receivedData.bits;
  data[8] = receivedData.address;
  data[9] = receivedData.command;
  for (uint16_t i = 0; i < receivedData.length; i++){
    data[NVS_HEADER_WORDS + i] = receivedData.raw[i];
  }
  char key[12];
  snprintf(key, sizeof(key), "slot%02d", slot);
  size_t bytes = (NVS_HEADER_WORDS + receivedData.length) * sizeof(uint16_t);
  if (slotPreferences.putBytes(key, data, bytes) != bytes){
    Serial.printf("NVS SAVE FAILED SLOT %d\n", slot + 1);
    return false;
  }
  // Keep the old RAM slot if persistence failed.
  slots[slot] = receivedData;
  Serial.printf("NVS SAVED SLOT %d\n", slot + 1);
  return true;
}

// ======================================================
// SLOT SEND
// ======================================================
void sendSlot(int slot){
  if (!slots[slot].used){return;}

  Serial.print("SEND SLOT ");
  Serial.println(slot + 1);

  IrSender.sendRaw(slots[slot].raw, slots[slot].length, 38);
}

// ======================================================
// SLOT -1
// ======================================================
void slotPrevious(){
  selectedSlot--;

  if (selectedSlot < 0){
    selectedSlot = SLOT_COUNT - 1;
  }
}

// ======================================================
// SLOT +1
// ======================================================
void slotNext(){
  selectedSlot++;

  if (selectedSlot >= SLOT_COUNT){
    selectedSlot = 0;
  }
}

// Names use separate NVS keys so renaming never overwrites IR data.
bool validSlotName(const char *name){
  size_t length = strlen(name);
  if (length == 0 || length > SLOT_NAME_MAX) return false;
  bool hasVisibleCharacter = false;
  for (size_t i = 0; i < length; i++){
    uint8_t c = (uint8_t)name[i];
    if (c < 0x20 || c > 0x7E) return false;
    if (c != ' ') hasVisibleCharacter = true;
  }
  return hasVisibleCharacter;
}

void loadSlotNames(){
  if (!nvsReady) return;
  for (int i = 0; i < SLOT_COUNT; i++){
    char key[12];
    snprintf(key, sizeof(key), "name%02d", i);
    if (!slotPreferences.isKey(key)) continue;
    char name[SLOT_NAME_MAX + 1] = {};
    if (slotPreferences.getString(key, name, sizeof(name)) > 0 && validSlotName(name)){
      memcpy(slotNames[i], name, sizeof(name));
    }
  }
}

void handleNameCommand(const char *line){
  size_t length = strlen(line);
  if (length < 4 || length > 3 + SLOT_NAME_MAX ||
      line[0] < '0' || line[0] > '9' ||
      line[1] < '0' || line[1] > '9' || line[2] != ' '){
    Serial.println("ERROR: use 01 NAME (01-20, 1-4 ASCII characters)");
    return;
  }
  int slot = (line[0] - '0') * 10 + (line[1] - '0') - 1;
  const char *name = line + 3;
  if (slot < 0 || slot >= SLOT_COUNT || !validSlotName(name)){
    Serial.println("ERROR: slot must be 01-20; name must be 1-4 ASCII characters");
    return;
  }
  if (!nvsReady){
    Serial.println("ERROR: NVS unavailable; name unchanged");
    return;
  }
  char key[12];
  snprintf(key, sizeof(key), "name%02d", slot);
  if (strcmp(slotNames[slot], name) != 0){
    if (slotPreferences.putString(key, name) != strlen(name)){
      Serial.println("ERROR: name save failed; name unchanged");
      return;
    }
    strcpy(slotNames[slot], name);
  }
  Serial.printf("NAME SAVED %02d %s\n", slot + 1, slotNames[slot]);
  if (slot == selectedSlot){
    if (screen == SCREEN_RECEIVE_SELECT) showReceiveSelect();
    else if (screen == SCREEN_SEND_SELECT) showSendSelect();
  }
}

void readSerialCommands(){
  static char line[3 + SLOT_NAME_MAX + 1] = {};
  static size_t used = 0;
  static bool invalid = false;
  // Bounded, non-blocking input so switches and IR reception keep running.
  for (int budget = 0; budget < 32 && Serial.available() > 0; budget++){
    int c = Serial.read();
    if (c < 0) break;
    if (c == '\r' || c == '\n'){
      if (invalid){
        Serial.println("ERROR: use 01 NAME (maximum 4 ASCII characters)");
      } else if (used > 0){
        line[used] = '\0';
        handleNameCommand(line);
      }
      used = 0;
      invalid = false;
    } else if (!invalid){
      if (c < 0x20 || c > 0x7E || used >= sizeof(line) - 1){
        invalid = true; // Discard the entire invalid line, not just its suffix.
      } else {
        line[used++] = (char)c;
      }
    }
  }
}

// ======================================================
// SETUP
// ======================================================
void setup(){
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("KUMITATE-C3 IR Remote");

  // ----------------------------
  // SWITCH
  // ----------------------------
  pinMode(SW1_PIN, INPUT);
  pinMode(SW2_PIN, INPUT);
  pinMode(SW3_PIN, INPUT);
  pinMode(SW4_PIN, INPUT);

  // ----------------------------
  // OLED
  // ----------------------------
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();

  // ----------------------------
  // IR
  // ----------------------------
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN);

  // ----------------------------
  // SLOT INIT
  // ----------------------------

  loadSlots();
  loadSlotNames();
  Serial.println("Rename: 01 NAME (01-20, 1-4 ASCII characters; newline required)");

  showMain();
}


// ======================================================
// LOOP
// ======================================================
void loop(){
  readSerialCommands();

  // ==================================================
  // READ BUTTON
  // ==================================================
  bool pressSW1 = buttonPressed(sw1);
  bool pressSW2 = buttonPressed(sw2);
  bool pressSW3 = buttonPressed(sw3);
  bool pressSW4 = buttonPressed(sw4);

  // ==================================================
  // MAIN
  // ==================================================

  if (screen == SCREEN_MAIN){
    // SW1 -> RECEIVE
    if (pressSW1){
      selectedSlot = 0;
      screen = SCREEN_RECEIVE_WAIT;
      showReceiveWait();
      return;
    }


    // SW4 -> SEND
    if (pressSW4){
      selectedSlot = 0;
      screen = SCREEN_SEND_SELECT;
      showSendSelect();
      return;
    }
  }

  // ==================================================
  // RECEIVE WAIT
  // ==================================================
  else if (screen == SCREEN_RECEIVE_WAIT){
    // SW1 -> BACK
    if (pressSW1) {
      screen = SCREEN_MAIN;
      showMain();
      return;
    }

    // IR RECEIVE
    if (IrReceiver.decode()){
      if (copyReceivedIR()){
        selectedSlot = 0;
        screen = SCREEN_RECEIVE_SELECT;
        showReceiveSelect();
      }
      IrReceiver.resume();
      return;
    }
  }

  // ==================================================
  // RECEIVE SELECT
  // ==================================================

  else if (screen == SCREEN_RECEIVE_SELECT){
    // SW1 -> BACK
    if (pressSW1){
      screen = SCREEN_MAIN;
      showMain();
      return;
    }

    // SW3 -> -1
    if (pressSW3){
      slotPrevious();
      showReceiveSelect();
      return;
    }

    // SW4 -> +1
    if (pressSW4){
      slotNext();
      showReceiveSelect();
      return;
    }

    // SW2 -> SAVE
    if (pressSW2){
      if (!saveSlot(selectedSlot)){
        showMessage("SAVE FAILED");
        delay(1000);
        showReceiveSelect();
        return;
      }
      showMessage("SAVED");
      delay(500);
      screen = SCREEN_RECEIVE_WAIT;
      showReceiveWait();
      return;
    }
  }


  // ==================================================
  // SEND SELECT
  // ==================================================

  else if (screen == SCREEN_SEND_SELECT){
    // SW1 -> BACK
    if (pressSW1){
      screen = SCREEN_MAIN;
      showMain();
      return;
    }

    // SW3 -> -1
    if (pressSW3){
      slotPrevious();
      showSendSelect();
      return;
    }

    // SW4 -> +1
    if (pressSW4){
      slotNext();
      showSendSelect();
      return;
    }

    // SW2 -> SEND
    if (pressSW2){
      if (slots[selectedSlot].used){
        showMessage("SENDING");
        sendSlot(selectedSlot);
        delay(300);
      } else {
        showMessage("EMPTY");
        delay(500);
      }
      showSendSelect();
      return;
    }
  }
}







#elif 0

#include <Arduino.h>
#include <IRremote.hpp>
#include <U8g2lib.h>
#include <Wire.h>

// -----------------------------
// KUMITATE-C3 ピンアサイン
// -----------------------------
#define OLED_SDA    1
#define OLED_SCL    2

#define SW1_PIN    21
#define SW2_PIN     3
#define SW3_PIN     9
#define SW4_PIN     8

#define IR_SEND_PIN 6

// -----------------------------
// OLED
// -----------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// -----------------------------
// 赤外線データ
// 受信済みの値を登録
// -----------------------------
uint64_t irData[4] = {
  0x324E128F5AAAULL, // SW1
  0x324E128F5AAAULL, // SW2 仮
  0x324E128F5AAAULL, // SW3 仮
  0x324E128F5AAAULL  // SW4 仮
};

const char* buttonName[4] = {
  "SW1",
  "SW2",
  "SW3",
  "SW4"
};

const uint8_t buttonPin[4] = {
  SW1_PIN,
  SW2_PIN,
  SW3_PIN,
  SW4_PIN
};


// -----------------------------
// 赤外線送信
// -----------------------------
void sendIR(uint64_t data)
{
  IrSender.sendPulseDistanceWidth(
    38,                 // 38kHz
    3400, 1650,         // Header
    400, 1300,          // bit 1
    400, 450,           // bit 0
    data,
    48,
    PROTOCOL_IS_LSB_FIRST,
    0,
    NO_REPEATS
  );
}


// -----------------------------
// OLED表示
// -----------------------------
void showSend(int index)
{
  char buf[32];

  snprintf(
    buf,
    sizeof(buf),
    "%012llX",
    (unsigned long long)irData[index]
  );

  oled.clearBuffer();

  oled.setFont(u8g2_font_helvB14_tf);
  oled.drawStr(0, 18, "IR SEND");

  oled.setFont(u8g2_font_helvB12_tf);
  oled.drawStr(0, 40, buttonName[index]);

  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 60, buf);

  oled.sendBuffer();
}


// -----------------------------
// 初期画面
// -----------------------------
void showReady()
{
  oled.clearBuffer();

  oled.setFont(u8g2_font_helvB14_tf);
  oled.drawStr(0, 22, "IR Remote");

  oled.setFont(u8g2_font_6x12_tf);
  oled.drawStr(0, 45, "Ready");

  oled.sendBuffer();
}


// -----------------------------
// setup
// -----------------------------
void setup()
{
  Serial.begin(115200);
  delay(1000);

  // スイッチ
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();

  // IR送信
  IrSender.begin(IR_SEND_PIN);

  showReady();

  Serial.println("KUMITATE-C3 IR Remote Start");
}


// -----------------------------
// loop
// -----------------------------
void loop()
{
  for (int i = 0; i < 4; i++) {

    if (digitalRead(buttonPin[i]) == LOW) {

      Serial.print("Send ");
      Serial.print(buttonName[i]);
      Serial.print(" : 0x");
      Serial.println(
        (unsigned long long)irData[i],
        HEX
      );

      // OLED表示
      showSend(i);

      // 赤外線送信
      sendIR(irData[i]);

      // チャタリング対策
      delay(50);

      // 離されるまで待つ
      while (digitalRead(buttonPin[i]) == LOW) {
        delay(10);
      }

      delay(50);
    }
  }
}

#elif 0
#include <IRremote.hpp>

#define IR_SEND_PIN 6

void setup() {
  Serial.begin(115200);
  delay(1000);

  IrSender.begin(IR_SEND_PIN);

  Serial.println("IR Send Start");
}

void loop() {

  Serial.println("SEND");

  IrSender.sendPulseDistanceWidth(
    38,                 // 38kHz
    3400, 1650,         // header mark / space
    400, 1300,          // bit 1 mark / space
    400, 450,           // bit 0 mark / space
    0x324E128F5AAAULL,  // 受信した48bit
    48,                 // 48bit
    PROTOCOL_IS_LSB_FIRST,
    0,
    NO_REPEATS
  );

  delay(2000);
}


#else


#include <IRremote.hpp>

#define IR_RECEIVE_PIN 7

void setup() {
  Serial.begin(115200);
  delay(1000);

  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  Serial.println("IR Receiver Start");
}

void loop() {
  if (IrReceiver.decode()) {

    Serial.println("--------------------");

    IrReceiver.printIRResultShort(&Serial);

    Serial.print("flags = 0x");
    Serial.println(IrReceiver.decodedIRData.flags, HEX);

    Serial.print("address = 0x");
    Serial.println(IrReceiver.decodedIRData.address, HEX);

    Serial.print("command = 0x");
    Serial.println(IrReceiver.decodedIRData.command, HEX);

    Serial.print("raw = 0x");
    Serial.println(
      (uint64_t)IrReceiver.decodedIRData.decodedRawData,
      HEX
    );

    // 実際のMARK/SPACEタイミングも表示
    IrReceiver.printIRResultRawFormatted(&Serial, true);

    Serial.println();

    IrReceiver.resume();
  }
}
#endif
