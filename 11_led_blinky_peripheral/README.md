# 11_led_blinky_peripheral(改良版)

[10_led_blinky_peripheral](../10_led_blinky_peripheral/) をベースに自分で手を入れていく改良版。

## ベース

| 項目 | 値 |
|------|-----|
| 原本 | [`10_led_blinky_peripheral`](../10_led_blinky_peripheral/) |
| SDK | MCUX SDK **v06.00-pvw1** |
| 共通アプリ部 由来 | `examples/demo_apps/led_blinky_peripheral/` |
| ボード固有部 由来 | `examples/_boards/frdmmcxn947/demo_apps/led_blinky/` |
| 対象ボード | FRDM-MCXN947 |
| 対象コア | Cortex-M33 Core0(`cm33_core0`) |

## 原本の動作(出発点)

`SysTick_Handler` 内で `GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN)` を呼び、SysTick 割り込み(1 Hz)ごとにオンボード **赤 LED** を反転 → 1 秒 ON / 1 秒 OFF(2 秒周期)で点滅する。

## 改造ポイント

- [x] **#1: ソフトウェア PWM による「呼吸する LED」**(slow fade-in / slow fade-out のループ)
- [ ] #2:(未着手)
- [ ] #3:(未着手)

### 改造 #1: ソフトウェア PWM ブリージング

ハードウェア PWM ペリフェラル(SCT / FLEXPWM 等)を使わず、**SysTick 割り込みだけで PWM を生成** する構成。peripheral 版の素朴な GPIO トグル構造を保ったまま輝度制御を載せられる学習向けアプローチ。

**タイミング設計**:

| 階層 | 周波数 | 役割 |
|------|-------|------|
| SysTick | **20 kHz**(`SysTick_Config(600)` @ 12 MHz) | 全体のタイムベース |
| PWM 1 周期 | **200 Hz** | 解像度 100 段、5 ms 周期。チラつき不可視 |
| 輝度更新 | **100 Hz**(200 SysTick ごと、10 ms 周期) | 1 段階 = 1% |
| 呼吸 1 サイクル | **0.5 Hz**(2 秒周期) | 0%→100% で 1 秒、100%→0% で 1 秒 |

**変更ファイル**:

- [board/cm33_core0/hardware_init.c](board/cm33_core0/hardware_init.c): `SysTick_Config(12000000UL)` → `SysTick_Config(600UL)` に変更(1 Hz → 20 kHz)
- [app/led_blinky.c](app/led_blinky.c): `SysTick_Handler` を「ソフト PWM + 三角波で輝度往復」のロジックに書き換え。`board.h` の `LED_RED_ON()` / `LED_RED_OFF()` マクロを使用(active LOW のラップ)
- [app/CMakeLists.txt](app/CMakeLists.txt): `mcux_add_source` の `BASE_PATH` を `${SdkRootDirPath}` から `${CMAKE_CURRENT_LIST_DIR}` に変更し、SDK 由来 `led_blinky.c` ではなく **ローカル app/led_blinky.c** をコンパイル対象にする

**ビルド時の確認ポイント**: ninja のログで `Building C object CMakeFiles/.../led_blinky.c.obj`(SDK パス prefix が **無い**)が出ていれば、ローカル copy が使われている証拠。SDK 側の `D_/GitHub/mcuxsdk/.../led_blinky.c.obj` が出ていたら CMakeLists.txt の修正漏れ。

### 主な編集対象ファイル(他の改造候補)

| 改造したいこと | 触る場所 |
|---|---|
| 点滅周期(オリジナル方式) | `hardware_init.c` の `SysTick_Config(...)` リロード値 |
| LED の色 | `app.h` の `BOARD_LED_GPIO` / `BOARD_LED_GPIO_PIN` を Green/Blue 系マクロに差し替え |
| 複数色シーケンス | 同上 + `pin_mux.c` で対応色のピンを GPIO 出力として有効化 + `app.h` で色ごとのマクロを増やす |
| 呼吸の速度 | `app/led_blinky.c` の `BRIGHTNESS_UPDATE_INTERVAL` を変える(大きいほど遅く) |
| 呼吸を非線形(自然な感じ)に | `s_brightness` を直接 duty に使うのではなく、ガンマ補正テーブルや `s_brightness*s_brightness/100` で 2 乗カーブ化 |
| ハードウェア PWM 化 | SCT / CTIMER / FLEXPWM を使う構成に置き換え。**peripheral 版ベースを大きく逸脱するので別フォルダ(`12_led_blinky_hw_pwm` 等)推奨** |

## ハードウェア接続

- USB ケーブル → **MCU-Link USB(J17)**(電源・書き込み・デバッグ)
- VCOM は現状未使用(`PRINTF` 系を呼んでいない)。改造で出力するなら 115200 / 8N1 で受信

## 動作確認ログ

- [x] CLI からの configure / build 通過: `.elf` / `.bin` 生成、m_text 7,976 B(原本比 +340 B、PWM ロジック + SysTick 上書き分)
- [ ] MCU-Link 経由で書き込み成功(修正後の再書き込み待ち)
- [ ] 期待動作を実機で確認: 赤 LED が **3 秒周期で滑らかに明 → 暗 → 明** を繰り返す

## 修正履歴

### 初版で失敗 → 修正版で再挑戦

**初版の症状**: `.elf` を書き込んでも赤 LED がほぼ点灯せず、呼吸動作にならなかった。

**原因**: `build.ninja` を確認すると `hardware_init.c` は **SDK 側のオリジナル**(`D:/GitHub/mcuxsdk/.../led_blinky/cm33_core0/hardware_init.c`)からコンパイルされていた。**ローカル `board/cm33_core0/hardware_init.c` への `SysTick_Config(600UL)` 変更は完全に無視されていた**。

[app/CMakeLists.txt:5](app/CMakeLists.txt#L5) の `PROJECT_BOARD_PORT_PATH` が `${board_root}/${board}/demo_apps/led_blinky` という **SDK ツリー内の絶対パス** を指していて、ボード固有部(`hardware_init.c` / `pin_mux.c` / `peripherals.c` / `app.h` / `prj.conf`)は SDK 側が自動的に拾われる構成だったため。結果として SysTick は **1 Hz のまま**、PWM 解像度 100 段の状態機械が 1 Hz で進むので輝度 1 段階上昇に 300 秒かかり、視覚的にはほぼ消えたままだった。

**修正**(2026-05-07):

1. ローカル `board/cm33_core0/hardware_init.c` を SDK 原本通りの `SysTick_Config(12000000UL)` に **戻した**(どうせ無視されるので、誤解を招かないため)
2. **[app/led_blinky.c](app/led_blinky.c) の `main()` で `SysTick_Config(600UL)` を呼び直し**、SDK 標準の 1 Hz を 20 kHz に上書き

   ```c
   int main(void) {
       BOARD_InitHardware();         /* SDK 標準。SysTick は 1 Hz で起動 */
       SysTick_Config(600UL);        /* 20 kHz に上書き */
       while (1) { }
   }
   ```

`app/` 配下のファイルは [app/CMakeLists.txt:9](app/CMakeLists.txt#L9) で `BASE_PATH ${CMAKE_CURRENT_LIST_DIR}` に変更してあるため、ローカル改造がビルドに反映される。

### 教訓

- **ローカル `board/` 配下のファイルは(現構成では)read-only と思え**。コピーは「読みやすさのための参考置き」でしかない
- ボード固有の振る舞いを変えたければ、選択肢は 2 つ:
  1. ✅ **`app/led_blinky.c` の中で SDK 設定を上書き再設定する**(SysTick なら `SysTick_Config(...)` を main で再呼び出し / pin mux なら `PORT_SetPinMux` を自前で呼ぶ)
  2. `CMakeLists.txt` の `PROJECT_BOARD_PORT_PATH` をローカルに向け直す(SDK 全部入りの依存関係を再構築する必要があるので大仕事)
- → このプロジェクトでは **方針 1 を採用**(12_led_blinky_rgb もこの方針)

## はまり点・気付き

_(改造で踏んだら追記)_
