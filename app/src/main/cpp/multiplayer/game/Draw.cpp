//
// Created on 21.09.2023.
//

#include "Draw.h"
#include "../util/patch.h"

void CDraw::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006779A0 : 0x84D378), &ms_fFarClipZ);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00676178 : 0x84A370), &ms_fNearClipZ);
    //CHook::Write(g_libGTASA + (VER_x32 ? 0x6784C0 : 0x84E9A8), &ms_fAspectRatio);
}
