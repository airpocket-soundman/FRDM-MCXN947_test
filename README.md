# FRDM-MCXN947_test

NXP **FRDM-MCXN947** 評価ボード(MCXN947 / Cortex-M33 デュアルコア)で MCUXpresso SDK サンプルを動かし、**原本(SDK そのまま)** と **改良版(自作)** をペアで残していく作業リポジトリ。

運用方針・ローカル SDK の場所・新 west レイアウトの要点・命名規則は **[CLAUDE.md](CLAUDE.md)** を参照。

## 進捗マトリクス

**番号付けの規則**: カテゴリごとに 10 刻みのベース番号を割り当て、原本=ベース、改良版=ベース+1 以降。

| ベース | カテゴリ        | 内容 |
|:-----:|----------------|------|
|  00   | hello_world    | UART/printf の疎通確認 |
|  10   | LED            | GPIO/SysTick/PWM 系の LED 制御 |
|  20   | NPU(eIQ AI)  | TensorFlow Lite Micro + Neutron NPU |

| #   | サンプル                       | 種別   | 由来(SDK 上のパス)                                                                                  | ビルド | 書込 | 動作 | 備考 |
|-----|-------------------------------|--------|--------------------------------------------------------------------------------------------------------|:------:|:----:|:----:|------|
| 00  | `00_hello_world`              | 原本   | `examples/demo_apps/hello_world/` + `_boards/frdmmcxn947/demo_apps/hello_world/`                       | -      | -    | -    | 取り込み済み。ビルド環境未確定 |
| 01  | `01_hello_world_my`           | 改良版 | (上記をベース)                                                                                       | -      | -    | -    | 未着手 |
| 10  | `10_led_blinky_peripheral`    | 原本   | `examples/demo_apps/led_blinky_peripheral/` + `_boards/frdmmcxn947/demo_apps/led_blinky/`(★名前不一致)| OK     | -    | -    | 取り込み済み。SysTick で赤 LED を 2 秒周期点滅。CLI で configure→build→`.elf` 生成確認済 |
| 11  | `11_led_blinky_peripheral`    | 改良版 | `10_led_blinky_peripheral` をベースに改造                                                              | OK     | OK   | NG   | #1 ソフト PWM ブリージング **失敗**: ローカル `board/cm33_core0/hardware_init.c` への SysTick 編集がビルドに反映されず、SysTick が 1 Hz のまま PWM ロジックが回り LED ほぼ点灯せず。`PROJECT_BOARD_PORT_PATH` が SDK パスを指す構成のためローカル board/ 配下は無視される(教訓は 12 へ) |
| 12  | `12_led_blinky_rgb`           | 改良版 | `10_led_blinky_peripheral` をベースに RGB 3 色シーケンス点灯化                                          | OK     | -    | -    | 全改造を `app/led_blinky.c` 内で完結(緑/青の pin mux + GPIO1 クロック + LED INIT を自前で追加)。CLI ビルド通過、書き込み未確認 |
| 20  | `20_tflm_label_image`         | 原本   | `examples/eiq_examples/tflm_label_image/` + `_boards/frdmmcxn947/eiq_examples/tflm_label_image/`       | -      | -    | -    | 取り込み済み。NPU 用 MobileNet V1 で stopwatch 画像を分類。共通モジュール(image/timer/model)依存のため当フォルダ単独ではビルド不可 |

凡例: `-` 未実施 / `OK` 確認済 / `NG` 失敗 / `WIP` 着手中

★ `10_led_blinky_peripheral` は共通アプリ側の名前(`led_blinky_peripheral`)とボード固有側の名前(`led_blinky`)が不一致。`example.yml` の `project-root-path` を信じてコピーする必要がある。

## 進め方

[CLAUDE.md](CLAUDE.md) の「進め方の方針」に従う:

1. **疎通確認** — `01_hello_world` を MCU-Link(J17)経由で書き込み、VCOM(115200/8N1)に `hello world.` が出ることを確認
2. **printf 出力先のバリエーション** — `hello_world_swo`(SWO 経由) → `hello_world_virtual_com`(MCU 直結 USB CDC)
3. **基本周辺機能** — `led_blinky` → `lpuart_*` → `gpio_*` → `lpadc_*`
4. **ボード固有機能** — オンボード TSI(P1_3, 1ch)を `touch_sensing` で
5. **応用** — FreeRTOS、デュアルコア、TrustZone、AI(eIQ + Neutron NPU)へ拡張

## 共通リソース

- `common/` — 自作の共有コード(printf ラッパー、デバッグ LED 制御 等)を将来入れる予定。SDK 由来コードは置かない

## ハードウェア接続(共通)

- 書き込み・デバッグ・printf 受信 → **MCU-Link USB(J17)**
- ターゲット MCU 自身が USB として動くサンプル(`hello_world_virtual_com` 等)を試すときのみ追加で **MCX HS USB(J11)** にも挿す
- VCOM のシリアルは原則 **115200 bps / 8N1**

## ライセンス

SDK 由来コードは **BSD-3-Clause / NXP プロプライエタリ条項** に従う。各ファイルのヘッダのライセンス表記を尊重すること。
