/*
 * board.h
 *
 *  Created on: 01.02.2017
 *      Author: Kreyl
 */

#pragma once

#include <inttypes.h>

// ==== General ====
#define BOARD_NAME          "SimpleJoyRx1"
#define APP_NAME            "SimpleJoyRx"

// MCU type as defined in the ST header.
#define STM32F072xB     // no matter, 8 or B

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ         12000000

#define SYS_TIM_CLK             (Clk.APBFreqHz)

#define SIMPLESENSORS_ENABLED   FALSE
#define BUTTONS_ENABLED         FALSE
#define ADC_REQUIRED            FALSE
#define I2C1_ENABLED            FALSE
#define I2C_USE_SEMAPHORE       FALSE

#if 1 // ========================== GPIO =======================================
// PortMinTim_t: GPIO, Pin, Tim, TimChnl, invInverted, omPushPull, TopValue

// Battery measurement
#define BAT_MEAS_PIN    GPIOA, 0
#define BAT_MEAS_EN     GPIOA, 1

// Servo
#define SRV1_PIN        GPIOA, 8, TIM1, 1
#define SRV2_PIN        GPIOA, 9, TIM1, 2
#define SRV3_PIN        GPIOA, 10, TIM1, 3
#define SRV4_PIN        GPIOA, 2, TIM15, 1
#define SRV5_PIN        GPIOA, 3, TIM15, 2
#define SRV6_PIN        GPIOA, 4, TIM14, 1

#define USB_DETECT      GPIOA, 5

// PWM
#define PWM1_PIN        GPIOC, 6, TIM3, 1, invNotInverted, omPushPull, 255
#define PWM2_PIN        GPIOC, 7, TIM3, 2, invNotInverted, omPushPull, 255
#define PWM3_PIN        GPIOC, 8, TIM3, 3, invNotInverted, omPushPull, 255
#define PWM4_PIN        GPIOC, 9, TIM3, 4, invNotInverted, omPushPull, 255
#define PWM5_PIN        GPIOA, 6, TIM16, 1, invNotInverted, omPushPull, 255
#define PWM6_PIN        GPIOA, 7, TIM17, 1, invNotInverted, omPushPull, 255

// Moter Dir
#define DIR1_PIN        GPIOC, 10
#define DIR2_PIN        GPIOC, 11
#define DIR3_PIN        GPIOC, 12
#define DIR4_PIN        GPIOC, 13

// USB
#define USB_DM          GPIOA, 11
#define USB_DP          GPIOA, 12

// LEDs
#define LED_PWR         GPIOC, 0, omPushPull
#define LED_LINK        GPIOC, 1, omPushPull

// UART
#define UART_GPIO       GPIOB
#define UART_TX_PIN     6
#define UART_RX_PIN     7

// Radio: SPI, PGpio, Sck, Miso, Mosi, Cs, Gdo0
#define CC_Setup0       SPI1, GPIOB, 3,4,5, 2, 1

// DIP switch
#define DIP_SW_CNT      8
#define DIP_SW1         { GPIOB, 8, pudPullUp }
#define DIP_SW2         { GPIOB, 9, pudPullUp }
#define DIP_SW3         { GPIOB, 10, pudPullUp }
#define DIP_SW4         { GPIOB, 11, pudPullUp }
#define DIP_SW5         { GPIOB, 12, pudPullUp }
#define DIP_SW6         { GPIOB, 13, pudPullUp }
#define DIP_SW7         { GPIOB, 14, pudPullUp }
#define DIP_SW8         { GPIOB, 15, pudPullUp }

#endif // GPIO

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
