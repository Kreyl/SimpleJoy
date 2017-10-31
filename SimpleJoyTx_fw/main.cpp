/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
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
#include "radio_lvl1.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
void OnCmd(Shell_t *PShell);
void ITask();

static const PinInputSetup_t DipSwPin[DIP_SW_CNT] = { DIP_SW8, DIP_SW7, DIP_SW6, DIP_SW5, DIP_SW4, DIP_SW3, DIP_SW2, DIP_SW1 };
static uint8_t GetDipSwitch();

LedBlinker_t LedPwr {LED_PWR};
LedBlinker_t LedLink {LED_LINK};

bool CalibrationMode = true;
void ProcessAdc(int32_t *Values);
#define CONTROL_CNT         6
#define CALIBRATION_CNT     16
int32_t AdcOffset[CONTROL_CNT];
int32_t CalibrationCounter[CONTROL_CNT];

TmrKL_t TmrEverySecond {MS2ST(999), evtIdEverySecond, tktPeriodic};

struct AdcValues_t {
    int32_t Battery;
    int32_t Ch[6];
    int32_t Vref;
} __packed;

int32_t Offset[6];

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

    // LEDs
    LedPwr.Init();
    LedLink.Init();
    LedPwr.On();
//    LedLink.StartOrRestart(lbsqBlink1s);

    if(Radio.Init() == retvOk) LedLink.StartOrRestart(lbsqBlink1s);
    else LedLink.StartOrRestart(lbsqFailure);

    TmrEverySecond.StartOrRestart();

    // ==== Adc ====
    PinSetupOut(BAT_MEAS_EN, omPushPull);
    PinSetHi(BAT_MEAS_EN);  // Enable battery measurement
    PinSetupAnalog(BAT_MEAS_PIN);
    PinSetupAnalog(ADC0_PIN);
    PinSetupAnalog(ADC1_PIN);
    PinSetupAnalog(ADC2_PIN);
    PinSetupAnalog(ADC3_PIN);
    PinSetupAnalog(ADC4_PIN);
    PinSetupAnalog(ADC5_PIN);
    memset(CalibrationCounter, 0, sizeof(CalibrationCounter));
    memset(AdcOffset, 0, sizeof(AdcOffset));
    memset(Offset, 0, sizeof(Offset));
    Adc.Init();
    Adc.EnableVRef();

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

            case evtIdEverySecond: {
                static int OldDip = -1;
                int dip = GetDipSwitch();
                if(dip != OldDip) {
                    OldDip = dip;
                    Printf("Dip: %u\r", dip);
                    Radio.SetChannel(dip);
                }
            } break;

            case evtIdAdcRslt:
//                Printf("ID: %u; V: %u\r", Msg.ValueID, Msg.Value);
                ProcessAdc((int32_t*)Msg.Ptr);
                break;

            default: break;
        } // switch
    } // while true
} // ITask()

void ProcessAdc(int32_t *Values) {
    AdcValues_t *pVal = (AdcValues_t*)Values;
    // Battery
//    int32_t VBat_mv = Adc.Adc2mV(pVal->Battery, pVal->Vref);
//    Printf("Battery: %u\r", VBat_mv);

//    Printf("%u\r", pVal->Ch[0]);
    if(CalibrationMode) {
        static int CalibrationCnt = 0;
        for(int i=0; i<6; i++) {
            Offset[i] += pVal->Ch[i];
        }
        CalibrationCnt++;
        if(CalibrationCnt == 8) {   // 8, why not? Put here something else if you want.
            for(int i=0; i<6; i++) Offset[i] /= 8;
            CalibrationMode = false;
            Printf("Calibration done\r");
        }
    }
    // Not calibration
    else {
        rPkt_t Pkt;
        for(int i=0; i<6; i++) {
            int32_t v = pVal->Ch[i]; // To make things shorter
            v -= Offset[i];
            v /= 16L;  // [0...4095] => [0...255]
            if(v < -128L) v = -128L;
            if(v > 127L) v = 127L;
            Pkt.Ch[i] = v;
        }
        Pkt.Print();
    }
}



// ====== DIP switch ======
uint8_t GetDipSwitch() {
    uint8_t Rslt = 0;
    for(int i=0; i<DIP_SW_CNT; i++) PinSetupInput(DipSwPin[i].PGpio, DipSwPin[i].Pin, DipSwPin[i].PullUpDown);
    for(int i=0; i<DIP_SW_CNT; i++) {
        if(!PinIsHi(DipSwPin[i].PGpio, DipSwPin[i].Pin)) Rslt |= (1 << i);
        PinSetupAnalog(DipSwPin[i].PGpio, DipSwPin[i].Pin);
    }
    return Rslt;
}

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