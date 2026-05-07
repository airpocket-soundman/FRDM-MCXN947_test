# FRDM-MCXN947_test — リポジトリ運用方針

このリポジトリは **FRDM-MCXN947(NXP MCXN947 搭載評価ボード)** で SDK サンプル(hello_world など)とその改良版を並べて作っていくための作業場です。

## 目的

- NXP MCUXpresso SDK のサンプルを 1 つずつ動かし、結果を残す
- サンプルを「**原本(SDK 由来そのまま)**」と「**改良版(自分の手を入れた版)**」を **並列に** 並べて差分を git で追える形にする
- 各サンプルの動作確認・改造ポイント・はまりどころを README として残し、後から見返せるログにする

## ローカル SDK の場所

```
D:\GitHub\mcuxsdk\mcuxsdk\
```

- バージョン: **MCUX SDK v06.00-pvw1**(2026 時点、west-managed monorepo 形式)
- 西スタイルの新レイアウトであり、従来の "ZIP SDK" とは構成が異なる

### SDK の新レイアウトの要点

| パス | 役割 |
|------|------|
| `examples/_boards/frdmmcxn947/<category>/<example>/` | **ボード固有ファイル**(pin_mux, hardware_init, board readme, prj.conf, *.mex, reconfig.cmake) |
| `examples/_boards/frdmmcxn947/<category>/<example>/cm33_core0/` | コア固有ファイル(M33 Core0 用)。Core1 用は `cm33_core1/` |
| `examples/<category>/<example>/` | **共通アプリソース**(hello_world.c, freertos_hello.c など。ボード/コア非依存) |
| `examples/<category>/<example>/CMakeLists.txt` | 共通ビルド設定 |

→ サンプルを取り込むときは、**ボード固有部分**(`_boards/frdmmcxn947/...`)と **共通アプリソース**(`examples/<category>/<example>/...`)の **両方** をコピーする必要がある。

### マルチコア / TrustZone は例外的な構造

- **multicore_examples/hello_world** には `cm33_core0/cm33_core1` ではなく **`primary/`(=Core0)・`secondary/`(=Core1)** のサブフォルダがある
- **trustzone_examples/hello_world** はボード側 `_boards/frdmmcxn947/trustzone_examples/hello_world/` 配下に **`hello_world_ns`(Non-Secure)・`hello_world_s`(Secure)** が分かれている。共通ソースは `examples/trustzone_examples/hello_world_ns/` と `hello_world_s/` の **別ディレクトリ** に入る(他のサンプルと違って 1 段目から分離)

## ハードウェア接続(共通)

- **書き込み・デバッグ・printf 受信** → **MCU-Link USB(J17)** に挿す
- **ターゲット MCU 自身が USB デバイス/ホストとして動くサンプル**(`hello_world_virtual_com` など)を試すときだけ追加で **MCX HS USB(J11)** にも挿す
- VCOM のシリアルは原則 **115200 bps / 8N1**

## ディレクトリ構成方針

```
FRDM-MCXN947_test/
├─ README.md                     全サンプルの一覧 + 進捗ステータス表
├─ CLAUDE.md                     本ファイル(運用方針・SDK 場所など)
├─ .gitignore                    ビルド成果物・IDE 個人設定を除外
├─ common/                       自分の共通コード(printf ラッパー、デバッグ LED 制御 等)
│
│   ─── カテゴリ 00: hello_world / printf 疎通 ───
├─ 00_hello_world/               原本(SDK 由来そのまま)
│   ├─ app/                      共通アプリ部(SDK の examples/<category>/<example>/ 由来)
│   ├─ board/                    ボード固有部(SDK の _boards/frdmmcxn947/<category>/<example>/ 由来)
│   │   └─ cm33_core0/           Core 専用ファイル
│   └─ README.md                 SDK のどのバージョンから取ったか / 動作確認結果 / はまり点
├─ 01_hello_world_my/            改良版(色付き出力、バナー、コマンド追加など)
│
│   ─── カテゴリ 10: LED / GPIO 制御 ───
├─ 10_led_blinky_peripheral/     原本
├─ 11_led_blinky_pwm/            改良版(PWM で輝度制御 等)
│
│   ─── カテゴリ 20: NPU / eIQ AI ───
├─ 20_tflm_label_image/          原本(NPU 用 MobileNet V1)
├─ 21_tflm_label_image_my/       改良版(モデル差し替え、入力画像差し替え 等)
│
│   ─── 以下は今後の予定枠(暫定割り当て) ───
├─ 30_lpuart_loopback/
├─ 40_lpadc_basic/
├─ 50_touch_sensing/             オンボード TSI(P1_3 1ch)
└─ ...
```

### 運用ルール

- **2 桁プレフィックス**で並び順を固定
- **カテゴリ単位で 10 刻みのベース番号** を割り当てる(00=hello_world, 10=LED, 20=NPU, ...)
- 同カテゴリ内では **原本=ベース番号**、**改良版=ベース+1, +2, ...**(例: `00_hello_world` / `01_hello_world_my`、`10_led_blinky_peripheral` / `11_led_blinky_pwm`)
- 各サンプルは内部を **`app/`(SDK 共通アプリ部)/ `board/`(SDK ボード固有部、`cm33_core0/` を子に持つ)** で二分割。新 SDK の物理レイアウトと一致させると差分が追いやすい
- 各サンプルフォルダに **必ず README.md** を置き、最低限以下を記録:
  - 由来(SDK 上のパス)
  - 動作確認した内容(boot 時のシリアル出力例 / LED 状態 / etc.)
  - 改造したポイント(改良版の場合)
  - はまった点・気付き
- ルートの `README.md` に **進捗マトリクス**(サンプル名 / ビルド可否 / 動作可否 / メモ)を維持
- `common/` には複数サンプルで共有する自作コードのみ(SDK 由来コードは置かない)

## .gitignore の方針

以下は除外する:

- `build/`、`Debug/`、`Release/`、`out/` 等のビルド成果物
- `*.o`、`*.elf`、`*.map`、`*.lst`(ELF だけは案件によって追跡することもある)
- `.settings/`、`.metadata/`、`*.launch`、`.cproject`/`.project` の中で個人パス依存のもの
- VS Code 個人設定 `.vscode/settings.json`(共通の `tasks.json` などはコミットして良い)
- MCUXpresso IDE の `__macosx`、`workspace_*` 等

## ビルド・書き込みの想定フロー

(IDE が確定したら追記する)

候補:
- **MCUXpresso for VS Code** + arm-none-eabi-gcc + CMake(SDK v6 の標準フロー)
- **MCUXpresso IDE(Eclipse 系)** — 旧来 SDK ZIP との親和性が高い
- **CMake 直叩き**(`west build` / `cmake --build`)

## 進め方の方針

1. **疎通確認**:`hello_world` を最初に丸ごと取り込んで MCU-Link 経由で書き込み、VCOM に "hello world." が出ることを確認
2. **printf 出力先のバリエーション** を試す:`hello_world_swo`(SWO 経由) → `hello_world_virtual_com`(MCU 直結 USB CDC)
3. **基本周辺機能**:`led_blinky` → `lpuart_*` → `gpio_*` → `lpadc_*`
4. **ボード固有**:オンボード TSI(P1_3, 1ch)を `touch_sensing` で試す
5. **応用**:FreeRTOS、デュアルコア、TrustZone、AI(eIQ + Neutron NPU)へ拡張

## メモ

- このリポジトリの内容を改造して再配布する場合、**SDK 由来コードのライセンス(BSD-3-Clause / NXP プロプライエタリ条項)** を尊重すること
- 各サンプルフォルダにオリジナルの `LICENSE` / `COPYING-BSD-3` をなるべく一緒にコピーする
- ボード仕様の一次資料: **UM12018 FRDM-MCXN947 Board User Manual**(NXP 公式)
- MCU リファレンス: **MCXN947 Reference Manual**、**MCX N Series User Guide**
