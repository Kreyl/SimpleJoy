#include "hal.h"
#include "board.h"
#include "MsgQ.h"
#include "shell.h"
#include "kl_lib.h"
#include "led.h"
#include "Sequences.h"
#include "kl_servo.h"
#include "radio_lvl1.h"
#include "IntelLedEffs.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
void OnCmd(Shell_t *PShell);
void ITask();

static void CloseDoors();
static void OpenDoors();

static const NeopixelParams_t LedParams(NPX1_SPI, NPX1_GPIO, NPX1_PIN, NPX1_AF, NPX1_DMA, NPX_DMA_MODE(NPX1_DMA_CHNL));
Neopixels_t Npx(&LedParams);
Effects_t Leds(&Npx);

#define MIN_OPENED_DURATION_S   11
int32_t OpenedTimeout = 0;

// Servo
#define SRV_CNT     3
static const Servo_t Srv1{SRV1_PIN, 700, 2300};
static const Servo_t Srv2{SRV2_PIN, 700, 2300};
//static const Servo_t Srv3{SRV3_PIN, 700, 2200};
static const Servo_t Srv4{SRV4_PIN, 700, 2200};
static const Servo_t Srv5{SRV5_PIN};
static const Servo_t Srv6{SRV6_PIN};
static const Servo_t* Srv[SRV_CNT] = { &Srv1, &Srv2, /*&Srv3,*/ &Srv4};

#define CLOSED_ANGLE    168
#define OPENED_ANGLE    63
uint32_t CurrAngle;

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
    Npx.Init();
    CommonEffectsInit();

    // Servo
    for(int i=0; i<SRV_CNT; i++) {
        Srv[i]->Init();
        Srv[i]->SetAngle_dg(CLOSED_ANGLE);
        chThdSleepMilliseconds(540);
    }
    CurrAngle = CLOSED_ANGLE;

    if(Radio.Init() == retvOk) {
        Leds.SeqAllTogetherStartOrRestart(lsqIdle);
    }
    else {
        Leds.SeqAllTogetherStartOrRestart(lsqFailure);
        chThdSleepMilliseconds(1008);
    }

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

            case evtIdRadioNoone:
                Leds.SeqAllTogetherStartOrContinue(lsqIdle);
                CloseDoors();
                break;

            case evtIdRadioLowPwr:
                Leds.SeqAllTogetherStartOrContinue(lsqLowPower);
                CloseDoors();
                break;

            case evtIdRadioHiPwrCrystal:
                Leds.SeqAllTogetherStartOrContinue(lsqHiPower);
                CloseDoors();
                break;

            case evtIdRadioHiPwrKey:
                Leds.SeqAllTogetherStartOrContinue(lsqHiPowerKey);
                OpenDoors();
                break;

            case evtIdEverySecond:
                if(OpenedTimeout > 0) OpenedTimeout--;
                break;

            default: break;
        } // switch
    } // while true
} // ITask()

void SetupAngle(uint32_t DesiredAngle, uint32_t Delay_ms) {
    while(CurrAngle != DesiredAngle) {
        // Adjust angle
        if(CurrAngle > DesiredAngle) CurrAngle--;
        else CurrAngle++;
        // Setup angle
        for(int i=0; i<SRV_CNT; i++) {
            Srv[i]->SetAngle_dg(CurrAngle);
            chThdSleepMilliseconds(Delay_ms);
        }
    }
}

void CloseDoors() {
    if(OpenedTimeout > 0) return; // Do not close too fast
    SetupAngle(CLOSED_ANGLE, 9);
}

void OpenDoors() {
    SetupAngle(OPENED_ANGLE, 9);
    OpenedTimeout = MIN_OPENED_DURATION_S;
}


#if 1 // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }

    else if(PCmd->NameIs("Srv")) {
        int32_t Angle = 0, Indx = 0;
        if(PCmd->GetNext<int32_t>(&Indx) != retvOk) PShell->Ack(retvBadValue);
        if(PCmd->GetNext<int32_t>(&Angle) != retvOk) PShell->Ack(retvBadValue);
        Srv[Indx]->SetAngle_dg(Angle);
//        Srv[1]->SetAngle_dg(Angle);
//        Srv[2]->SetAngle_dg(Angle);
//        Srv[3]->SetAngle_dg(Angle);
        PShell->Ack(retvOk);
    }

    else if(PCmd->NameIs("c")) {
        CloseDoors();
    }
    else if(PCmd->NameIs("o")) {
        OpenDoors();
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif
