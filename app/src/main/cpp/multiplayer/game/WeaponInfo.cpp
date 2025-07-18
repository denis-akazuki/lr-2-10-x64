//
// Created on 24.04.2024.
//

#include "WeaponInfo.h"
#include "util/patch.h"
#include "Models/ModelInfo.h"
#include "game/Entity/Ped/Ped.h"
#include "CFileMgr.h"
#include "FileLoader.h"
#include "Animation/AnimManager.h"
#include "Enums/eMeleeCombo.h"

CWeaponInfo* CWeaponInfo::GetWeaponInfo(eWeaponType weaponID, eWeaponSkill skill) {
    return &aWeaponInfo[GetWeaponInfoIndex(weaponID, skill)];
}

uint32 CWeaponInfo::GetWeaponInfoIndex(eWeaponType wt, eWeaponSkill skill) {
    assert(TypeIsWeapon(wt) || (skill == eWeaponSkill::STD && (wt >= WEAPON_RAMMEDBYCAR && wt <= WEAPON_FLARE))); // Damage events also have their weapon info entries

    const auto GetNonSTDSkillLevelIndex = [wt](uint32 i) {
        assert(TypeHasSkillStats(wt));
        return WEAPON_LAST_WEAPON           // Offset for std skill level
               + wt - FIRST_WEAPON_WITH_SKILLS // Offset for this weapon [relative to the other weapons with skill levels]
               + i * NUM_WEAPONS_WITH_SKILL;   // Offset for skill level
    };

    switch (skill) {
        case eWeaponSkill::POOR: return GetNonSTDSkillLevelIndex(0);
        case eWeaponSkill::STD:  return (uint32)wt;
        case eWeaponSkill::PRO:  return GetNonSTDSkillLevelIndex(1);
        case eWeaponSkill::COP:  return GetNonSTDSkillLevelIndex(2);
        default:                 DLOG("Invalid weapon skill");
    }
}

bool CWeaponInfo::TypeHasSkillStats(eWeaponType wt) {
    return wt >= FIRST_WEAPON_WITH_SKILLS && wt <= LAST_WEAPON_WITH_SKILLS;
}

bool CWeaponInfo::TypeIsWeapon(eWeaponType type) {
    return type < WEAPON_LAST_WEAPON;
}

auto CWeaponInfo::GetWeaponInfo(CPedGta* ped) {
    return GetWeaponInfo(ped->GetActiveWeapon().m_nType, ped->GetWeaponSkill());
}

AnimationId CWeaponInfo::GetCrouchReloadAnimationID() const {
    return flags.bCrouchFire && flags.bReload ? ANIM_ID_CROUCHRELOAD : ANIM_ID_WALK;
}

uint32 CWeaponInfo::GetWeaponReloadTime() const {
    if (flags.bReload)
        return flags.bTwinPistol ? 2000u : 1000u;

    if (flags.bLongReload)
        return 1000u;

    const auto& ao = g_GunAimingOffsets[m_nAimOffsetIndex];
    return std::max(400u, (uint32)std::max({ ao.RLoadA, ao.RLoadB, ao.CrouchRLoadA, ao.CrouchRLoadB }) + 100);
}

eStats CWeaponInfo::GetSkillStatIndex(eWeaponType wt) {
    switch (wt) {
        case WEAPON_PISTOL:          return STAT_PISTOL_SKILL;
        case WEAPON_PISTOL_SILENCED: return STAT_SILENCED_PISTOL_SKILL;
        case WEAPON_DESERT_EAGLE:    return STAT_DESERT_EAGLE_SKILL;
        case WEAPON_SHOTGUN:         return STAT_SHOTGUN_SKILL;
        case WEAPON_SAWNOFF_SHOTGUN: return STAT_SAWN_OFF_SHOTGUN_SKILL;
        case WEAPON_SPAS12_SHOTGUN:  return STAT_COMBAT_SHOTGUN_SKILL;
        case WEAPON_MICRO_UZI:       return STAT_MACHINE_PISTOL_SKILL;
        case WEAPON_MP5:             return STAT_SMG_SKILL;
        case WEAPON_AK47:            return STAT_AK_47_SKILL;
        case WEAPON_M4:              return STAT_M4_SKILL;
        case WEAPON_TEC9:            return STAT_MACHINE_PISTOL_SKILL;
    }
}

void CWeaponInfo::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00678ECC : 0x84FDC8), &aWeaponInfo);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006790A0 : 0x850170), &g_GunAimingOffsets);
}