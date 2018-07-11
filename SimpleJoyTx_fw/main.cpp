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

Interface_t Interface;

bool CalibrationMode = true;
void ProcessAdc(int32_t *Values);
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
static uint8_t OldRadioLvl = 255;
static rPkt_t Pkt;
#endif

int main(void) {
    // ==== Init Clock system ====
    Clk.EnablePrefetch();
    Clk.SetupFlashLatency(48000000);
    Clk.SwitchTo(csHSI48);
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

    Radio.Init();

    Lcd.Init();
    Lcd.SetBacklight(100);
    Interface.Start();
    Interface.EnterIdle();

    // Buttons
    SimpleSensors::Init();

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

    UsbCDC.Init();

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
//              Printf("BtnType %u; N=%u; Btns %u %u\r", Msg.BtnEvtInfo.Type, Msg.BtnEvtInfo.BtnCnt, Msg.BtnEvtInfo.BtnID[0], Msg.BtnEvtInfo.BtnID[1]);
                if(Msg.BtnEvtInfo.BtnID == 4) { // "L" btn
                    if(Lcd.GetBacklight() == 0) Lcd.SetBacklight(100);
                    else Lcd.SetBacklight(0);
                }
                break;

//            case evtIdRadioRx: {
////                Printf("Rx: %d\r", Msg.Value);
//                RPktReceived = true;
//                uint8_t Lvl = 0;
//                if(Msg.Value > -35) Lvl = 4;
//                else if(Msg.Value > -65) Lvl = 3;
//                else if(Msg.Value > -85) Lvl = 2;
//                else Lvl = 1;
//                if(Lvl != OldRadioLvl) {
//                    OldRadioLvl = Lvl;
//                    Interface.ShowRadioLvl(Lvl);
//                }
//                // ADC values received
//                if(AdcOn and UsbCDC.IsActive()) {
//                    UsbCDC.Printf("%u;%u;%u;%u\r\n",
//                        Radio.rPktReply.Adc[0], Radio.rPktReply.Adc[1],
//                        Radio.rPktReply.Adc[2], Radio.rPktReply.Adc[3]);
//                }
//            } break;

            case evtIdEverySecond:
                // Radio: reset rx level if nothing received
//                if(RPktReceived) RPktReceived = false;
//                else {
//                    Interface.ShowRadioLvl(0);
//                    OldRadioLvl = 255;
//                }
                break;

#if 1 // ======= USB =======
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
        // Put adc values to pkt
        for(int i=0; i<4; i++) {
            int32_t v = pVal->Ch[i]; // To make things shorter
            v -= Offset[i];
            v /= 16L;  // [0...4095] => [0...255]
            if(v < -128L) v = -127L;
            if(v > 127L) v = 127L;
            Pkt.Ch[i] = v;
        }
        // Correct sign
        Pkt.Ch[0] = -Pkt.Ch[0];
        Pkt.Ch[3] = -Pkt.Ch[3];
        Pkt.ColorH = (uint16_t)(360L - (pVal->R2 * 360L) / 4096L);
        Pkt.R2 = (uint16_t)(360L - (pVal->R1 * 360L) / 4096L);

        // Add buttons
        uint8_t b = 0;
        for(int i=0; i<7; i++) {
            if(GetBtnState(i) == pssLo) b |= 1<<i;
        }
        Pkt.Btns = b;
    }
    if(!CalibrationMode) {
        // Display data
        //Printf("H=%d; R2=%d\r", Pkt.ColorH, Pkt.R2);
        Interface.ShowColor(Pkt.ColorH);
        Lcd.Update();
        //SendData(Pkt);
    }
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
