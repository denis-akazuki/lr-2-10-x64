//
// Created on 23.09.2023.
//

#include "PointLights.h"
#include "../util/patch.h"

void CPointLights::RenderFogEffect() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005B19D0 + 1 : 0x6D6068));
}
