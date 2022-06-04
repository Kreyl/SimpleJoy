/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#pragma once

#include "kl_lib.h"
#include "ch.h"
#include "cc1101.h"
#include "kl_buf.h"
#include "shell.h"
#include "MsgQ.h"

#define CC_TX_PWR   CC_PwrPlus10dBm

#if 1 // =========================== Pkt_t =====================================
struct rPkt_t {
    uint32_t DWord;
    uint16_t Type;
    uint16_t Activated;
//    bool operator == (const rPkt_t &APkt) { return (DWord32 == APkt.DWord32); }
//    rPkt_t& operator = (const rPkt_t &Right) { DWord32 = Right.DWord32; return *this; }
} __attribute__ ((__packed__));
#define RPKT_LEN    sizeof(rPkt_t)

#define THE_WORD        0xCA115EA1
#endif


#if 1 // ======================= Channels & cycles =============================
#define RCHNL_MIN       10
#define ID2RCHNL(ID)    (RCHNL_MIN + ID)
#endif

#define RMSG_Q_LEN      18
#define RMSGID_PKT      1
#define RMSGID_CHNL     2

union RMsg_t {
    uint32_t DWord[3];
    rPkt_t Pkt;
    struct {
        uint32_t _Rsrvd;
        uint32_t Value;
        uint32_t ID;
    };
    RMsg_t& operator = (const RMsg_t &Right) {
        DWord[0] = Right.DWord[0];
        DWord[1] = Right.DWord[1];
        DWord[2] = Right.DWord[2];
        return *this;
    }
    RMsg_t() {
        DWord[0] = 0;
        DWord[1] = 0;
        DWord[2] = 0;
    }
    RMsg_t(rPkt_t &APkt)  { ID = RMSGID_PKT;  Pkt = APkt; }
    RMsg_t(uint8_t AChnl) { ID = RMSGID_CHNL; Value = AChnl; _Rsrvd = 0; }
} __attribute__((__packed__));


class rLevel1_t {
private:
    void TryToSleep(uint32_t SleepDuration) {
//        if(SleepDuration >= MIN_SLEEP_DURATION_MS) CC.EnterPwrDown();
        chThdSleepMilliseconds(SleepDuration); // XXX
    }
public:
    int8_t Rssi;
//    EvtMsgQ_t<RMsg_t, RMSG_Q_LEN> RMsgQ;
    rPkt_t PktRx, PktTx;
    uint8_t Init();
    // Inner use
    void ITask();
};

extern rLevel1_t Radio;
