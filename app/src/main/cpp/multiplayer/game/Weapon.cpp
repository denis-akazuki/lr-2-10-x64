//
// Created on 07.04.2023.
//

#include "Weapon.h"
#include "game/Entity/Ped/Ped.h"

void CWeapon::Reload(CPedGta* owner) {
    if (!m_nTotalAmmo)
        return;

  //  uint32 ammo = GetWeaponInfo(owner).m_nAmmoClip;
   // m_nAmmoInClip = std::min(ammo, m_nTotalAmmo);
}


// 0x73B1E0
bool CWeapon::IsType2Handed() {
    switch (m_nType) {
        case eWeaponType::WEAPON_M4:
        case eWeaponType::WEAPON_AK47:
        case eWeaponType::WEAPON_SPAS12_SHOTGUN:
        case eWeaponType::WEAPON_SHOTGUN:
        case eWeaponType::WEAPON_SNIPERRIFLE:
        case eWeaponType::WEAPON_FLAMETHROWER:
        case eWeaponType::WEAPON_COUNTRYRIFLE:
            return true;
    }

    return false;
}

// 0x73B210
bool CWeapon::IsTypeProjectile() {
    switch (m_nType) {
        case eWeaponType::WEAPON_GRENADE:
        case eWeaponType::WEAPON_REMOTE_SATCHEL_CHARGE:
        case eWeaponType::WEAPON_TEARGAS:
        case eWeaponType::WEAPON_MOLOTOV:
        case eWeaponType::WEAPON_FREEFALL_BOMB:
            return true;
    }

    return false;
}

bool CWeapon::HasWeaponAmmoToBeUsed() {
    switch (m_nType) {
        case eWeaponType::WEAPON_UNARMED:
        case eWeaponType::WEAPON_BRASSKNUCKLE:
        case eWeaponType::WEAPON_GOLFCLUB:
        case eWeaponType::WEAPON_NIGHTSTICK:
        case eWeaponType::WEAPON_KNIFE:
        case eWeaponType::WEAPON_BASEBALLBAT:
        case eWeaponType::WEAPON_KATANA:
        case eWeaponType::WEAPON_CHAINSAW:
        case eWeaponType::WEAPON_DILDO1:
        case eWeaponType::WEAPON_DILDO2:
        case eWeaponType::WEAPON_VIBE1:
        case eWeaponType::WEAPON_VIBE2:
        case eWeaponType::WEAPON_FLOWERS:
        case eWeaponType::WEAPON_PARACHUTE:
            return true;
    }

    return m_nTotalAmmo != 0;
}

bool CWeapon::IsFiredWeapon() const { // огнестрельное?
    switch (m_nType) {
        case eWeaponType::WEAPON_UNARMED:
        case eWeaponType::WEAPON_BRASSKNUCKLE:
        case eWeaponType::WEAPON_GOLFCLUB:
        case eWeaponType::WEAPON_NIGHTSTICK:
        case eWeaponType::WEAPON_KNIFE:
        case eWeaponType::WEAPON_BASEBALLBAT:
        case eWeaponType::WEAPON_KATANA:
        case eWeaponType::WEAPON_CHAINSAW:
        case eWeaponType::WEAPON_DILDO1:
        case eWeaponType::WEAPON_DILDO2:
        case eWeaponType::WEAPON_VIBE1:
        case eWeaponType::WEAPON_VIBE2:
        case eWeaponType::WEAPON_FLOWERS:
        case eWeaponType::WEAPON_PARACHUTE:
            return false;
    }

    return true;
}