//
// Created on 24.04.2024.
//

#include "WeaponInfo.h"

CWeaponInfo* CWeaponInfo::GetWeaponInfo(int iWeapon, int iSkill)
{
    // CWeaponInfo::GetWeaponInfo
    return ((CWeaponInfo*(*)(int, int))(g_libGTASA + (VER_x32 ? 0x005E42E8 + 1 : 0x709BA8)))(iWeapon, iSkill);
}