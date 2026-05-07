# 10_led_blinky_peripheral(原本)

NXP MCUXpresso SDK 付属の `led_blinky_peripheral` デモを **SDK そのまま** の形で取り込んだ原本サンプル。SysTick 割り込みで FRDM-MCXN947 オンボードの **赤 LED** を一定周期で点滅させる、最小の "BSP 動作確認" 用デモ。

## 由来

| 項目 | 値 |
|------|-----|
| SDK | MCUX SDK **v06.00-pvw1** |
| SDK ルート | `D:\GitHub\mcuxsdk\mcuxsdk\` |
| 共通アプリ部 由来 | `examples/demo_apps/led_blinky_peripheral/` |
| ボード固有部 由来 | `examples/_boards/frdmmcxn947/demo_apps/led_blinky/` ← **`_peripheral` サフィックスなし**(peripheral 版と通常版でボードフォルダを共有) |
| 対象ボード | FRDM-MCXN947 |
| 対象コア | Cortex-M33 Core0(`cm33_core0`) |
| ライセンス | BSD-3-Clause |

> ⚠ **新 SDK レイアウトの罠**: `example.yml` の `project-root-path` を見ると `boards/${board}/demo_apps/led_blinky/...` を指しており、**共通アプリ側の名前(`led_blinky_peripheral`)とボード側の名前(`led_blinky`)が一致しない**。同じ MCU 用に通常版・peripheral 版・別 IDE 版があるとき、ボード固有資産は使い回されることがある。

## ディレクトリ構成

```
03_led_blinky_peripheral/
├─ README.md                       本ファイル
├─ app/                            共通アプリ部
│   ├─ led_blinky.c                main()。SysTick ハンドラで GPIO_PortToggle
│   ├─ CMakeLists.txt
│   ├─ CMakePresets.json
│   ├─ example.yml
│   ├─ mcux_include.json
│   └─ readme.md                   SDK 由来の英語 readme
└─ board/                          ボード固有部(SDK の _boards/.../led_blinky/ 由来)
    ├─ pin_mux.c / pin_mux.h       Config Tools 生成のピンマップ
    ├─ peripherals.c / peripherals.h  Config Tools 生成の周辺機器初期化(systick 等)
    ├─ led_blinky.mex              Config Tools プロジェクト
    ├─ reconfig.cmake              ボード共通 CMake オーバーライド
    ├─ example_board_readme.md     SDK 由来のボード手順
    └─ cm33_core0/                 Core0 専用
        ├─ app.h                   BOARD_LED_GPIO / BOARD_LED_GPIO_PIN マクロ定義
        ├─ hardware_init.c         BOARD_InitHardware()。GPIO0 クロック・FRO12M・SysTick・LED 初期化
        ├─ prj.conf
        └─ reconfig.cmake
```

### 除外したもの

| 種別 | 理由 |
|------|------|
| `led_blinky.bin`(2 箇所)| ビルド成果物 |
| `.vscode/` | マシン依存 |
| `.claude/` | ローカル設定 |
| ボード root の `prj.conf` | **SDK には存在しない**(cm33_core0 配下にしかない) |

## 期待動作

1. `BOARD_InitHardware()` が GPIO0 クロックを有効化、ピンマップ、12 MHz FRO クロック、SysTick(12,000,000 リロード=**1 秒周期**)、赤 LED を初期化
2. `main()` は無限ループ(処理しない)
3. **SysTick 割り込み(1 Hz)ごとに `GPIO_PortToggle` で赤 LED をトグル** → 結果として LED は **1 秒 ON / 1 秒 OFF**(=2 秒周期で点滅)

```c
void SysTick_Handler(void) {
    GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
}
```

`BOARD_LED_GPIO` は `app.h` で **`BOARD_LED_RED_GPIO`** に解決(オンボード赤 LED)。

## ハードウェア接続

- USB ケーブル → **MCU-Link USB(J17)**(電源・書き込み・デバッグ)
- VCOM は本サンプルでは未使用(`PRINTF` を呼んでいない)

## 動作確認ログ

- [ ] 自分のビルド環境で当フォルダから単独ビルド可能か
- [ ] MCU-Link 経由で書き込み成功
- [ ] **オンボード赤 LED が約 2 秒周期で点滅**

## 改造ポイント

**無し**(原本のため)。改良版(色変更・周期変更・PWM 輝度制御 等)は別フォルダ(例: `11_led_blinky_pwm`)で行う。

## はまり点・気付き

- **共通アプリ名とボード固有フォルダ名が違う**点(`led_blinky_peripheral` vs `led_blinky`)はこのサンプル特有。`example.yml` の `project-root-path` が真実
- LED 周期を変えるなら `hardware_init.c` の `SysTick_Config(12000000UL)` を書き換える(例: `6000000UL` で 0.5 秒周期 = 1 秒点滅)
- 色を変えるなら `app.h` の `BOARD_LED_GPIO` を `BOARD_LED_GREEN_GPIO` / `BOARD_LED_BLUE_GPIO` に差し替え(対応するマクロが `pin_mux.h` か `board.h` に定義されている前提)
- "peripheral" 系サンプルは Config Tools 生成の `peripherals.c/h` に依存しており、HAL ドライバ直叩き版と比べてピン/周辺の設定が GUI で書き換えやすい
