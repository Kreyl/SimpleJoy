/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "cc1101.h"
#include "uart.h"

cc1101_t CC(CC_Setup0);

//#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOB
#define DBG_PIN1    10
#define DBG1_SET()  PinSetHi(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinSetLo(DBG_GPIO1, DBG_PIN1)
#define DBG_GPIO2   GPIOB
#define DBG_PIN2    9
#define DBG2_SET()  PinSetHi(DBG_GPIO2, DBG_PIN2)
#define DBG2_CLR()  PinSetLo(DBG_GPIO2, DBG_PIN2)
#else
#define DBG1_SET()
#define DBG1_CLR()
#endif

rLevel1_t Radio;
int8_t Rssi;

//extern EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;

#if 1 // ================================ Task =================================
static THD_WORKING_AREA(warLvl1Thread, 128);
__noreturn
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    Radio.ITask();
}

__noreturn
void rLevel1_t::ITask() {
    while(true) {
        CC.Recalibrate();
        uint8_t Rslt = CC.Receive(270, &PktRx, RPKT_LEN, &Rssi);
        if(Rslt == retvOk) {
            Printf("%d %d %d %d 0x%02X; rssi %d\r",
                    PktRx.Joy[0], PktRx.Joy[1], PktRx.Joy[2], PktRx.Joy[3], PktRx.Flags,
                    Rssi);
            if(PktRx.Sign == 0xCA11) {

            }
        }

//        CC.Transmit(&PktTx, RPKT_LEN);
//        chThdSleepMilliseconds(4);
    } // while true
}
#endif // task

#if 1 // ============================
uint8_t rLevel1_t::Init(uint32_t RPwrId) {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
    PinSetupOut(DBG_GPIO2, DBG_PIN2, omPushPull);
#endif

//    RMsgQ.Init();
    if(CC.Init() == retvOk) {
        CC.SetPktSize(RPKT_LEN);
        CC.DoIdleAfterTx();
        CC.SetChannel(RCHNL_INDX);
        CC.SetTxPower(PwrTable[RPwrId]);
        CC.SetBitrate(CCBitrate10k);
//        CC.EnterPwrDown();
//        Printf("RPwr: %u\r", RPwrId);
        // Thread
        chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
        return retvOk;
    }
    else return retvFail;
}
#endif
