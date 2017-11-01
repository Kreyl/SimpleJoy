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
#define R_WIDTH     5
#define R1_X        72
#define R2_X        78

class Interface_t {
public:
    void Reset() {
        Lcd.Print(0, 0, "Время:  00:00");
        Lcd.Update();
    }

    void DrawR1(uint8_t Value) {}
    void DrawR2(uint8_t Value) {
        for(int y = Y0; y < (Y0 + R_HEIGHT); y++) {
            Lcd.DrawPixel(R2_X, y, Inverted);
            Lcd.DrawPixel((R2_X+R_WIDTH), y, Inverted);
            if(y == Y0 or y == (Y0 + R_HEIGHT -1)) {

            }
            else {

            }
        }
    }

    void Error(const char* msg) { Lcd.PrintInverted(0, 0, "%S", msg); }
};

extern Interface_t Interface;
