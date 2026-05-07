# 12_led_blinky_rgb(改良版)

[10_led_blinky_peripheral](../10_led_blinky_peripheral/) をベースに、オンボード **ユーザー RGB LED の 3 色を 1 秒ごとに 赤 → 緑 → 青 → 赤 ... と切り替える** ようにした版。

## ベース

| 項目 | 値 |
|------|-----|
| 原本 | [`10_led_blinky_peripheral`](../10_led_blinky_peripheral/) |
| SDK | MCUX SDK **v06.00-pvw1** |
| 共通アプリ部 由来 | `examples/demo_apps/led_blinky_peripheral/` |
| ボード固有部 由来 | `examples/_boards/frdmmcxn947/demo_apps/led_blinky/` |
| 対象ボード | FRDM-MCXN947 |
| 対象コア | Cortex-M33 Core0(`cm33_core0`) |

## 動作

- SysTick 1 Hz(SDK 既定。リロード値 12,000,000)で割り込み発生
- 1 回ごとに点灯色を進める: **赤 → 緑 → 青 → 赤 → ...**
- 1 周 = **3 秒**

## 改造ポイント

- [x] **#1: RGB 3 色シーケンス点灯**
- [ ] #2:(未着手)

### 改造 #1: RGB 3 色シーケンス

#### 触ったファイル

| ファイル | 変更内容 |
|---|---|
| [app/CMakeLists.txt](app/CMakeLists.txt) | `mcux_add_source` の `BASE_PATH` を SDK から `${CMAKE_CURRENT_LIST_DIR}` に変更 → ローカル `app/led_blinky.c` をビルド対象にする |
| [app/led_blinky.c](app/led_blinky.c) | RGB 拡張初期化 + state machine による 3 色循環ロジックに書き換え |

#### **触っていないファイル(触っても効かない)**

`board/cm33_core0/hardware_init.c` などの **ローカル `board/` 配下のファイルはビルドに反映されない**。`CMakeLists.txt` の `PROJECT_BOARD_PORT_PATH` が SDK パスを指しているため、ボード固有部は SDK の正本がそのまま使われる。**ローカル `board/` 以下のコピーは「読みやすさのための参考置き」**でしかない(これは [11_led_blinky_peripheral](../11_led_blinky_peripheral/) で踏んだ罠の教訓)。

→ ボード固有の振る舞いを変えたい場合(SysTick 周波数の変更・追加ピンの mux 設定など)は **`app/led_blinky.c` の中で自前で再設定する**のが正解。

#### 自前でやった追加初期化(`RGB_Init_Extra()`)

SDK 側 `BOARD_InitHardware()` は **赤 LED(P0_10)分しか初期化しない**。緑(P0_27)と青(P1_2)を点けるには以下が追加で必要:

| 必要な設定 | やったこと |
|---|---|
| 青用 GPIO1 のクロック有効化 | `CLOCK_EnableClock(kCLOCK_Gpio1)` |
| 青用 PORT1 のクロック有効化 | `CLOCK_EnableClock(kCLOCK_Port1)` |
| 緑(P0_27)を GPIO Alt0 に mux | `PORT_SetPinMux(PORT0, 27u, kPORT_MuxAlt0)` |
| 青(P1_2)を GPIO Alt0 に mux | `PORT_SetPinMux(PORT1, 2u, kPORT_MuxAlt0)` |
| 緑・青を出力方向 + 初期 OFF | `LED_GREEN_INIT(LOGIC_LED_OFF)` / `LED_BLUE_INIT(LOGIC_LED_OFF)` |

board.h で定義済みのマクロ(`BOARD_LED_GREEN_GPIO_PIN` / `LED_GREEN_INIT()` など)を使うので、ピン番号や GPIO 番地は手書きしない。

#### LED ピン配置(参考)

| 色 | GPIO | Pin | 物理ピン | board.h マクロ |
|---|---|---|---|---|
| 赤 | GPIO0 | 10 | PIO0_10 | `BOARD_LED_RED_GPIO` / `_PIN` |
| 緑 | GPIO0 | 27 | PIO0_27 | `BOARD_LED_GREEN_GPIO` / `_PIN` |
| 青 | GPIO1 | 2 | PIO1_2 | `BOARD_LED_BLUE_GPIO` / `_PIN` |

すべて **ACTIVE_LOW**(出力 Low = LED ON)。

## ハードウェア接続

- USB ケーブル → **MCU-Link USB(J17)**(電源・書き込み・デバッグ)
- VCOM は未使用

## 動作確認ログ

- [x] CLI からの configure / build 通過: `.elf` / `.bin` 生成、m_text 8,304 B(原本 +668 B)
- [ ] MCU-Link 経由で書き込み成功
- [ ] 期待動作を実機で確認: ユーザー RGB LED が **1 秒ごとに 赤 → 緑 → 青 → 赤** と切り替わる

## はまり点・気付き

- **ローカル `board/` 配下の編集は build に反映されない**(11 で踏んだ罠)。`PROJECT_BOARD_PORT_PATH` を local に向けるか、`mcux_add_source` で個別追加しないとダメ。今回はそこを避けて全部 `app/led_blinky.c` 内で完結させた
- SDK の `BOARD_InitHardware()` / `BOARD_InitPins()` は **赤しか面倒見ない**。緑/青を使う時は GPIO クロック・PORT クロック・pin mux を **自前で追加**する
- LED 全部消してから 1 つだけ点ける書き方(switch の前に全 OFF)にすると、状態機械が増えても切り替えが綺麗。toggle 系で書こうとすると競合しやすい
