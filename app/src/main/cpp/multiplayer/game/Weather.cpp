//
// Created on 20.09.2023.
//

#include "Weather.h"
#include "../util/patch.h"

void CWeather::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x0067616C : 0x84A358), &Wind);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00678C44 : 0x84F8B8), &WindDir);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679788 : 0x850F28), &Foggyness);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679A18 : 0x851450), &CloudCoverage);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006764E0 : 0x84AA30), &Rainbow);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00678378 : 0x84E718), &ExtraSunnyness);
}
