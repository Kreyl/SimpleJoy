/*
 * SnsPins.h
 *
 *  Created on: 17 џэт. 2015 у.
 *      Author: Kreyl
 */

/* ================ Documentation =================
 * There are several (may be 1) groups of sensors (say, buttons and USB connection).
 *
 */

#pragma once

#include "SimpleSensors.h"

#ifndef SIMPLESENSORS_ENABLED
#define SIMPLESENSORS_ENABLED   FALSE
#endif

#if SIMPLESENSORS_ENABLED
#define SNS_POLL_PERIOD_MS      72

// Button handler
extern void ProcessButtons(PinSnsState_t *PState, uint32_t Len);

const PinSns_t PinSns[] = {
        // Buttons
        {BTN_A_PIN, ProcessButtons},
        {BTN_B_PIN, ProcessButtons},
        {BTN_C_PIN, ProcessButtons},
        {BTN_D_PIN, ProcessButtons},
        {BTN_L_PIN, ProcessButtons},
        {BTN_J1_PIN, ProcessButtons},
        {BTN_J2_PIN, ProcessButtons},
};
#define PIN_SNS_CNT     countof(PinSns)

#define BTN_A_INDX  0
#define BTN_B_INDX  1
#define BTN_C_INDX  2
#define BTN_D_INDX  3
#define BTN_L_INDX  4
#define BTN_J1_INDX  5
#define BTN_J2_INDX  6

#endif  // if enabled
