/*
 * board.h
 *
 *  Created on: 01.02.2017
 *      Author: Kreyl
 */

#pragma once

#include <inttypes.h>

// ==== General ====
#define BOARD_NAME          "SimpleJoyTx1"
#define APP_NAME            "SimpleJoyTx"

// MCU type as defined in the ST header.
#define STM32F072xB     // no matter, 8 or B

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ         12000000

#define SYS_TIM_CLK             (Clk.APBFreqHz)

#define SIMPLESENSORS_ENABLED   TRUE
#define BUTTONS_ENABLED         TRUE
#define ADC_REQUIRED            TRUE
#define I2C1_ENABLED            FALSE
#define I2C_USE_SEMAPHORE       FALSE

#if 1 // ========================== GPIO =======================================
// PortMinTim_t: GPIO, Pin, Tim, TimChnl, invInverted, omPushPull, TopValue

// Battery measurement
#define BAT_MEAS_PIN    GPIOA, 0
#define BAT_MEAS_EN     GPIOA, 1

#define USB_DETECT      GPIOA, 2

// Buttons
#define BTN_A_PIN       GPIOC, 7, pudPullUp
#define BTN_B_PIN       GPIOC, 8, pudPullUp
#define BTN_C_PIN       GPIOC, 9, pudPullUp
#define BTN_D_PIN       GPIOA, 8, pudPullUp
#define BTN_L_PIN       GPIOC, 6, pudPullUp
#define BTN_J1_PIN      GPIOB, 8, pudPullUp
#define BTN_J2_PIN      GPIOB, 9, pudPullUp

// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10

// USB
#define USB_DM          GPIOA, 11
#define USB_DP          GPIOA, 12

// Bluetooth
#define BT_RST_PIN      GPIOA, 15, omPushPull
#define BTRX_MCUTX      GPIOC, 10
#define BTTX_MCURX      GPIOC, 11
#define BT_PIO11        GPIOC, 12

// DIP switch
#define DIP_SW_CNT      8
#define DIP_SW1         { GPIOB, 0, pudPullUp }
#define DIP_SW2         { GPIOB, 1, pudPullUp }
#define DIP_SW3         { GPIOB, 2, pudPullUp }
#define DIP_SW4         { GPIOB, 3, pudPullUp }
#define DIP_SW5         { GPIOB, 4, pudPullUp }
#define DIP_SW6         { GPIOB, 5, pudPullUp }
#define DIP_SW7         { GPIOB, 6, pudPullUp }
#define DIP_SW8         { GPIOB, 7, pudPullUp }

// LCD
#define LCD_RST_PIN     GPIOB, 10
#define LCD_CE_PIN      GPIOB, 11
#define LCD_DC_PIN      GPIOB, 12
#define LCD_CLK_PIN     GPIOB, 13
#define LCD_BCKLT_PIN   GPIOB, 14, TIM15, 1, invNotInverted, omPushPull, 100
#define LCD_DIN_PIN     GPIOB, 15

// ADC
#define ADC0_PIN        GPIOC, 0
#define ADC1_PIN        GPIOC, 1
#define ADC2_PIN        GPIOC, 2
#define ADC3_PIN        GPIOC, 3
#define ADC4_PIN        GPIOC, 4
#define ADC5_PIN        GPIOC, 5

// LEDs
#define LED_PWR         GPIOC, 13, omPushPull
#define LED_LINK        GPIOC, 14, omPushPull

// Radio: SPI, PGpio, Sck, Miso, Mosi, Cs, Gdo0
#define CC_Setup0       SPI1, GPIOA, 5,6,7, 4, 3

#endif // GPIO

#if 1 // =========================== SPI =======================================
#define LCD_SPI         SPI2

#endif

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN FALSE
#define CMD_UART        USART1
#define UART_USE_INDEPENDENT_CLK    TRUE
#define UART_TXBUF_SZ   1024

#endif

#if 1 // ========================== USB ========================================
#define USBDrv          USBD1   // USB driver to use

// CRS
#define CRS_PRESCALER   RCC_CRS_SYNC_DIV1
#define CRS_SOURCE      RCC_CRS_SYNC_SOURCE_USB
#define CRS_POLARITY    RCC_CRS_SYNC_POLARITY_RISING
#define CRS_RELOAD_VAL  ((48000000 / 1000) - 1) // Ftarget / Fsync - 1
#define CRS_ERROR_LIMIT 34
#define HSI48_CALIBRATN 32
#endif

#if ADC_REQUIRED // ======================= Inner ADC ==========================
#define ADC_MEAS_PERIOD_MS  36
// Clock divider: clock is generated from the APB2
#define ADC_CLK_DIVIDER     adcDiv2

// ADC channels
#define ADC_BAT_CHNL        0
#define ADC0_CHNL           10
#define ADC1_CHNL           11
#define ADC2_CHNL           12
#define ADC3_CHNL           13
#define ADC4_CHNL           14
#define ADC5_CHNL           15

#define ADC_VREFINT_CHNL    17  // All 4xx, F072 and L151 devices. Do not change.
#define ADC_CHANNELS        { ADC_BAT_CHNL, ADC0_CHNL, ADC1_CHNL, ADC2_CHNL, ADC3_CHNL, ADC4_CHNL, ADC5_CHNL, ADC_VREFINT_CHNL }
#define ADC_CHANNEL_CNT     8   // Do not use countof(AdcChannels) as preprocessor does not know what is countof => cannot check
#define ADC_SAMPLE_TIME     ast55d5Cycles
#define ADC_SAMPLE_CNT      8   // How many times to measure every channel

#define ADC_SEQ_LEN         (ADC_SAMPLE_CNT * ADC_CHANNEL_CNT)

#endif

#if 1 // =========================== DMA =======================================
#define STM32_DMA_REQUIRED  TRUE
// ==== Uart ====
#define UART_DMA_TX     STM32_DMA1_STREAM2
#define UART_DMA_RX     STM32_DMA1_STREAM3
#define UART_DMA_CHNL   0   // Dummy

// LCD
#define LCD_DMA         STM32_DMA1_STREAM5

// ==== I2C1 ====
#define I2C1_DMA_TX     STM32_DMA1_STREAM2
#define I2C1_DMA_RX     STM32_DMA1_STREAM3
#define I2C1_DMA_CHNL   0 // Dummy

#if ADC_REQUIRED
#define ADC_DMA         STM32_DMA1_STREAM1
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(0) |   /* dummy */ \
                        DMA_PRIORITY_LOW | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */
#endif // ADC

#endif // DMA
