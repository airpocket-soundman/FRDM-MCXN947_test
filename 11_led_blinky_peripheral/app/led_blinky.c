/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * 11_led_blinky_peripheral 改良版 #1:
 *   ソフトウェア PWM による「呼吸する LED」(slow fade-in / slow fade-out の繰り返し)
 *
 *   タイミング階層:
 *     SysTick   : 20 kHz       (main() で SysTick_Config(600) を呼び直して上書き)
 *     PWM tick  : 200 Hz       (PWM_RESOLUTION = 100 で 1 周期 = 100 SysTick = 5 ms)
 *     呼吸 tick : 0.5 Hz       (BRIGHTNESS_UPDATE_INTERVAL = 200 SysTick ごとに 1 段階更新、
 *                              0→100 で 1 秒、100→0 で 1 秒 → 1 サイクル = 2 秒)
 *
 * 注: 当初は board/cm33_core0/hardware_init.c の SysTick_Config を 12000000 → 600 に
 *     書き換える方針だったが、CMakeLists.txt の PROJECT_BOARD_PORT_PATH が SDK パス
 *     を指している都合で **ローカル board/ 配下の編集はビルドに反映されない**。
 *     そこで SDK 標準の BOARD_InitHardware() を呼んだ後、main() 内で SysTick_Config を
 *     呼び直して上書きする方式にした(app/ 配下のファイルはビルドに反映される)。
 */

#include "board.h"
#include "app.h"

/* PWM 解像度: 0..(PWM_RESOLUTION-1) で duty を表現 */
#define PWM_RESOLUTION             100u

/* この回数の SysTick ごとに duty(輝度)を 1 段階更新する。
 * 20 kHz SysTick × 200 = 10 ms ごとに 1 段階 → 0→100 で 1 秒、100→0 で 1 秒 → 往復 2 秒 */
#define BRIGHTNESS_UPDATE_INTERVAL 200u

static volatile uint32_t s_pwm_counter = 0;       /* 0..(PWM_RESOLUTION-1) のループ */
static volatile uint32_t s_brightness  = 0;       /* 現在の duty 値(0..PWM_RESOLUTION) */
static volatile int32_t  s_dir         = 1;       /* +1: 増加中 / -1: 減少中 */
static volatile uint32_t s_bright_tick = 0;       /* duty 更新までのカウンタ */

void SysTick_Handler(void)
{
    /* --- 1. ソフトウェア PWM ---
     * pwm_counter が brightness より小さい間だけ LED を ON にする。
     * LED は active LOW なので ON = 出力 Low(PortClear)、OFF = 出力 High(PortSet)。
     * board.h の LED_RED_ON()/OFF() マクロがこれをラップ済み。 */
    if (s_pwm_counter < s_brightness)
    {
        LED_RED_ON();
    }
    else
    {
        LED_RED_OFF();
    }

    s_pwm_counter++;
    if (s_pwm_counter >= PWM_RESOLUTION)
    {
        s_pwm_counter = 0;
    }

    /* --- 2. ゆっくりした輝度の往復(三角波) --- */
    s_bright_tick++;
    if (s_bright_tick >= BRIGHTNESS_UPDATE_INTERVAL)
    {
        s_bright_tick = 0;

        if (s_dir > 0)
        {
            if (s_brightness >= PWM_RESOLUTION)
            {
                /* 上端に到達したら降下に折り返す */
                s_dir = -1;
            }
            else
            {
                s_brightness++;
            }
        }
        else
        {
            if (s_brightness == 0)
            {
                /* 下端に到達したら上昇に折り返す */
                s_dir = 1;
            }
            else
            {
                s_brightness--;
            }
        }
    }
}

int main(void)
{
    BOARD_InitHardware();

    /* SDK 既定の SysTick(1 Hz)を上書きして 20 kHz に再設定。
     * 12 MHz core clock / 600 = 20 kHz tick → ソフト PWM 200 Hz が回る。 */
    SysTick_Config(600UL);

    while (1)
    {
        /* メインは何もしない。すべて SysTick 割り込みで駆動。 */
    }
}
