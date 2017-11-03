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
#include "kl_servo.h"
#include "radio_lvl1.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
void OnCmd(Shell_t *PShell);
void ITask();

static const PinInputSetup_t DipSwPin[DIP_SW_CNT] = { DIP_SW8, DIP_SW7, DIP_SW6, DIP_SW5, DIP_SW4, DIP_SW3, DIP_SW2, DIP_SW1 };
static uint8_t GetDipSwitch();
static void OnRadioRx();

LedBlinker_t LedPwr {LED_PWR};
LedBlinker_t LedLink {LED_LINK};

// Servo
#define SRV_CNT     6
static const Servo_t Srv1{SRV1_PIN};
static const Servo_t Srv2{SRV2_PIN};
static const Servo_t Srv3{SRV3_PIN};
static const Servo_t Srv4{SRV4_PIN};
static const Servo_t Srv5{SRV5_PIN};
static const Servo_t Srv6{SRV6_PIN};
static const Servo_t* Srv[SRV_CNT] = { &Srv1, &Srv2, &Srv3, &Srv4, &Srv5, &Srv6 };

// PWM
#define PWM_CNT     6
static const PinOutputPWM_t Pwm1(PWM1_PIN);
static const PinOutputPWM_t Pwm2(PWM2_PIN);
static const PinOutputPWM_t Pwm3(PWM3_PIN);
static const PinOutputPWM_t Pwm4(PWM4_PIN);
static const PinOutputPWM_t Pwm5(PWM5_PIN);
static const PinOutputPWM_t Pwm6(PWM6_PIN);
static const PinOutputPWM_t *Pwm[PWM_CNT] = { &Pwm1, &Pwm2, &Pwm3, &Pwm4, &Pwm5, &Pwm6 };

// Motor
#define MOTOR_CNT   4
enum Motordir_t { mdirForward, mdirBackward };
class Motor_t {
private:
    const PinOutput_t IDirPin;
    const PinOutputPWM_t *IPwmPin;
public:
    void Init() const {
        IDirPin.Init();
    }
    void Set(Motordir_t Dir, uint8_t Value) const {
        if(Dir == mdirForward) IDirPin.SetHi();
        else IDirPin.SetLo();
        IPwmPin->Set(Value);
    }
    Motor_t(GPIO_TypeDef *APGPIO, uint16_t APin, const PinOutputPWM_t *PPwm) :
        IDirPin(APGPIO, APin, omPushPull), IPwmPin(PPwm) {}
};

static const Motor_t Motor1{DIR1_PIN, &Pwm1};
static const Motor_t Motor2{DIR2_PIN, &Pwm2};
static const Motor_t Motor3{DIR3_PIN, &Pwm3};
static const Motor_t Motor4{DIR4_PIN, &Pwm4};
static const Motor_t *Motor[MOTOR_CNT] = { &Motor1, &Motor2, &Motor3, &Motor4 };

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

    // LEDs
    LedPwr.Init();
    LedLink.Init();
    LedPwr.On();
//    LedLink.StartOrRestart(lbsqBlink1s);

    // Servo
    for(int i=0; i<SRV_CNT; i++) {
        Srv[i]->Init();
        Srv[i]->SetAngle_dg(90);
    }

    // Pwm
    for(int i=0; i<PWM_CNT; i++) {
        Pwm[i]->Init();
        Pwm[i]->SetFrequencyHz(3600);
        Pwm[i]->Set(81);
    }

    // Motors
    for(int i=0; i<MOTOR_CNT; i++) {
        Motor[i]->Init();
    }

    if(Radio.Init() == retvOk) LedLink.StartOrRestart(lbsqBlink1s);
    else LedLink.StartOrRestart(lbsqFailure);

    TmrEverySecond.StartOrRestart();

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

            case evtIdRadioRx:
                OnRadioRx();
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

            case evtIdAdcRslt: {
                } break;

            default: break;
        } // switch
    } // while true
} // ITask()

void OnRadioRx() {
    rPkt_t Pkt = Radio.PktRx;
//    Pkt.Print();
    // Servo
    for(int i=0; i<4; i++) {
        Srv[i]->SetValue(Pkt.Ch[i], -127L, 127L);
    }
    Srv[4]->SetValue(255-Pkt.R1, 0, 255);
    Srv[5]->SetValue(255-Pkt.R2, 0, 255);
    // Motors
    for(int i=0; i<4; i++) {
        Motordir_t Dir = (Pkt.Ch[i] > 0)? mdirForward : mdirBackward;
        uint32_t v = 2 * ABS(Pkt.Ch[i]);
        if(v > 255) v = 255;
        Motor[i]->Set(Dir, v);
    }
    // PWM
    Pwm5.Set(Pkt.R1);
    Pwm6.Set(Pkt.R2);
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
