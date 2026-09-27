# KUMITATE-SENSOR

KUMITATE-SENSORは、温度・湿度・照度・3軸加速度を
1枚にまとめたI²Cセンサーボードです。

KUMITATE-C3の拡張ボードとして使用できるほか、
I²Cに対応した各種マイコンや開発ボードから利用できます。

また、HY2.0-4Pコネクタを搭載しており、
M5StackのI²Cセンサーなどと接続して、
I²Cバスをさらに拡張することもできます。

## 特徴

- 温度・湿度・照度・3軸加速度を1枚で測定
- 3種類のセンサーをI²Cで接続
- KUMITATE-C3に対応
- ESP32などの一般的なマイコンからも利用可能
- HY2.0-4Pコネクタ搭載
- M5StackのI²Cセンサーなどを接続可能
- 3.3V動作

## 搭載センサー

| センサー | 測定内容 | I²Cアドレス |
|---|---|---|
| SHT40-A | 温度・湿度 | `0x44` |
| BH1750 | 照度 | `0x23` |
| MSA3S02 | 3軸加速度 | `0x62` |

3種類のセンサーは同じI²Cバスに接続されています。

## コネクタ

KUMITATE-SENSORには、I²C接続用のコネクタを搭載しています。

### I²Cコネクタ

| 信号 | 説明 |
|---|---|
| GND | GND |
| 3V3 | 3.3V電源 |
| SDA | I²Cデータ |
| SCL | I²Cクロック |

KUMITATE-C3だけでなく、ESP32などのI²C対応マイコンから
KUMITATE-SENSORを使用できます。

### HY2.0-4P

HY2.0-4Pコネクタも搭載しています。

M5StackのI²Cセンサーなどを接続し、
KUMITATE-SENSORと同じI²Cバス上で使用できます。

そのため、

```text
マイコン
  │
  │ I²C
  ▼
KUMITATE-SENSOR
  │
  │ HY2.0-4P
  ▼
外部I²Cセンサー
```

のようにセンサーを追加して使用することもできます。

> 接続する機器の電源電圧、I²Cアドレス、ピン配列を確認してから使用してください。

## サンプルプログラム

このリポジトリのサンプルでは、KUMITATE-SENSORに搭載された
3種類のセンサーから値を取得し、シリアルモニターへ表示します。

確認できる項目：

- I²Cデバイスのスキャン
- SHT40-Aの温度
- SHT40-Aの湿度
- BH1750の照度
- MSA3S02のX軸加速度
- MSA3S02のY軸加速度
- MSA3S02のZ軸加速度

## 詳しい解説記事

KUMITATE-SENSORに搭載している各センサーの仕様や、
Arduino・ESP32からの使い方について詳しく解説しています。

### SHT40-A 温度・湿度センサー

SHT40の仕様、I²Cアドレス、温度・湿度の取得方法、
SHT30との違いなどを解説しています。

[SHT40とは？Arduino・ESP32で温度・湿度を測定する方法を解説](https://kumicla.tatepro.com/sht40/)

### BH1750 照度センサー

BH1750のI²Cアドレスや、Arduino・ESP32から
照度（lux）を取得する方法を解説しています。

[BH1750とは？Arduino・ESP32で照度（lux）を測定する方法を解説](https://kumicla.tatepro.com/bh1750/)

### MSA3S02 3軸加速度センサー

MSA3S02のI²Cレジスタや、
X・Y・Z軸の加速度を取得する方法を解説しています。

[MSA3S02とは？Arduino・ESP32で3軸加速度を測定する方法を解説](https://kumicla.tatepro.com/msa3s02/)


## I²Cについて学ぶ

KUMITATE-SENSORでは、3種類のセンサーを
同じSDA・SCLのI²Cバスに接続しています。

I²Cの仕組みや、複数のI²Cデバイスを扱う方法については
以下の記事で詳しく解説しています。

[I²Cとは？通信の仕組みを解説](https://kumicla.tatepro.com/i2c/)

[Arduino・ESP32でI²Cスキャナを使う方法｜I²Cアドレスを調べるサンプルコード](https://kumicla.tatepro.com/i2c-scanner/)

[Arduino・ESP32で複数のI²Cセンサーを同時に使う方法｜アドレス重複の対処も解説](https://kumicla.tatepro.com/i2c-multiple-devices/)

## I²Cアドレス

```cpp
#define BH1750_ADDR   0x23
#define MSA3S02_ADDR  0x62
#define SHT40_ADDR    0x44
```

正常に認識されている場合、I²Cスキャンでは以下の3つが検出されます。

```text
===== I2C Scan =====
Found : 0x23
Found : 0x44
Found : 0x62
====================
```

## シリアルモニター表示例

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

KUMITATE-SENSORを傾けると、
MSA3S02のX/Y/Zの値が変化します。

## KUMITATE-C3で使用する場合

KUMITATE-C3と組み合わせることで、
センサーの読み取りからWi-Fi・ESP-NOW・OLED表示などへ
発展させることができます。

KUMITATE-SENSORはKUMITATE-C3専用品ではないため、
I²Cが利用できる他のマイコンでも使用できます。

使用するマイコンに合わせて、プログラム内の
SDA / SCLピンを変更してください。

```cpp
#define SDA_PIN 8
#define SCL_PIN 9
```

## KUMITATE-SENSOR

製品情報：

https://kumicla.tatepro.com/asp-products/kumitate-sensor/

KUMITATE：

https://kumicla.tatepro.com/