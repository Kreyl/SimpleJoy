/*
 * interface.h
 *
 *  Created on: 22 марта 2015 г.
 *      Author: Kreyl
 */

#pragma once

#include "lcd5110.h"
#include "lcd_images.h"
#include "battery_consts.h"
#include "main.h"

#define Y0          8
#define R_HEIGHT    40
#define R_WIDTH     4
#define R_COEF      (255 / R_HEIGHT)
#define R1_X        0
#define R2_X        79
#define J1_X        7
#define J2_X        44
#define J_Y         40
#define J_HEIGHTy   32
#define J_WIDTHy    4
#define J_COEFy     (255 / J_HEIGHTy)
#define J_WIDTHx    28
#define J_HEIGHTx   4
#define J_COEFx     (255 / J_WIDTHx)

class Interface_t {
public:
    void Start() {
        Lcd.Cls();
    }

    void EnterIdle() {
        Lcd.Print(0, 1, "Режим: выкл");
        Lcd.Print(0, 2, "Цвет: 000");
        Lcd.Print(0, 3, "Мерцание: выкл");
        Lcd.Update();
    }

    void ShowMode(Mode_t AMode) {
        switch(AMode) {
            case modeOff:    Lcd.Print(0, 1, "Режим: выкл   "); break;
            case modeSync:   Lcd.Print(0, 1, "Режим: синхр  "); break;
            case modeAsync:  Lcd.Print(0, 1, "Режим: асинхр "); break;
            case modeRandom: Lcd.Print(0, 1, "Режим: случайн"); break;
        }
    }

    void ShowColor(uint16_t AColorH) { Lcd.Print(6, 2, "%03u ", AColorH); }

    void ShowPeriod(uint8_t APeriodS) {
        if(APeriodS > BLINK_PERIOD_MAX_S) Lcd.Print(0, 3, "Мерцание: выкл");
        else Lcd.Print(0, 3, "Мерцание: %u   ", APeriodS);
    }

    void DrawR(uint8_t x, uint8_t Value) {
        for(int y = Y0; y < (Y0 + R_HEIGHT); y++) {
            Lcd.DrawPixel(x, y, Inverted);
            Lcd.DrawPixel((x+R_WIDTH), y, Inverted);
            int Threshold = R_HEIGHT - Value / R_COEF;
            if(y == Y0 or y > (Y0 + Threshold) or y == (Y0 + R_HEIGHT - 1)) {
                Lcd.DrawPixel((x+1), y, Inverted);
                Lcd.DrawPixel((x+2), y, Inverted);
                Lcd.DrawPixel((x+3), y, Inverted);
            }
            else {
                Lcd.DrawPixel((x+1), y, NotInverted);
                Lcd.DrawPixel((x+2), y, NotInverted);
                Lcd.DrawPixel((x+3), y, NotInverted);
            }
        } // for
    }

    void DrawJy(uint8_t x0, int8_t Value) {
        int Threshold = (J_HEIGHTy / 2) - Value / J_COEFy;
        for(int y = Y0; y < (Y0 + J_HEIGHTy); y++) {
            Lcd.DrawPixel(x0, y, Inverted);
            Lcd.DrawPixel((x0+J_WIDTHy), y, Inverted);
            if(     (y < (Y0 + J_HEIGHTy / 2) and Value > 0 and y > (Y0 + Threshold)) or // Top half
                    (y > (Y0 + J_HEIGHTy / 2) and Value < 0 and y < (Y0 + Threshold)) or // Bottom half
                    y == Y0 or y == (Y0 + J_HEIGHTy - 1) or y == (Y0 + J_HEIGHTy / 2)) {
                Lcd.DrawPixel((x0+1), y, Inverted);
                Lcd.DrawPixel((x0+2), y, Inverted);
                Lcd.DrawPixel((x0+3), y, Inverted);
            }
            else {
                Lcd.DrawPixel((x0+1), y, NotInverted);
                Lcd.DrawPixel((x0+2), y, NotInverted);
                Lcd.DrawPixel((x0+3), y, NotInverted);
            }
        } // for
    }
    void DrawJx(uint8_t x0, uint8_t y0, int8_t Value) {
        int Threshold = (J_WIDTHx / 2) + Value / J_COEFx;
        for(int x = x0; x < (x0 + J_WIDTHx); x++) {
            Lcd.DrawPixel(x, y0, Inverted);
            Lcd.DrawPixel(x, (y0 + J_HEIGHTx), Inverted);
            if(     (x < (x0 + J_WIDTHx / 2) and Value < 0 and x > (x0 + Threshold)) or
                    (x > (x0 + J_WIDTHx / 2) and Value > 0 and x < (x0 + Threshold)) or
                    x == x0 or x == (x0 + J_WIDTHx / 2) or x == (x0 + J_WIDTHx - 1)) {
                Lcd.DrawPixel(x, (y0+1), Inverted);
                Lcd.DrawPixel(x, (y0+2), Inverted);
                Lcd.DrawPixel(x, (y0+3), Inverted);
            }
            else {
                Lcd.DrawPixel(x, (y0+1), NotInverted);
                Lcd.DrawPixel(x, (y0+2), NotInverted);
                Lcd.DrawPixel(x, (y0+3), NotInverted);
            }
        } // for
    }

    void ShowBattery(BatteryState_t BatteryState) {
        switch (BatteryState) {
            case bsFull:  Lcd.DrawImage(70, 0, icon_BatteryFull);  break;
            case bsHalf:  Lcd.DrawImage(70, 0, icon_BatteryHalf);  break;
            case bsEmpty: Lcd.DrawImage(70, 0, icon_BatteryEmpty); break;
            default: break;
        }
    }

    void ShowRadioLvl(uint8_t Lvl) {
        switch(Lvl) {
            case 1: Lcd.DrawImage(54, 0, icon_Radio1); break;
            case 2: Lcd.DrawImage(54, 0, icon_Radio2); break;
            case 3: Lcd.DrawImage(54, 0, icon_Radio3); break;
            case 4: Lcd.DrawImage(54, 0, icon_Radio4); break;
            default: Lcd.DrawImage(54, 0, icon_Radio0); break;
        }
    }

    void Error(const char* msg) { Lcd.PrintInverted(0, 0, "%S", msg); }
};

extern Interface_t Interface;
