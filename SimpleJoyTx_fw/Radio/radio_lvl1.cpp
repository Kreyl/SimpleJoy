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
extern bool TxOn;
static bool TxWasOn = false;

#if 1 // ================================ Task =================================
static THD_WORKING_AREA(warLvl1Thread, 256);
__noreturn
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    Radio.ITask();
}

__noreturn
void rLevel1_t::ITask() {
    while(true) {
        RMsg_t Msg = RMsgQ.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case RMSGID_PKT:
//                Msg.Pkt.Print();
                if(TxOn) {
                    TxWasOn = true;
                    // Transmit
                    DBG1_SET();
                    CC.Recalibrate();
                    CC.Transmit(&Msg.Pkt, RPKT_LEN);
                    DBG1_CLR();
                }
                else {
                    if(TxWasOn) {
                        TxWasOn = false;
                        CC.EnterPwrDown();
                    }
                }
                break;

            case RMSGID_CHNL:
                CC.SetChannel(Msg.Value);
                if(!TxOn) CC.EnterPwrDown();
                break;

            default: break;
        } // switch
//
//
//            Printf("Par %u; Rssi=%d\r", PktRx.CmdID, Rssi);
            // Transmit reply, it formed inside OnRadioRx
//            if(OnRadioRx() == retvOk) CC.Transmit(&PktTx);
//        } // if RxRslt ok
    } // while
}
#endif // task

#if 1 // ============================
uint8_t rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
//    PinSetupOut(DBG_GPIO2, DBG_PIN2, omPushPull);
#endif    // Init radioIC

    RMsgQ.Init();

    if(CC.Init() == retvOk) {
        CC.SetTxPower(CC_TX_PWR);
        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(0);
        CC.EnterPwrDown();
        // Thread
        chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), NORMALPRIO, (tfunc_t)rLvl1Thread, NULL);
        return retvOk;
    }
    else return retvFail;
}
#endif
