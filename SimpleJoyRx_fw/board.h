/*
 * board.h
 *
 *  Created on: 01.02.2017
 *      Author: Kreyl
 */

#pragma once

#include <inttypes.h>

// ==== General ====
#define BOARD_NAME          "Lume3"
#define APP_NAME            "Lume"

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

// Luminocity measurement
#define LUM_MEAS_PIN    GPIOA, 1

// Buttons
#define BTN1_PIN        GPIOB, 3, pudPullUp
#define BTN2_PIN        GPIOB, 4, pudPullUp
#define BTN3_PIN        GPIOB, 5, pudPullUp
#define BTN4_PIN        GPIOB, 6, pudPullUp

// LCD
#define LCD_SDA         GPIOB, 10
#define LCD_XRES        GPIOB, 11
#define LCD_SCLK        GPIOB, 12
#define LCD_XCS         GPIOB, 13
#define LCD_BCKLT       { GPIOB, 14, TIM15, 1, invNotInverted, omPushPull, 100 }
#define LCD_PWR         GPIOA, 6

// Battery measurement
#define BAT_MEAS_PIN    GPIOA, 2

// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10

// LEDs
#define LEDWS_PIN       GPIOB, 15, omPushPull, pudNone, AF0, psHigh

#endif // GPIO

#if 1 // =========================== SPI =======================================
#define LEDWS_SPI       SPI2
#endif

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN FALSE
#define CMD_UART        USART1
#define UART_USE_INDEPENDENT_CLK    TRUE
#define UART_TXBUF_SZ   1024

// LCD USART
#define LCD_UART_SPEED      100000
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
#define ADC_MEAS_PERIOD_MS  450
// Clock divider: clock is generated from the APB2
#define ADC_CLK_DIVIDER     adcDiv2

// ADC channels
#define ADC_LUM_CHNL        1

#define ADC_VREFINT_CHNL    17  // All 4xx, F072 and L151 devices. Do not change.
#define ADC_CHANNELS        { ADC_LUM_CHNL, ADC_VREFINT_CHNL }
#define ADC_CHANNEL_CNT     2   // Do not use countof(AdcChannels) as preprocessor does not know what is countof => cannot check
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

#define LCD_DMA         STM32_DMA1_STREAM7  // USART3 TX

#define LEDWS_DMA       STM32_DMA1_STREAM5  // SPI2 TX

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
