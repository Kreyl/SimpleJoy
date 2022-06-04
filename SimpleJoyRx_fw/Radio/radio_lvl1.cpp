/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "cc1101.h"
#include "MsgQ.h"
#include "led.h"
#include "Sequences.h"
#include "main.h"

cc1101_t CC(CC_Setup0);

//#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOC
#define DBG_PIN1    3
#define DBG1_SET()  PinSetHi(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinSetLo(DBG_GPIO1, DBG_PIN1)
//#define DBG_GPIO2   GPIOB
//#define DBG_PIN2    9
//#define DBG2_SET()  PinSet(DBG_GPIO2, DBG_PIN2)
//#define DBG2_CLR()  PinClear(DBG_GPIO2, DBG_PIN2)
#else
#define DBG1_SET()
#define DBG1_CLR()
#endif

rLevel1_t Radio;

#if 1 // ================================ Task =================================
static THD_WORKING_AREA(warLvl1Thread, 256);
__noreturn
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    Radio.ITask();
}

#define RSSI_DUMMY  (-117) // Some lowest value not possible
#define RSSI_WEAK   (-72)  // Ignore all weaker signals
#define RSSI_NEAR   (-60)

__noreturn
void rLevel1_t::ITask() {
    EvtMsg_t msg;
    int32_t MaxRssi;
    while(true) {
        msg.ID = evtIdRadioNoone;
        MaxRssi = RSSI_DUMMY;
        for(int N=0; N<4; N++) { // Iterate channels N times
            // Iterate channels
            for(int32_t i = ID_MIN; i <= ID_MAX; i++) {
                CC.SetChannel(ID2RCHNL(i));
                CC.Recalibrate();
                uint8_t RxRslt = CC.Receive(45, &PktRx, RPKT_LEN, &Rssi);   // Double pkt duration + TX sleep time
                if(RxRslt == retvOk) {
//                    Printf("Ch=%u; Rssi=%d; Type=%u\r", ID2RCHNL(i), Rssi, PktRx.Type);
                    if(PktRx.DWord == THE_WORD) {
                        if(PktRx.Type == appmKey) {
                            if(Rssi > RSSI_NEAR) {
                                if(PktRx.Activated != 0) msg.ID = evtIdRadioActivated;
                                else msg.ID = evtIdRadioHiPwrKey;
                                chThdSleepMilliseconds(999);
                                goto CycleEnd;
                            }
                            else if(Rssi > RSSI_WEAK) {
                                if(Rssi > MaxRssi) MaxRssi = Rssi;
                            }
                        }
                        else if(PktRx.Type == appmCrystal) {
                            if(Rssi > RSSI_WEAK) { // Ignore weak signals
                                if(Rssi > MaxRssi) MaxRssi = Rssi;
                            }
                        }
                    } // if the word
                } // if rslt
            } // for i
            TryToSleep(270);
        } // For N
        CycleEnd:
        // Check who is near
        if(msg.ID != evtIdRadioHiPwrKey and MaxRssi != RSSI_DUMMY) {
            msg.ID = (MaxRssi > -60)? evtIdRadioHiPwrCrystal : evtIdRadioLowPwr;
        }
        EvtQMain.SendNowOrExit(msg);
    } // while
}
#endif // task

#if 1 // ============================
uint8_t rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
//    PinSetupOut(DBG_GPIO2, DBG_PIN2, omPushPull);
#endif    // Init radioIC

    if(CC.Init() == retvOk) {
        CC.SetTxPower(CC_TX_PWR);
        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(0);
//        CC.EnterPwrDown();
        // Thread
        chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), NORMALPRIO, (tfunc_t)rLvl1Thread, NULL);
        return retvOk;
    }
    else return retvFail;
}
#endif
