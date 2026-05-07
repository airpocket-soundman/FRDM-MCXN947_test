/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * 12_led_blinky_rgb 改良版 #1:
 *   オンボードユーザー RGB LED の 3 色を 1 秒ごとに 赤 → 緑 → 青 → 赤 ... と切り替える。
 *
 *   ベース: 10_led_blinky_peripheral(SDK の led_blinky_peripheral)
 *
 * 注意:
 *   SDK の BOARD_InitHardware() は 赤 LED(P0_10)用に PORT0 / GPIO0 クロック有効化と
 *   pin mux + PDDR 設定をしてくれるが、緑(P0_27) / 青(P1_2)については何もしない。
 *   そこで main() の最初で「自前の RGB 拡張初期化」を呼ぶ必要がある。
 *
 *   SysTick は SDK の hardware_init.c で 1 Hz(SysTick_Config(12000000UL))に設定済み。
 *   1 Hz ハンドラ呼び出しごとに 1 色進む → 1 周 = 3 秒。
 */

#include "board.h"
#include "app.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* 現在点灯している色: 0=赤, 1=緑, 2=青 */
static volatile uint32_t s_color_state = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief 緑(P0_27)と青(P1_2)を GPIO 出力として有効化する。
 *        赤(P0_10)は SDK 側 BOARD_InitHardware() で初期化済みなので触らない。
 */
static void RGB_Init_Extra(void)
{
    /* PORT1 と GPIO1 のクロックを有効化(青 LED は P1_2 = GPIO1.2 のため)。
     * PORT0 と GPIO0 は SDK 側で既に有効化されているが、二重に有効化しても害はない。 */
    CLOCK_EnableClock(kCLOCK_Port0);
    CLOCK_EnableClock(kCLOCK_Port1);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* pin mux: 緑と青を Alt0(=GPIO 機能)に割り当てる。
     * SDK の pin_mux.c は赤(P0_10)しか設定しないため、ここで補う。 */
    PORT_SetPinMux(PORT0, BOARD_LED_GREEN_GPIO_PIN, kPORT_MuxAlt0);
    PORT_SetPinMux(PORT1, BOARD_LED_BLUE_GPIO_PIN,  kPORT_MuxAlt0);

    /* 緑と青を OFF(active LOW なので出力 HIGH)で初期化、出力方向にする。 */
    LED_GREEN_INIT(LOGIC_LED_OFF);
    LED_BLUE_INIT(LOGIC_LED_OFF);
}

/*!
 * @brief SysTick(1 Hz)ハンドラ。1 秒ごとに 1 色だけ点灯する状態に進む。
 */
void SysTick_Handler(void)
{
    /* 一旦すべて消灯してから現在の色だけを点灯する(active LOW)。 */
    LED_RED_OFF();
    LED_GREEN_OFF();
    LED_BLUE_OFF();

    switch (s_color_state)
    {
        case 0: LED_RED_ON();   break;
        case 1: LED_GREEN_ON(); break;
        case 2: LED_BLUE_ON();  break;
        default: break;
    }

    s_color_state++;
    if (s_color_state >= 3u)
    {
        s_color_state = 0;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* SDK 標準のボード初期化(赤 LED 部分のみ)。SysTick は 1 Hz で起動済み。 */
    BOARD_InitHardware();

    /* 緑・青 LED を自前で追加初期化。 */
    RGB_Init_Extra();

    while (1)
    {
        /* 全ての処理は SysTick 割り込みで駆動。 */
    }
}
