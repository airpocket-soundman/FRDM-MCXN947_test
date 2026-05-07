# 00_hello_world(原本)

NXP MCUXpresso SDK 付属の `hello_world` デモを **SDK そのまま** の形で取り込んだ原本サンプル。改造前のリファレンスとして保持する。

## 由来

| 項目 | 値 |
|------|-----|
| SDK | MCUX SDK **v06.00-pvw1**(west-managed monorepo, 2026 時点) |
| SDK ルート | `D:\GitHub\mcuxsdk\mcuxsdk\` |
| 共通アプリ部 由来 | `examples/demo_apps/hello_world/` |
| ボード固有部 由来 | `examples/_boards/frdmmcxn947/demo_apps/hello_world/` |
| 対象ボード | FRDM-MCXN947 |
| 対象コア | Cortex-M33 Core0(`cm33_core0`) |
| ライセンス | BSD-3-Clause(各ファイルのヘッダ参照) |

## ディレクトリ構成

```
01_hello_world/
├─ README.md                       本ファイル
├─ app/                            共通アプリ部(SDK の examples/demo_apps/hello_world/ 由来)
│   ├─ hello_world.c               main()。SDK バージョン文字列と "hello world." を出力し、以後 echo
│   ├─ CMakeLists.txt
│   ├─ CMakePresets.json
│   ├─ Kconfig
│   ├─ example.yml
│   ├─ mcux_include.json
│   └─ readme.md                   SDK 由来の英語 readme(共通向け)
└─ board/                          ボード固有部(SDK の _boards/frdmmcxn947/demo_apps/hello_world/ 由来)
    ├─ pin_mux.c / pin_mux.h       MCUXpresso Config Tools 生成のピンマップ
    ├─ hello_world.mex             Config Tools プロジェクト(.mex)。pin_mux 再生成元
    ├─ prj.conf                    ボード共通 Kconfig 設定
    ├─ reconfig.cmake              ボード共通 CMake オーバーライド
    ├─ example_board_readme.md     SDK 由来のボード手順(英語)
    └─ cm33_core0/                 Core0 専用(本サンプルが対象とするコア)
        ├─ app.h
        ├─ hardware_init.c         BOARD_InitHardware()。クロック・ピン・デバッグ UART 初期化
        ├─ prj.conf                Core0 用 Kconfig 設定
        └─ reconfig.cmake
```

### コピー対象から除外したもの

| 種別 | 理由 |
|------|------|
| `hello_world.bin`(2 箇所) | SDK 同梱のビルド済みバイナリ。ビルド成果物扱いとしてリポジトリには入れない |
| `.vscode/`(c_cpp_properties.json, launch.json, mcuxpresso-tools.json, settings.json, tasks.json) | マシン依存の絶対パスを含む可能性。必要なら自分でビルド環境を整えた上で再生成 |
| `.claude/settings.local.json` | Claude Code のローカル設定で機密性あり |

## 期待動作

`hello_world.c` は次の処理を行う(SDK そのまま):

1. `BOARD_InitHardware()` でクロック設定、ピンマップ、デバッグ UART を初期化
2. `MCUX SDK version: <version>` を VCOM に出力
3. `hello world.` を出力
4. 以後 1 文字ずつ受信して同じ文字をエコーバック(無限ループ)

```c
PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);
PRINTF("hello world.\r\n");
while (1) {
    ch = GETCHAR();
    PUTCHAR(ch);
}
```

## ハードウェア接続 / シリアル設定

- **Type-C USB ケーブル → MCU-Link USB(J17)** に挿す(書き込み・デバッグ・printf 受信を兼用)
- シリアル: **115200 bps / 8 データビット / パリティ無し / 1 ストップビット / フロー制御無し**
- デバッグ UART は **FLEXCOMM4 + FRO 12M**(`hardware_init.c` 参照)

## ビルド・書き込み手順(未確定)

ビルド環境(MCUXpresso for VS Code / MCUXpresso IDE / west + CMake 直叩き)が確定したら追記する。確定する前は、SDK ツリー側(`D:\GitHub\mcuxsdk\mcuxsdk\examples\demo_apps\hello_world\`)で動作確認しておき、当フォルダはあくまで「SDK のスナップショット」として保持する。

## 動作確認ログ

- [ ] 自分のビルド環境で当フォルダから単独ビルド可能か(現時点では SDK ツリー外なので未確認)
- [ ] MCU-Link 経由で書き込み成功
- [ ] VCOM に `MCUX SDK version: ...` が表示
- [ ] VCOM に `hello world.` が表示
- [ ] キー入力がエコーバックされる

## 改造ポイント

**無し**(原本のため)。改良版は `01_hello_world_my/`(予定)で行い、こちらは触らない。

## はまった点・気付き

- 新 SDK レイアウト(west monorepo)では **共通アプリ部** と **ボード固有部** が物理的に別ディレクトリに分かれる。両方を取り込まないとビルドに必要なファイル(`pin_mux.*`, `hardware_init.c`, `prj.conf` 等)が揃わない
- `hardware_init.c` でデバッグ UART は **FLEXCOMM4 を FRO 12M で駆動**。printf 出力先を変更する場合はここを書き換える
- `.mex` を残しておけば MCUXpresso Config Tools でピン/クロック設定を GUI 編集可能
