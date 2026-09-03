# KUMITATE-C3 距離連動モーター制御

KUMITATE-C3のGPIO拡張ピンに測距センサーとモータードライバを接続し、距離に応じてDCモーターの回転速度を変えるArduinoサンプルです。

測距センサーで取得した距離をPWM値へ変換し、物体が近づくほどモーターを速く回します。測定距離、モーターの動作状態、速度はKUMITATE-C3搭載OLEDとシリアルモニタにリアルタイム表示します。

## デモ動画

[距離を測定してモーター回転速度を変える - YouTube Shorts](https://youtube.com/shorts/43JAQDESIUQ)

## 主な機能

- HC-SR04互換測距センサーによる距離測定
- 距離に応じたPWMデューティ比の計算
- L9110モータードライバによるDCモーター制御
- 近づくほどモーターの回転速度を上げる制御
- 測定範囲外または測定失敗時のモーター停止
- SSD1306 OLEDへの日本語表示
- シリアルモニタへの距離、PWM値、速度の表示
- OLED、測距センサー、モーター制御を機能別ファイルに分割

## 動作

現在の初期設定では、次のように動作します。

- 30cmより遠い：モーター停止
- 5～30cm：近づくほど回転速度が上昇
- 5cm以下：最高速度
- 距離を測定できない：安全のためモーター停止

動作開始距離は、メインスケッチの次の値で変更できます。

```cpp
constexpr float MOTOR_START_DISTANCE_CM = 30.0f;
```

例えば50cm以内で動作させる場合は、`50.0f`へ変更します。

## 使用部品

- KUMITATE-C3
- 3.3Vで使用できるHC-SR04互換測距センサー
- L9110モータードライバ
- DCモーター
- モーター用電源
- ブレッドボードおよびジャンパーワイヤー

## ピン接続

### 測距センサー

| 測距センサー | KUMITATE-C3 |
|---|---:|
| TRIG | IO7 |
| ECHO | IO0 |
| VCC | センサー仕様に適合する電源 |
| GND | GND |

### L9110

| L9110 | KUMITATE-C3 |
|---|---:|
| INA | IO6 |
| INB | IO5 |
| GND | GND |

DCモーターはL9110のモーター出力端子へ接続します。KUMITATE-C3とL9110のGNDは共通にしてください。

### 内蔵OLED

| OLED | KUMITATE-C3 |
|---|---:|
| SDA | IO1 |
| SCL | IO2 |
| I2Cアドレス | `0x3C` |

OLEDはKUMITATE-C3に搭載されているSSD1306 128×64ディスプレイを使用します。

## 必要な開発環境

- Arduino IDE
- ESP32 Arduino Core
- U8g2ライブラリ

Arduino IDEのライブラリマネージャーで`U8g2`を検索し、インストールしてください。

## 書き込み方法

1. このリポジトリをダウンロードまたはクローンします。
2. `KUMITATE_C3_DISTANCE_MOTOR.ino`をArduino IDEで開きます。
3. Arduino IDEが同じフォルダ内の`.h`と`.cpp`を別タブとして読み込みます。
4. 使用するESP32-C3のボードとシリアルポートを選択します。
5. KUMITATE-C3へ書き込みます。
6. シリアルモニタを`115200 bps`で開きます。

## ファイル構成

```text
KUMITATE_C3_DISTANCE_MOTOR/
├── KUMITATE_C3_DISTANCE_MOTOR.ino  メイン処理と距離・速度の対応
├── Hcsr04Sensor.h                  測距センサー関数の宣言
├── Hcsr04Sensor.cpp                距離測定処理
├── L9110Motor.h                    モーター制御関数の宣言
├── L9110Motor.cpp                  L9110のPWM制御
├── OledDisplay.h                   OLED表示関数の宣言
├── OledDisplay.cpp                 日本語表示処理
└── README.md
```

各モジュールはクラスを使用せず、C言語に近い関数ベースの構成にしています。ただし、Arduino APIとU8g2はC++ライブラリのため、実装ファイルの拡張子は`.cpp`です。

## シリアルモニタの表示例

```text
Distance: 24.5 cm / PWM: 134 / Speed: 52%
Distance: 12.8 cm / PWM: 206 / Speed: 80%
Distance: 42.1 cm / PWM: 0 / Speed: 0%
```

## 注意事項

- HC-SR04は3.3V対応品を使用してください。
- DCモーターをGPIOや3.3V端子から直接駆動しないでください。
- モーターには仕様に合った別電源を使用し、KUMITATE-C3とGNDを共通にしてください。
- モーターのノイズで測距値が不安定になる場合は、モーター端子間へのノイズ対策用コンデンサ追加を検討してください。

## 関連リンク

- [KUMITATE-C3 製品ページ](https://kumicla.tatepro.com/asp-products/kumitateboard-c3/)
- [KUMITATE-C3 レッスン集](https://kumicla.tatepro.com/courses/kumitatec3/)
