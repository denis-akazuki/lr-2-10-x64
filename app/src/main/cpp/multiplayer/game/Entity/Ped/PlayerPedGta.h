/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Ped.h"


class CPlayerPedGta : public CPedGta {
public:
    CPedGta* m_p3rdPersonMouseTarget;
    int32 field_7A0;

    // did we display "JCK_HLP" message
    static bool& bHasDisplayedPlayerQuitEnterCarHelpText;

    // Android
    static bool bDebugPlayerInvincible;
    static bool bDebugTargeting;
    static bool bDebugTapToTarget;

public:
    static void InjectHooks();

    CPlayerPedGta(int32 playerId, bool bGroupCreated);

    bool Load_Reversed();
    bool Save_Reversed();

//    void ProcessControl() override;
//    bool Load() override;
//    bool Save() override;

  //  CPad* GetPadFromPlayer() const;
    bool CanPlayerStartMission();
    bool IsHidden();
    void ReApplyMoveAnims();
};

VALIDATE_SIZE(CPlayerPedGta, (VER_x32 ? 0x7AC : 0x998));


