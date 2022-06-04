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
static bool IsFlickering = false;
static uint8_t OldPeriod = BLINK_PERIOD_MAX_S+1; // No Flicker
static uint16_t ClrH = 0;
static uint16_t Period = BLINK_PERIOD_MAX_S+1;


static LedHSVChunk_t lsqFlicker[] = {
        {csSetup, 99, hsvRed},
        {csSetup, 99, hsvBlack},
        {csEnd}
};

bool CalibrationMode = true;
void ProcessAdc(int32_t *Values);
void ProcessData();
#define CONTROL_CNT         6
#define CALIBRATION_CNT     16
int32_t AdcOffset[CONTROL_CNT];
int32_t CalibrationCounter[CONTROL_CNT];

struct AdcValues_t {
    int32_t Battery;
    int32_t Ch[4];
    int32_t R1, R2;
    int32_t Vref;
} __packed;

static int32_t Offset[6];

bool TxOn = false;

Timer_t SyncTmr(TIM7);
uint16_t GetTimerArr(uint32_t Period);
#endif

int main(void) {
    // ==== Init Clock system ====
    Clk.EnablePrefetch();
    Clk.SetupFlashLatency(12000000);
    Clk.SwitchTo(csHSE);
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
    LedHsv.Init();

    Radio.Init();

    Lcd.Init();
    Lcd.SetBacklight(100);
    Interface.Start();
    Interface.EnterIdle();

    // Buttons
    SimpleSensors::Init();

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
                // Mode
                if(Msg.BtnEvtInfo.BtnID == 2 or Msg.BtnEvtInfo.BtnID == 3) {
                    if(Msg.BtnEvtInfo.BtnID == 2) {
                        if(Mode == modeOff) Mode = modeRandom;
                        else Mode = (Mode_t)((uint8_t)Mode - 1);
                    }
                    else if(Msg.BtnEvtInfo.BtnID == 3) {
                        if(Mode == modeRandom) Mode = modeOff;
                        else Mode = (Mode_t)((uint8_t)Mode + 1);
                    }
                    // Handle synctmr
                    LedHsv.Stop();
                    SyncTmr.SetCounter(0);
                    if(Mode == modeOff) SyncTmr.Disable();
                    else {
                        SyncTmr.Enable();
                        if(Mode != modeRandom) LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 0));
                        EvtQMain.SendNowOrExit(evtIdSyncTmrUpdate);
                    }
                    Interface.ShowMode(Mode);
                }
                else if(Msg.BtnEvtInfo.BtnID == 4) {
                    TxOn = !TxOn;
                    if(TxOn) LedPwr.On();
                    else LedPwr.Off();
                    Interface.ShowTxOnOff(TxOn);
                }
                Lcd.Update();
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

            case evtIdAdcRslt: ProcessAdc((int32_t*)Msg.Ptr); break;

            case evtIdSyncTmrUpdate:
                if(Mode == modeRandom) {
                    int16_t H;
                    int16_t OldH = lsqFlicker[0].Color.H;
                    do {
                        H = Random::Generate(0, 360);
                    } while(ABS(H - OldH) < 36);
                    Interface.ShowColor(H);
                    Lcd.Update();
                    if(IsFlickering) {
                        lsqFlicker[0].Color.FromHSV(H, 100, 100);
                        lsqFlicker[1].Color.FromHSV(H, 100, 0);
                        LedHsv.SetColorAndMakeCurrent(ColorHSV_t(H, 100, 0));
                    }
                    else LedHsv.SetColorAndMakeCurrent(ColorHSV_t(H, 100, 100));
                }
                if(IsFlickering) LedHsv.StartOrRestart(lsqFlicker);  // Restart flicker
                else SyncTmr.Disable();
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
        ClrH = (uint16_t)(360L - (pVal->R2 * 360L) / 4096L);
        Period = (uint8_t)(((BLINK_PERIOD_MAX_S / 2) + 1) - (pVal->R1 * ((BLINK_PERIOD_MAX_S / 2) + 1)) / 4096L) * 2;
        ProcessData();
    }
}

void ProcessData() {
    // Display data
    //Printf("H=%d; R2=%d\r", Pkt.ColorH, Pkt.R2);
    if(Mode != modeRandom) Interface.ShowColor(ClrH);
    Interface.ShowPeriod(Period);
    Lcd.Update();

    // LED
    IsFlickering = (Period <= BLINK_PERIOD_MAX_S);
    switch(Mode) {
        case modeOff: break;

        case modeAsync:
        case modeSync:
        {
            // Color
            lsqFlicker[0].Color.FromHSV(ClrH, 100, 100);
            lsqFlicker[1].Color.FromHSV(ClrH, 100, 0);    // Make it black

            // Period
            if(OldPeriod != Period) {
                OldPeriod = Period;
                // Timer
                SyncTmr.SetCounter(0);
                SyncTmr.SetTopValue(GetTimerArr(Period));
                // lsq
                lsqFlicker[0].Value = Period * 18;
                lsqFlicker[1].Value = lsqFlicker[0].Value;
                // LED
                LedHsv.Stop();
                if(IsFlickering) {
                    LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 0));
                    LedHsv.StartOrContinue(lsqFlicker);
                }
            }
            if(IsFlickering) LedHsv.SetCurrentH(ClrH);
            else LedHsv.SetColorAndMakeCurrent(ColorHSV_t(ClrH, 100, 100));
        }
        break;

        case modeRandom:
            if(OldPeriod != Period) {
                OldPeriod = Period;
                // Timer
                SyncTmr.SetCounter(0);
                SyncTmr.SetTopValue(GetTimerArr(Period));
                LedHsv.Stop();
                if(IsFlickering) {
                    // lsq
                    lsqFlicker[0].Value = Period * 18;
                    lsqFlicker[1].Value = lsqFlicker[0].Value;
                    // LED
                    LedHsv.StartOrContinue(lsqFlicker);
                }
                else {
                    EvtQMain.SendNowOrExit(evtIdSyncTmrUpdate);
                }
            }
            break;
    } // switch

    // Send pkt
    rPkt_t Pkt;
    Pkt.Mode = (uint8_t)Mode;
    Pkt.ColorH = ClrH;
    Pkt.Period = Period;
    Pkt.Time = SyncTmr.GetCounter();
    Radio.RMsgQ.SendNowOrExit(RMsg_t(Pkt));
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
