# 20_tflm_label_image(原本)

NXP MCUXpresso SDK 付属の eIQ サンプル `tflm_label_image` を **SDK そのまま** の形で取り込んだ原本サンプル。**TensorFlow Lite Micro** + **Neutron NPU** で MobileNet V1(0.25 / 128 / int8 量子化)を動かし、内蔵された静的画像 "stopwatch.bmp" を ImageNet 1000 クラスに分類する AI 推論デモ。

## 由来

| 項目 | 値 |
|------|-----|
| SDK | MCUX SDK **v06.00-pvw1** |
| SDK ルート | `D:\GitHub\mcuxsdk\mcuxsdk\` |
| 共通アプリ部 由来 | `examples/eiq_examples/tflm_label_image/` |
| ボード固有部 由来 | `examples/_boards/frdmmcxn947/eiq_examples/tflm_label_image/` |
| 対象ボード | FRDM-MCXN947(MCXN947 = Neutron NPU 内蔵) |
| 対象コア | Cortex-M33 Core0(`cm33_core0`) |
| モデル | `mobilenet_v1_0.25_128_quant_int8_npu.tflite`(Neutron Converter で NPU 用に変換済み) |
| 入力画像 | `stopwatch.bmp`(128×128 RGB、CMU/Wikimedia 由来) |
| ライセンス | BSD-3-Clause(SDK 部分)/ Apache-2.0(TFLite-micro)/ NXP プロプライエタリ条項(Neutron Driver / Firmware バイナリ) |

## ディレクトリ構成

```
50_tflm_label_image/
├─ README.md                                           本ファイル
├─ app/                                                共通アプリ部
│   ├─ main.cpp                                        BOARD_Init → MODEL_Init → 無限ループで推論
│   ├─ demo_config.h                                   検出しきい値・モデル名等のマクロ
│   ├─ image_data.h                                    stopwatch.bmp を 128×128 RGB の C 配列に変換したもの
│   ├─ labels.h / labels.txt                           ImageNet 1000 クラスのラベル
│   ├─ stopwatch.bmp                                   元画像(参考用、ビルドには image_data.h を使う)
│   ├─ CMakeLists.txt
│   ├─ CMakePresets.json
│   ├─ mcux_include.json
│   ├─ example.yml
│   ├─ IDE.yml                                         IDE 別プロジェクト生成設定
│   └─ readme.md                                       SDK 由来の詳細英語 readme
└─ board/                                              ボード固有部
    ├─ reconfig.cmake
    ├─ example_board_readme.md
    ├─ pcq_npu/                                        ★ NPU 量子化済みモデル(本サンプルの本体)
    │   ├─ mobilenet_v1_0.25_128_quant_int8_npu.tflite (バイナリ)
    │   ├─ model_data.h                                .tflite を C 配列化したヘッダ + kTensorArenaSize
    │   └─ model_mobilenet_ops_npu.cpp                 NPU カスタムオペレータの登録
    └─ cm33_core0/                                     Core0 専用
        ├─ pin_mux.c / pin_mux.h
        ├─ hardware_init.c                             BOARD_Init() を提供
        ├─ app.h
        ├─ board.readme
        ├─ IDE.yml
        ├─ prj.conf
        ├─ reconfig.cmake
        ├─ arm/MCXN947_cm33_core0_flash.scf            Keil/MDK 用リンカスクリプト
        ├─ gcc/MCXN947_cm33_core0_flash.ld             ARM GCC 用リンカスクリプト
        └─ iar/MCXN947_cm33_core0_flash.icf            IAR EWARM 用リンカスクリプト
```

### 除外したもの

| 種別 | 理由 |
|------|------|
| `.vscode/`、`.claude/` | マシン依存 / ローカル設定 |
| `pcq/`(CPU 用モデル) | FRDM-MCXN947 では NPU 版を使用するため不要(ボードに NPU 搭載) |

### このプロジェクトには **入っていないが必要な依存**

`main.cpp` は以下のヘッダをインクルードしているが、**SDK ツリー側の共通モジュールに依存** している(コピーしていない):

- `board_init.h` — ボード初期化のラッパ
- `image.h` / `image_utils.h` — `examples/eiq_examples/common/image/` 由来
- `model.h` / `output_postproc.h` — `examples/eiq_examples/common/model/` 由来(推測)
- `timer.h` — eIQ 共通のタイマモジュール
- `demo_info.h` — `DEMO_PrintInfo()` の宣言
- TensorFlow Lite Micro / Neutron Driver / Neutron Firmware — SDK の middleware 層

→ **当フォルダ単独でのビルドは不可**。SDK ツリー外でビルドするには上記モジュールも一緒に取り込み、`CMakeLists.txt` のパス参照を修正する必要がある(初期段階では SDK ツリー側でビルドして書き込むのが現実的)。

## 期待動作

[`app/main.cpp`](app/main.cpp) より:

1. `BOARD_Init()` でボード初期化、`TIMER_Init()` でタイマ初期化
2. `DEMO_PrintInfo()` で検出しきい値・モデル名・コア周波数・TensorArena アドレス等を VCOM に出力
3. `MODEL_Init()` で TFLite Micro インタープリタ初期化(NPU 設定含む)
4. 無限ループで:
   - `IMAGE_GetImage()` で入力テンソルに `image_data.h` の画像をロード
   - `MODEL_RunInference()` で推論(Neutron NPU 上で実行)
   - 推論時間を計測し、`MODEL_ProcessOutput()` でトップ N 結果を VCOM に出力

期待される出力(SDK readme より、参考は MIMXRT700-EVK の値):
```
Label image example using a TensorFlow Lite Micro model.
Detection threshold: 23%
Model: mobilenet_v1_0.25_128_quant_int8_npu
Core/NPU Frequency: 324 MHz
TensorArena Addr: ...
TensorArena Size: ...
Model Addr: ...
Model Size: ...

Static data processing:
----------------------------------------
     Inference time: <数 ms>
     Detected: stopwatch (xx%)
----------------------------------------
```

FRDM-MCXN947 上での具体値(コアクロック・推論時間)は実機で確認すること。

## ハードウェア接続

- USB ケーブル → **MCU-Link USB(J17)**(電源・書き込み・printf 受信)
- VCOM: **115200 / 8N1**

## 動作確認ログ

- [ ] SDK ツリー側でビルド成功
- [ ] MCU-Link 経由で書き込み成功
- [ ] VCOM に `Detected: stopwatch (xx%)` が表示
- [ ] 推論時間が現実的な値(ミリ秒オーダー)
- [ ] 当フォルダ単独でのビルドが可能になる構成を試す(将来課題)

## 改造ポイント

**無し**(原本のため)。改良の余地としては SDK readme に詳しく記載されている:

- **モデル差し替え**: `model_data.h` と `model_mobilenet_ops_npu.cpp` を差し替え。NPU 用は **eIQ Neutron SDK 同梱の `neutron-converter`** で `--target mcxn94x` で変換
- **入力画像差し替え**: `image_data.h` を差し替え。Python で OpenCV の `cv2.resize(img, (128, 128))` → `BGR2RGB` → C 配列出力(SDK readme の Python スクリプト参照)
- **kTensorArenaSize**: モデル変更時は `Total data * 1.05` に再設定。不足すると `Failed to resize buffer` エラー

## はまり点・気付き

- **構造が複雑**: 共通アプリ・ボード固有・モデル本体(`pcq_npu/`)・IDE 別リンカスクリプトの 4 階層に分かれる。コピー時に取りこぼしやすい
- **NPU 対応モデルは事前変換が必須**: 普通の TFLite モデルをそのまま `model_data.h` に差し替えても NPU 上では動かない(CPU 実行に落ちる)
- **`pcq_npu` という命名**: PCQ = Per-Channel Quantization。NPU 用と CPU 用を混在させる設計
- **SDK readme が非常に詳しい**(モデル差し替え手順、変換ツールの使い方、エラー時の対処)。改造を始める前に読み返す価値あり
