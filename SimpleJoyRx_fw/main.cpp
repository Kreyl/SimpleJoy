/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "hal.h"
#include "board.h"
#include "MsgQ.h"
#include "uart.h"
#include "shell.h"
#include "kl_lib.h"
#include "kl_adc.h"
#include "led.h"
#include "Sequences.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
void OnCmd(Shell_t *PShell);
void ITask();

LedBlinker_t LedPwr {LED_PWR};
LedBlinker_t LedLink {LED_LINK};

TmrKL_t TmrEverySecond {MS2ST(999), evtIdEverySecond, tktPeriodic};

#endif

int main(void) {
    // ==== Init Clock system ====
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();

    // ==== Init hardware ====
    EvtQMain.Init();
    Uart.Init(115200);
    Printf("\r%S %S\r", APP_NAME, BUILD_TIME);
    Clk.PrintFreqs();

    LedPwr.Init();
    LedLink.Init();
    LedPwr.On();
    LedLink.StartOrRestart(lbsqBlink1s);

    // Adc
//    PinSetupAnalog(LUM_MEAS_PIN);
//    Adc.Init();
//    Adc.EnableVRef();
    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdEverySecond:
                break;

            case evtIdAdcRslt: {
                } break;

            default: break;
        } // switch
    } // while true
} // ITask()

#if UART_RX_ENABLED // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif
