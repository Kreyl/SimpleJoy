#include "hal.h"
#include "board.h"
#include "MsgQ.h"
#include "uart.h"
#include "shell.h"
#include "kl_lib.h"
#include "kl_adc.h"
#include "led.h"
#include "Sequences.h"
#include "kl_servo.h"
#include "radio_lvl1.h"
#include "kl_adc.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
void OnCmd(Shell_t *PShell);
void ITask();

static void OnRadioRx();

// Servo
#define SRV_CNT     6
static const Servo_t Srv1{SRV1_PIN};
static const Servo_t Srv2{SRV2_PIN};
static const Servo_t Srv3{SRV3_PIN};
static const Servo_t Srv4{SRV4_PIN};
static const Servo_t Srv5{SRV5_PIN};
static const Servo_t Srv6{SRV6_PIN};
static const Servo_t* Srv[SRV_CNT] = { &Srv1, &Srv2, &Srv3, &Srv4, &Srv5, &Srv6 };

TmrKL_t TmrEverySecond {MS2ST(999),  evtIdEverySecond, tktPeriodic};
#endif

int main(void) {
    // ==== Init Clock system ====
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();

    // ==== Init hardware ====
    EvtQMain.Init();
    Uart.Init();
    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

    // LEDs

    // Servo XXX
//    for(int i=0; i<SRV_CNT; i++) {
//        Srv[i]->Init();
//        Srv[i]->SetAngle_dg(90);
//    }

//    if(
            Radio.Init();
//    == retvOk) LedLink.StartOrRestart(lbsqBlink1s);
//    else LedLink.StartOrRestart(lbsqFailure);

    TmrEverySecond.StartOrRestart();

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

            case evtIdRadioRx:
                OnRadioRx();
                break;

            case evtIdEverySecond: {
            } break;

            default: break;
        } // switch
    } // while true
} // ITask()

void OnRadioRx() {
//    rPkt_t Pkt = Radio.PktRx;
//    Pkt.Print();
    // Servo
//    for(int i=0; i<4; i++) {
//        Srv[i]->SetValue(Pkt.Ch[i], -127L, 127L);
//    }
}

#if 1 // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }

    else if(PCmd->NameIs("Srv")) {
        int32_t Angle;
//        if(PCmd->GetNext<int32_t>(&Indx) != retvOk) PShell->Ack(retvBadValue);
        if(PCmd->GetNext<int32_t>(&Angle) != retvOk) PShell->Ack(retvBadValue);
        Srv[0]->SetAngle_dg(Angle);
        Srv[1]->SetAngle_dg(Angle);
        Srv[2]->SetAngle_dg(Angle);
        Srv[3]->SetAngle_dg(Angle);
        PShell->Ack(retvOk);
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif
