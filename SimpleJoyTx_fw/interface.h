/*
 * interface.h
 *
 *  Created on: 22 марта 2015 г.
 *      Author: Kreyl
 */

#pragma once

#include "lcd5110.h"

#define Y0          8
#define R_HEIGHT    40
#define R_WIDTH     4
#define R_COEF      (255 / R_HEIGHT)
#define R1_X        0
#define R2_X        79
#define J1_X        7
#define J2_X        36
#define J_HEIGHT    32
#define J_WIDTH     4
#define J_COEF      (255 / J_HEIGHT)

class Interface_t {
public:
    void Reset() {
        Lcd.Print(0, 0, "Канал: 00");
        Lcd.Update();
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

    void DrawJy(uint8_t x, int8_t Value) {
        for(int y = Y0; y < (Y0 + J_HEIGHT); y++) {
            Lcd.DrawPixel(x, y, Inverted);
            Lcd.DrawPixel((x+J_WIDTH), y, Inverted);
            int Threshold = (J_HEIGHT / 2) - Value / R_COEF;
            if(     (y < (Y0 + J_HEIGHT / 2) and Value > 0 and y > (Y0 + Threshold)) or // Top half
                    (y > (Y0 + J_HEIGHT / 2) and Value < 0 and y < (Y0 + Threshold)) or // Bottom half
                    y == Y0 or y == (Y0 + J_HEIGHT - 1) or y == (Y0 + J_HEIGHT / 2)) {
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

    void Error(const char* msg) { Lcd.PrintInverted(0, 0, "%S", msg); }
};

extern Interface_t Interface;
