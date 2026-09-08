# KUMITATE-C3 L9110ファン制御

KUMITATE-C3の内蔵スイッチとOLEDを使い、L9110ファンモジュールの回転、停止、速度、回転方向を制御するArduinoスケッチです。

## 主な機能

- SW1によるファンのON/OFF
- SW2、SW3によるPWM出力の増減
- SW4による回転方向の切り替え
- 内蔵OLEDへの動作状態、回転方向、設定値、出力値の表示
- スイッチのチャタリング対策
- 回転方向を変更する前の一時停止
- Arduino-ESP32 Core 2.xおよび3.xのLEDC APIに対応

## 使用機器

- KUMITATE-C3
  - ESP32-C3-WROOM-02搭載
  - 128×64 I2C OLED搭載
  - SW1～SW4搭載
- L9110ファンモジュール
  - 使用電圧：3.3Vまたは5V
  - 端子：VCC、GND、INA、INB
  - 75mmプロペラ付きタイプ
- USBケーブル
- ジャンパーワイヤー

使用したファンモジュール：

- [Amazon商品ページ](https://link.amazon/B09af8nnZ)
- [L9110データシート](https://cdn-shop.adafruit.com/product-files/4489/4489_datasheet-l9110.pdf)

## 配線

| L9110ファンモジュール | KUMITATE-C3 | 説明 |
|---|---|---|
| VCC | 3.3Vまたは5V | ファンモジュール電源 |
| GND | GND | KUMITATE-C3とGNDを共通化 |
| INA | IO4（J4ピン4） | L9110入力A／PWM |
| INB | IO5（J4ピン5） | L9110入力B／PWM |

> [!CAUTION]
> ファンモジュールは3.3Vでも使用できます。
> 5V使用時より回転数や風量が低くなったり、低いPWM設定では始動しにくくなったりする場合があります。
> また、回転中のプロペラには触れないでください。

## KUMITATE-C3の使用ピン

| 機能 | GPIO | 備考 |
|---|---:|---|
| OLED SDA | IO1 | I2Cデータ |
| OLED SCL | IO2 | I2Cクロック |
| SW1 | IO21 | 押下時LOW、ON/OFF |
| SW2 | IO3 | 押下時LOW、速度アップ |
| SW3 | IO9 | 押下時LOW、速度ダウン |
| SW4 | IO8 | 押下時LOW、方向切り替え |
| L9110 INA | IO4 | PWM出力 |
| L9110 INB | IO5 | PWM出力 |

## スイッチ操作

| スイッチ | 動作 |
|---|---|
| SW1 | ファンのON/OFFを切り替える |
| SW2 | PWM設定値を16増やす |
| SW3 | PWM設定値を16減らす |
| SW4 | 回転方向を切り替える |

設定値の範囲は0～255です。起動時の設定値は128、ファン出力はOFFです。

## OLED表示

OLEDには次の情報が表示されます。

- `POWER`：ファン出力のON/OFF
- `DIR`：`FWD`（正転）または`REV`（逆転）
- `SET`：設定したPWM値と割合
- `OUT`：実際に出力しているPWMの割合
- `OUTPUT DISABLED`：出力OFF
- `SPEED ZERO`：速度設定0
- `RUNNING`：回転中

## PWM仕様

| 項目 | 設定値 |
|---|---:|
| PWM周波数 | 20kHz |
| PWM分解能 | 8bit |
| PWM範囲 | 0～255 |
| 速度変更量 | 16 |
| 方向変更時の停止時間 | 150ms |

ファンモジュールや電源条件によっては、低いPWM値ではファンが始動しない場合があります。その場合は、スケッチ側で最低PWM値を設定してください。

## 必要なソフトウェア

- Arduino IDE
- Arduino-ESP32ボードパッケージ
- U8g2ライブラリ
- Serial Bluetooth Terminal（BLE対応版を操作する場合）

Arduino IDEのライブラリマネージャーで`U8g2`を検索し、インストールしてください。

## BLE対応版

BLE対応版では、KUMITATE-C3の内蔵スイッチに加えて、Androidアプリの[Serial Bluetooth Terminal](https://github.com/kai-morich/SimpleBluetoothTerminal)からファンを操作できます。

ESP32-C3はBluetooth ClassicのSPPを使用できないため、通信にはBluetooth Low Energy（BLE）とNordic UART Service（NUS）を使用します。

### BLE仕様

| 項目 | 設定 |
|---|---|
| BLEデバイス名 | `KUMITATE-C3-CAR` |
| Service UUID | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| RX Characteristic | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| TX Characteristic | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |

### Serial Bluetooth Terminalからの接続

1. KUMITATE-C3へBLE対応版のスケッチを書き込みます。
2. Android端末でBluetoothと位置情報を有効にします。
3. Serial Bluetooth Terminalを起動します。
4. メニューから`Devices`を開き、Bluetooth LEのスキャンを実行します。
5. 一覧に表示された`KUMITATE-C3-CAR`を選択します。
6. 接続後、ターミナルへコマンドを入力して送信します。

Androidのシステム設定画面で事前にペアリングする必要はありません。Serial Bluetooth Terminalから直接BLE接続します。

### BLEコマンド

コマンドは半角英数字で入力します。大文字・小文字は区別しません。

| コマンド | 動作 |
|---|---|
| `ON` | ファンをONにする |
| `OFF` | ファンを停止する |
| `TOGGLE` | ON/OFFを切り替える |
| `FWD` | 正転に設定する |
| `REV` | 逆転に設定する |
| `DIR` | 現在の回転方向を反転する |
| `SPEED 0`～`SPEED 100` | 速度を0～100%で指定する |
| `UP` | 速度を1段階上げる |
| `DOWN` | 速度を1段階下げる |
| `STATUS` | 現在のON/OFF、方向、速度、PWM出力を取得する |
| `HELP` | 使用できるコマンドを表示する |

使用例：

```text
ON
SPEED 50
FWD
STATUS
OFF
```

### 最低PWM出力補正

使用しているファンモジュールは、低いPWM出力では停止状態から始動できない場合があります。BLE対応版では、操作上の速度0～100%を実際のPWM 43～100%へ線形変換します。

| 操作上の速度 | 実際のPWM出力 |
|---:|---:|
| 0% | 約43%（110/255） |
| 50% | 約72% |
| 100% | 100%（255/255） |

`OFF`コマンドまたはSW1で停止した場合のみ、PWM出力は0になります。したがって、`ON`かつ`SPEED 0`は停止ではなく、最低出力での運転を意味します。

### 本体スイッチとの併用

BLE対応版でもKUMITATE-C3のSW1～SW4を使用できます。BLEコマンドと本体スイッチは同じ動作状態を変更し、結果は内蔵OLEDへ反映されます。

## 書き込み方法

1. Arduino IDEへESP32ボードパッケージをインストールします。
2. U8g2ライブラリをインストールします。
3. `001-L9110.ino`をArduino IDEで開きます。
4. KUMITATE-C3に対応するESP32C3 Dev ModuleボードとCOMポートを選択します。
5. 必要に応じて`USB CDC On Boot`を`Enabled`に設定します。
6. スケッチを書き込みます。
7. 電源投入時にプロペラが停止していることを確認してから操作します。

## ファイル構成

```text
001-L9110/
├── 001-L9110.ino
└── README.md
```

## 注意事項

- ファンの電源とKUMITATE-C3のGNDは必ず共通にしてください。
- INA、INBへ5V信号を入力しないでください。KUMITATE-C3のGPIOは3.3Vロジックです。
- 配線を変更するときは電源を切ってください。
- プロペラの周囲に指、配線、衣類などが入らないようにしてください。
- 方向を急に反転するとモーター、プロペラ、電源へ負担がかかります。このスケッチでは反転前に150ms停止します。
- 火を扱う実験には使用せず、送風実験用として安全な環境で使用してください。
