//
// Created on 10.11.2023.
//

#include "CrossHair.h"
#include "common.h"
#include "../util/util.h"
#include "../main.h"
#include "../util/patch.h"
#include "../net/netgame.h"
#include "game.h"
#include "Entity/Ped/PlayerPedGta.h"

void CCrossHair::Init()
{
    Log("CCrossHair::Init");
    pCircleTex = new CSprite2d();
    pCircleTex->m_pTexture = CUtil::LoadTextureFromDB("txd", "siteM16");
}

bool CCrossHair::IsCircleCrosshairMode(eCamMode mode)
{
    return mode == MODE_AIMWEAPON ||
           mode == MODE_DW_CAM_MAN ||
           (mode & 0xFFFD) == MODE_AIMWEAPON_ATTACHED;
}

void CCrossHair::Render()
{
    const auto pPed = GamePool_FindPlayerPed();
    if (!pPed)
        return;

    const auto camMode = static_cast<eCamMode>(GameGetLocalPlayerCameraMode());
    if (IsCircleCrosshairMode(camMode))
    {
        static float fCHairScreenMultX = (RsGlobal->maximumWidth - (RsGlobal->maximumHeight / 9 * 16)) / 2 + ((RsGlobal->maximumHeight / 9 * 16) * 0.524);
        static float fFixedOffset = RsGlobal->maximumWidth / (RsGlobal->maximumWidth - (RsGlobal->maximumHeight / 9 * 16)) * 2.0;
        static auto gunRadius = reinterpret_cast<CPlayerPedGta*>(pPed)->GetWeaponRadiusOnScreen();
        static float fCHairScreenMultY = (RsGlobal->maximumHeight / 9 * 16) / 10 * 6 * 0.4 + fFixedOffset;

        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, RWRSTATE(rwFILTERLINEAR));
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));

        float f1 = ((RsGlobal->maximumHeight / 448.0) * 64.0) * gunRadius;

        float fPosX1 = ((f1 * 0.5) + fCHairScreenMultX) - f1;
        float fPosY1 = ((f1 * 0.5) + fCHairScreenMultY) - f1;

        float fPosX2 = (f1 * 0.5) + fPosX1;
        float fPosY2 = (f1 * 0.5) + fPosY1;

        pCircleTex->Draw(CRect{fPosX1, fPosY1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1 + f1, fPosY1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1, fPosY1 + f1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1 + f1, fPosY1 + f1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
    }
}