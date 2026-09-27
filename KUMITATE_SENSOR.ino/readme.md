# KUMITATE-SENSOR Sample

KUMITATE-SENSORの動作確認用Arduinoサンプルプログラムです。

KUMITATE-SENSORは、温度・湿度・照度・3軸加速度を測定できる
KUMITATE-C3用のI²Cセンサーボードです。

このサンプルでは、KUMITATE-SENSORに搭載された3種類のセンサーから値を取得し、
シリアルモニターへ表示します。

## KUMITATE-SENSOR

KUMITATE-SENSORには、以下のセンサーを搭載しています。

| センサー | 測定内容 | I²Cアドレス |
|---|---|---|
| SHT40-A | 温度・湿度 | `0x44` |
| BH1750 | 照度 | `0x23` |
| MSA3S02 | 3軸加速度 | `0x62` |

1本のI²Cバスで3種類のセンサーを使用できます。

## このサンプルでできること

- I²Cデバイスのスキャン
- SHT40-Aの温度測定
- SHT40-Aの湿度測定
- BH1750の照度測定
- MSA3S02のX軸加速度取得
- MSA3S02のY軸加速度取得
- MSA3S02のZ軸加速度取得

## 使用環境

- KUMITATE-C3
- KUMITATE-SENSOR
- Arduino IDE または PlatformIO
- シリアルモニター：115200 bps

## 接続

KUMITATE-SENSORとKUMITATE-C3をI²Cで接続します。

| 信号 | 説明 |
|---|---|
| 3V3 | 3.3V電源 |
| GND | GND |
| SDA | I²Cデータ |
| SCL | I²Cクロック |

使用するSDA / SCLのGPIO番号は、プログラム内の以下の部分で設定します。

```cpp
#define SDA_PIN 8
#define SCL_PIN 9
```

使用するKUMITATE-C3の配線に合わせて変更してください。

## I²Cアドレス

プログラムでは以下のアドレスを使用しています。

```cpp
#define BH1750_ADDR   0x23
#define MSA3S02_ADDR  0x62
#define SHT40_ADDR    0x44
```

起動時にはI²Cスキャンを実行します。

正常に認識されている場合は、シリアルモニターに以下のように表示されます。

```text
===== I2C Scan =====
Found : 0x23
Found : 0x44
Found : 0x62
====================

Device check
BH1750  [0x23] : OK
MSA3S02 [0x62] : OK
SHT40-A [0x44] : OK
```

3つのアドレスが表示されれば、各センサーとのI²C通信が確認できています。

## 測定結果

センサーの測定値は約1秒ごとにシリアルモニターへ表示されます。

表示例：

```text
---------- SENSOR ----------
Temperature : 24.81 degC
Humidity    : 48.62 %
Light       : 315.8 lx
Accel X     : 23
Accel Y     : -41
Accel Z     : 1018
----------------------------
```

### Temperature

SHT40-Aで測定した温度です。

単位は `degC` です。

### Humidity

SHT40-Aで測定した相対湿度です。

単位は `%` です。

### Light

BH1750で測定した照度です。

単位は `lx`（ルクス）です。

### Accel X / Y / Z

MSA3S02から取得した3軸加速度のRAW値です。

このサンプルでは、X/Y/Zの値をそのまま表示しています。

基板を傾けたり裏返したりすると、それぞれの軸の値が変化します。

## MSA3S02

MSA3S02は3軸加速度センサーです。

このサンプルでは±2gレンジで使用します。

加速度データは以下のレジスタから取得しています。

| 軸 | レジスタ |
|---|---|
| X | `0x02` / `0x03` |
| Y | `0x04` / `0x05` |
| Z | `0x06` / `0x07` |

取得した12bitの2の補数データを符号付き整数へ変換して表示します。

## BH1750

BH1750はデジタル照度センサーです。

このサンプルではContinuous High Resolution Modeを使用しています。

```cpp
Wire.write(0x10);
```

取得したRAWデータから照度（lx）へ変換して表示します。

## SHT40-A

SHT40-Aは温度・湿度センサーです。

このサンプルではHigh Precision Measurementを使用しています。

```cpp
Wire.write(0xFD);
```

取得したRAWデータから温度と相対湿度へ変換しています。

## 動作確認方法

1. KUMITATE-C3とKUMITATE-SENSORを接続します。
2. サンプルプログラムを書き込みます。
3. シリアルモニターを115200 bpsで開きます。
4. `0x23`、`0x44`、`0x62`が検出されることを確認します。
5. 温度・湿度・照度が表示されることを確認します。
6. KUMITATE-SENSORを傾け、X/Y/Zの値が変化することを確認します。

これでKUMITATE-SENSORに搭載された3種類のセンサーをまとめて動作確認できます。

## KUMITATE

KUMITATEは、組み込み開発・電子工作を実機を使って学ぶための学習環境です。

KUMITATE-C3を中心に、センサーやモーターなどの拡張ボードと組み合わせながら、
GPIO、I²C、SPI、UART、Wi-Fi、無線通信などを段階的に学習できます。

KUMITATE-SENSORの製品情報：

https://kumicla.tatepro.com/asp-products/kumitate-sensor/

KUMITATE：

https://kumicla.tatepro.com/