/*
 * lcd_images.h
 *
 *  Created on: 2 ����. 2017 �.
 *      Author: Kreyl
 */

#pragma once

// ===== BatteryFull =====
const uint8_t icon_BatteryFull [] = { 14, 1,
        0x38,0x7C,0x82,0xBA,0xBA,0xBA,0xBA,0xBA,0xBA,0xBA,0xBA,0xBA,0x82,0x7C,
};
// ===== BatteryHalf =====
const uint8_t icon_BatteryHalf [] = { 14, 1,
        0x38,0x7C,0x82,0x82,0x82,0x82,0x8A,0x9A,0xBA,0xBA,0xBA,0xBA,0x82,0x7C,
};
// ===== BatteryEmpty =====
const uint8_t icon_BatteryEmpty [] = { 14, 1,
        0x38,0x7C,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x7C,
};

// Radio 0
const uint8_t icon_Radio0[] = { 11, 1,
        0x80,0x40,0xFE,0x40,0x82,0x00,0x02,0x00,0x02,0x00,0x02,
};
// Radio 1
const uint8_t icon_Radio1[] = { 11, 1,
        0x80,0x40,0xFE,0x40,0x86,0x00,0x02,0x00,0x02,0x00,0x02,
};
// Radio 2
const uint8_t icon_Radio2[] = { 11, 1,
        0x80,0x40,0xFE,0x40,0x86,0x00,0x0E,0x00,0x02,0x00,0x02,
};
// Radio 3
const uint8_t icon_Radio3[] = { 11, 1,
        0x80,0x40,0xFE,0x40,0x86,0x00,0x0E,0x00,0x1E,0x00,0x02,
};
// Radio 4
const uint8_t icon_Radio4[] = { 11, 1,
        0x80,0x40,0xFE,0x40,0x86,0x00,0x0E,0x00,0x1E,0x00,0x3E,
};
