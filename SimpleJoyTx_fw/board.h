/*
 * board.h
 *
 *  Created on: 01.02.2017
 *      Author: Kreyl
 */

#pragma once

// ==== General ====
#define BOARD_NAME          "SimpleJoyTx1"
#define APP_NAME            "SimpleJoyTx"

// MCU type as defined in the ST header.
#define STM32F072xB     // no matter, 8 or B

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ         12000000

// Ch timer
#define STM32_ST_IRQ_PRIORITY   2
#define STM32_ST_USE_TIMER      2
#define STM32_TIMCLK1           (Clk.APBFreqHz)

#define SIMPLESENSORS_ENABLED   TRUE
#define BUTTONS_ENABLED         TRUE
#define ADC_REQUIRED            TRUE
#define I2C1_ENABLED            FALSE
#define I2C_USE_SEMAPHORE       FALSE

// ADC timer
#define ADC_TIM                 TIM1

#if 1 // ========================== GPIO =======================================
// PortMinTim_t: GPIO, Pin, Tim, TimChnl, invInverted, omPushPull, TopValue

// Battery measurement
#define BAT_MEAS_PIN    GPIOA, 1

// Buttons
#define BTN_A_PIN       GPIOC, 7, pudPullUp
#define BTN_B_PIN       GPIOC, 8, pudPullUp
#define BTN_C_PIN       GPIOC, 9, pudPullUp
#define BTN_D_PIN       GPIOA, 8, pudPullUp
#define BTN_L_PIN       GPIOC, 6, pudPullUp
#define BTN_J1_PIN      GPIOB, 8, pudPullUp
#define BTN_J2_PIN      GPIOB, 9, pudPullUp

// LED RGB
#define LED_B_PIN       { GPIOB, 1, TIM3, 4, invNotInverted, omPushPull, 255 }
#define LED_R_PIN       { GPIOB, 0, TIM3, 3, invNotInverted, omPushPull, 255 }
#define LED_G_PIN       { GPIOB, 4, TIM3, 1, invNotInverted, omPushPull, 255 }
// LEDs
#define LED_PWR         GPIOC, 13, omPushPull
#define LED_LINK        GPIOC, 14, omPushPull

// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10

// USB
#define USB_DETECT_PIN  GPIOA, 2
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
#define LCD_BCKLT_PIN   {GPIOB, 14, TIM15, 1, invNotInverted, omPushPull, 100}
#define LCD_DIN_PIN     GPIOB, 15

// ADC
#define ADC0_PIN        GPIOC, 0, 10
#define ADC1_PIN        GPIOC, 1, 11
#define ADC2_PIN        GPIOC, 2, 12
#define ADC3_PIN        GPIOC, 3, 13
#define ADC4_PIN        GPIOC, 4, 14
#define ADC5_PIN        GPIOC, 5, 15

// Radio: SPI, PGpio, Sck, Miso, Mosi, Cs, Gdo0
#define CC_Setup0       SPI1, GPIOA, 5,6,7, GPIOA,4, GPIOA,3

#endif // GPIO

#define LCD_SPI         SPI2

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

#if 1 // =========================== DMA =======================================
#define STM32_DMA_REQUIRED  TRUE
// ==== Uart ====
#define UART_DMA_TX_MODE(Chnl) (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_LOW | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_TCIE)
#define UART_DMA_RX_MODE(Chnl) (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_MEDIUM | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_P2M | STM32_DMA_CR_CIRC)
#define UART_DMA_TX     STM32_DMA_STREAM_ID(1, 2)
#define UART_DMA_RX     STM32_DMA_STREAM_ID(1, 3)
#define UART_DMA_CHNL   0   // Dummy

#define LCD_DMA         STM32_DMA_STREAM_ID(1, 5)

#if ADC_REQUIRED
#define ADC_DMA         STM32_DMA_STREAM_ID(1, 1)
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(0) |   /* dummy */ \
                        DMA_PRIORITY_HIGH | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */
#endif // ADC

#endif // DMA

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN FALSE
#define UART_TXBUF_SZ   2048
#define UART_RXBUF_SZ   512
#define CMD_BUF_SZ      256

#define CMD_UART        USART1


#define CMD_UART_PARAMS \
    CMD_UART, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN, \
    UART_DMA_TX, UART_DMA_RX, UART_DMA_TX_MODE(UART_DMA_CHNL), UART_DMA_RX_MODE(UART_DMA_CHNL), \
    uartclkHSI // independent clock

#endif
