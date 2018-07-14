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
#include "SimpleSensors.h"
#include "lcd5110.h"
#include "interface.h"
#include "battery_consts.h"
#include "usb_cdc.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
void OnCmd(Shell_t *PShell);
void ITask();

LedBlinker_t LedPwr {LED_PWR};
LedHSV_t LedHsv {LED_R_PIN, LED_G_PIN, LED_B_PIN};

Interface_t Interface;
Mode_t Mode = modeOff;

static uint8_t OldPeriod = BLINK_PERIOD_MAX_S+1; // No Flicker

bool CalibrationMode = true;
void ProcessAdc(int32_t *Values);
void ProcessData();
#define CONTROL_CNT         6
#define CALIBRATION_CNT     16
int32_t AdcOffset[CONTROL_CNT];
int32_t CalibrationCounter[CONTROL_CNT];

TmrKL_t TmrEverySecond {MS2ST(999), evtIdEverySecond, tktPeriodic};
bool RPktReceived = false;

struct AdcValues_t {
    int32_t Battery;
    int32_t Ch[4];
    int32_t R1, R2;
    int32_t Vref;
} __packed;

static int32_t Offset[6];
static rPkt_t Pkt;

static LedHSVChunk_t lsqFlicker[] = {
        {csSetup, 99, hsvRed},
        {csSetup, 99, hsvBlack},
        {csEnd}
};

Timer_t SyncTmr(TIM7);
uint16_t GetTimerArr(uint32_t Period);
#endif

int main(void) {
    // ==== Init Clock system ====
    Clk.EnablePrefetch();
//    Clk.SetupFlashLatency(48000000);
//    Clk.SwitchTo(csHSI48);
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();

    // ==== Init hardware ====
    EvtQMain.Init();
    Uart.Init();
    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();
    Uart.StartRx();

    // LEDs
    LedPwr.Init();
    LedPwr.On();
    LedHsv.Init();
//    LedHsv.SetupSeqEndEvt(evtIdLedDone);

//    Radio.Init();

    Lcd.Init();
    Lcd.SetBacklight(100);
    Interface.Start();
    Interface.EnterIdle();

    // Buttons
    SimpleSensors::Init();

//    TmrEverySecond.StartOrRestart();

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

//    UsbCDC.Init();

    // Setup sync timer
    SyncTmr.Init();
    SyncTmr.SetTopValue(63000);
    SyncTmr.SetupPrescaler(1000);
    SyncTmr.EnableIrqOnUpdate();
    SyncTmr.EnableIrq(TIM7_IRQn, IRQ_PRIO_LOW);
    SyncTmr.Enable();

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdUsbNewCmd:
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdButtons:
//                Printf("Btn %u\r", Msg.BtnEvtInfo.BtnID);
                if(Msg.BtnEvtInfo.BtnID == 2) {
                    if(Mode == modeOff) {
                        Mode = modeRandom;
                        LedHsv.Stop();
                        EvtQMain.SendNowOrExit(evtIdLedDone);
                    }
                    else Mode = (Mode_t)((uint8_t)Mode - 1);
                }
                else if(Msg.BtnEvtInfo.BtnID == 3) {
                    if(Mode == modeRandom) Mode = modeOff;
                    else {
                        Mode = (Mode_t)((uint8_t)Mode + 1);
                        OldPeriod = BLINK_PERIOD_MAX_S+1;   // Restart LED
                    }
                    if(Mode == modeRandom) {
                        LedHsv.Stop();
                        EvtQMain.SendNowOrExit(evtIdLedDone);
                    }
                }
                Interface.ShowMode(Mode);
                Lcd.Update();
                break;

            case evtIdEverySecond:
//                Printf("%u\r", (TIM2->CNT % 1000));
                // Radio: reset rx level if nothing received
//                if(RPktReceived) RPktReceived = false;
//                else {
//                    Interface.ShowRadioLvl(0);
//                    OldRadioLvl = 255;
//                }
                break;

#if 0 // ======= USB =======
            case evtIdUsbConnect:
                Printf("USB connect\r");
                Clk.EnableCRS();
                Clk.SelectUSBClock_HSI48();
                UsbCDC.Connect();
                break;
            case evtIdUsbDisconnect:
                Printf("USB disconnect\r");
                UsbCDC.Disconnect();
                Clk.DisableCRS();
                break;
            case evtIdUsbReady:
                Printf("USB ready\r");
                break;
#endif

            case evtIdAdcRslt:
//                Printf("ID: %u; V: %u\r", Msg.ValueID, Msg.Value);
                ProcessAdc((int32_t*)Msg.Ptr);
                break;

//            case evtIdLedDone:
//                Printf("Tmr: %u\r", SyncTmr.GetCounter());
//                SyncTmr.SetCounter(0);
//                break;

            case evtIdSyncTmrUpdate:
//                Printf("Aga\r");
                if(Pkt.Period <= BLINK_PERIOD_MAX_S) {
                    if(Mode == modeRandom) {
                        int16_t H;
                        while(true) {
                            H = Random::Generate(0, 360);
                            int16_t OldH = lsqFlicker[0].Color.H;
                            if(ABS(H - OldH) > 36) break;
                        }
                        Interface.ShowColor(H);
                        Lcd.Update();
                        lsqFlicker[0].Color.FromHSV(H, 100, 100);
                        lsqFlicker[1].Color.FromHSV(H, 100, 0);
                        LedHsv.SetColorAndMakeCurrent(ColorHSV_t(H, 100, 0));
                    }
                    LedHsv.StartOrRestart(lsqFlicker);  // Restart flicker
                }
                break;

            default: break;
        } // switch
    } // while true
} // ITask()

void ProcessUsbDetect(PinSnsState_t *PState, uint32_t Len) {
    EvtMsg_t Msg;
    if(*PState == pssRising) Msg.ID = evtIdUsbConnect;
    else if(*PState == pssFalling) Msg.ID = evtIdUsbDisconnect;
    EvtQMain.SendNowOrExit(Msg);
}

void ProcessAdc(int32_t *Values) {
    AdcValues_t *pVal = (AdcValues_t*)Values;
    // Battery
    int32_t VBat_mv = 2 * Adc.Adc2mV(pVal->Battery, pVal->Vref); // *2 because of divider
    static BatteryState_t OldBatState = bsNone;
    BatteryState_t CurrState;
    if(VBat_mv > 4100) CurrState = bsFull;
    else if(VBat_mv > 3400) CurrState = bsHalf;
    else CurrState = bsEmpty;
    if(CurrState != OldBatState) {
        OldBatState = CurrState;
        Interface.ShowBattery(CurrState);
    }
//    Printf("Battery: %u\r", VBat_mv);

    if(CalibrationMode) {
        static int CalibrationCnt = 0;
        for(int i=0; i<4; i++) {
            Offset[i] += pVal->Ch[i];
        }
        CalibrationCnt++;
        if(CalibrationCnt == 8) {   // 8, why not? Put here something else if you want.
            for(int i=0; i<4; i++) Offset[i] /= 8;
            CalibrationMode = false;
            Printf("Calibration done\r");
        }
    }
    // Not calibration
    else {
        Pkt.ColorH = (uint16_t)(360L - (pVal->R2 * 360L) / 4096L);
        Pkt.Period = (uint8_t)((BLINK_PERIOD_MAX_S + 1) - (pVal->R1 * (BLINK_PERIOD_MAX_S + 1)) / 4096L);
        ProcessData();
    }
}

void ProcessData() {
    // Send pkt

    // Display data
    //Printf("H=%d; R2=%d\r", Pkt.ColorH, Pkt.R2);
    if(Mode != modeRandom) Interface.ShowColor(Pkt.ColorH);
    Interface.ShowPeriod(Pkt.Period);
    Lcd.Update();

    // LED
    switch(Mode) {
        case modeOff: LedHsv.Stop(); break;

        case modeAsync:
        case modeSync:
        {
            // Color
            lsqFlicker[0].Color.FromHSV(Pkt.ColorH, 100, 100);
            lsqFlicker[1].Color.FromHSV(Pkt.ColorH, 100, 0);    // Make it black
            // Period
            if(OldPeriod != Pkt.Period) {
                OldPeriod = Pkt.Period;
                // Timer
                SyncTmr.SetCounter(0);
                SyncTmr.SetTopValue(GetTimerArr(Pkt.Period));
                // lsq
                lsqFlicker[0].Value = Pkt.Period * 18;
                lsqFlicker[1].Value = lsqFlicker[0].Value;
                // LED
                LedHsv.Stop();
                if(Pkt.Period <= BLINK_PERIOD_MAX_S) {
                    LedHsv.SetColorAndMakeCurrent(ColorHSV_t(Pkt.ColorH, 100, 0));
                    LedHsv.StartOrRestart(lsqFlicker);
                }
                else LedHsv.SetColorAndMakeCurrent(ColorHSV_t(Pkt.ColorH, 100, 100));
            }
        }
        break;

        case modeRandom:
            if(OldPeriod != Pkt.Period) {
                OldPeriod = Pkt.Period;
                // Timer
                SyncTmr.SetCounter(0);
                SyncTmr.SetTopValue(GetTimerArr(Pkt.Period));
                // lsq
                lsqFlicker[0].Value = Pkt.Period * 18;
                lsqFlicker[1].Value = lsqFlicker[0].Value;
                // LED
                LedHsv.Stop();
                if(Pkt.Period <= BLINK_PERIOD_MAX_S) LedHsv.StartOrRestart(lsqFlicker);
            }
            break;
    } // switch
}

uint16_t GetTimerArr(uint32_t Period) {
    if     (Period >= 80) return Period * 114 + 11;
    else if(Period >= 60) return Period * 115 + 11;
    else if(Period >= 40) return Period * 116 + 11;
    else if(Period >= 30) return Period * 117 + 11;
    else if(Period >= 20) return Period * 119 + 11;
    else if(Period >= 15) return Period * 121 + 11;
    else if(Period >= 10) return Period * 125 + 11;
    else if(Period >= 7) return Period * 130 + 11;
    else if(Period >= 4) return Period * 155 + 11;
    else if(Period == 3) return 534;
    else if(Period == 2) return 469;
    else return 412;
}

#if 1 // ================= Command processing ====================
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

// IRQ
extern "C"
void Vector88() {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    SyncTmr.ClearIrqPendingBit();
    EvtQMain.SendNowOrExitI(EvtMsg_t(evtIdSyncTmrUpdate));
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
