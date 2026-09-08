# L9110ファン制御 BLE追加版

KUMITATE-C3のL9110ファン制御プログラムにBluetooth Low Energy（BLE）通信を追加し、AndroidアプリのSerial Bluetooth Terminalから操作するための説明です。

本体のSW1〜SW4とOLED表示は、BLE操作と併用できます。

## 使用アプリ

- [Serial Bluetooth Terminal](https://github.com/kai-morich/SimpleBluetoothTerminal)

ESP32-C3はBluetooth ClassicのSPPに対応していないため、Bluetooth Low EnergyとNordic UART Service（NUS）を使用します。

## BLE仕様

| 項目 | 設定 |
|---|---|
| BLEデバイス名 | `KUMITATE-C3-CAR` |
| Service UUID | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| RX Characteristic | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| TX Characteristic | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |

## 接続方法

1. KUMITATE-C3へBLE対応版のスケッチを書き込みます。
2. Android端末でBluetoothと位置情報を有効にします。
3. Serial Bluetooth Terminalを起動します。
4. メニューから`Devices`を開き、Bluetooth LEのスキャンを実行します。
5. `KUMITATE-C3-CAR`を選択して接続します。
6. ターミナルからコマンドを送信します。

Androidのシステム設定で事前にペアリングする必要はありません。Serial Bluetooth Terminalから直接BLE接続します。

## コマンド一覧

コマンドは半角英数字で入力します。大文字・小文字は区別しません。

| コマンド | 動作 |
|---|---|
| `ON` | ファンをONにする |
| `OFF` | ファンを停止する |
| `TOGGLE` | ON/OFFを切り替える |
| `FWD` | 正転に設定する |
| `REV` | 逆転に設定する |
| `DIR` | 回転方向を反転する |
| `SPEED 0`〜`SPEED 100` | 速度を0〜100%で指定する |
| `UP` | 速度を1段階上げる |
| `DOWN` | 速度を1段階下げる |
| `STATUS` | 現在の状態を取得する |
| `HELP` | 使用できるコマンドを表示する |

## 操作例

ファンをONにして速度を50%に設定する例です。

```text
ON
SPEED 50
```

正転に設定して現在の状態を確認する例です。

```text
FWD
STATUS
```

ファンを停止します。

```text
OFF
```

## 最低PWM出力補正

使用しているL9110ファンモジュールは、低いPWM出力では停止状態から始動できません。そのため、操作上の速度0〜100%を実際のPWM 43〜100%へ線形変換しています。

| 操作上の速度 | 実際のPWM出力 |
|---:|---:|
| 0% | 約43%（110/255） |
| 50% | 約72% |
| 100% | 100%（255/255） |

`OFF`コマンドまたはSW1で停止した場合のみ、PWM出力は0になります。

したがって、`ON`かつ`SPEED 0`は停止ではなく、最低出力での運転を意味します。

## 本体スイッチとの併用

| スイッチ | 動作 |
|---|---|
| SW1 | ON/OFF |
| SW2 | 速度アップ |
| SW3 | 速度ダウン |
| SW4 | 回転方向の切り替え |

BLEコマンドと本体スイッチは同じ動作状態を変更し、結果は内蔵OLEDへ反映されます。

## 必要なライブラリ

- U8g2
- Arduino-ESP32に含まれるBLEライブラリ

BLE関連では、次のヘッダーを使用します。

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
```

## 注意事項

- BLE接続が切れた場合でもファンは直前の状態を維持します。必要に応じて`OFF`にしてから切断してください。
- 緊急停止には本体のSW1を使用できます。
- ファンの回転中はプロペラに触れないでください。
- ファンモジュールのGNDとKUMITATE-C3のGNDは共通にしてください。
