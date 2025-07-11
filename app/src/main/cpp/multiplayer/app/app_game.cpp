//
// Created on 23.09.2023.
//

#include "app_game.h"
#include "../game/Birds.h"
#include "../game/Skidmarks.h"
#include "../main.h"
#include "../game/Coronas.h"
#include "../game/Clouds.h"
#include "../game/Glass.h"
#include "../game/MovingThings.h"
#include "../game/VisibilityPlugins.h"
#include "../game/WaterLevel.h"
#include "../game/WaterCannons.h"
#include "../game/WeaponEffects.h"
#include "../game/SpecialFX.h"
#include "../game/PointLights.h"
#include "../game/PostEffects.h"
#include "game/Camera.h"
#include "util/patch.h"
#include "keyboard.h"
#include "HUD.h"
#include "game/Widgets/TouchInterface.h"
#include "game/CrossHair.h"
#include "tools/DebugModules.h"

void RenderEffects() {
//	RenderEffects();
    CBirds::Render();
    CSkidmarks::Render();
//    CRopes::Render();
//    CGlass::Render();
    CMovingThings::Render();
    CVisibilityPlugins::RenderReallyDrawLastObjects();
    CCoronas::Render();

    // FIXME
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    auto g_fx = *(uintptr_t *) (g_libGTASA + (VER_x32 ? 0x00820520 : 0xA062A8));
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00363DF0 + 1 : 0x433F54), &g_fx, TheCamera.m_pRwCamera, false);

    CWaterCannons::Render();
    CWaterLevel::RenderWaterFog();
    CClouds::MovingFogRender();
 //   CClouds::VolumetricCloudsRender();
////    if (CHeli::NumberOfSearchLights || CTheScripts::NumberOfScriptSearchLights) {
////        CHeli::Pre_SearchLightCone();
////        CHeli::RenderAllHeliSearchLights();
////        CTheScripts::RenderAllSearchLights();
////        CHeli::Post_SearchLightCone();
////    }
    CWeaponEffects::Render();
////    if (CReplay::Mode != MODE_PLAYBACK && !CPad::GetPad(0)->DisablePlayerControls) {
////        FindPlayerPed()->DrawTriangleForMouseRecruitPed();
////    }
    CSpecialFX::Render();
//    //CVehicleRecording::Render();
    CPointLights::RenderFogEffect();
//    //CRenderer::RenderFirstPersonVehicle();
    CPostEffects::MobileRender();

    DebugModules::Render3D();
}

void Render2dStuff()
{
    if( CHook::CallFunction<bool>(g_libGTASA + (VER_x32 ? 0x001BB7F4 + 1 : 0x24EA90)) ) // emu_IsAltRenderTarget()
        CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x001BC20C + 1 : 0x24F5B8)); // emu_FlushAltRenderTarget()

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, RWRSTATE(rwRENDERSTATENARENDERSTATE));
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLNONE));

    CGUI::Render();

    CCrossHair::Render();

    if (CHUD::bIsShow) {

        // radar
        auto radar = CTouchInterface::m_pWidgets[WIDGET_RADAR];

        if (radar)
        {

            radar->m_fOriginX = CHUD::radarPos.x;
            radar->m_fOriginY = CHUD::radarPos.y;

            radar->m_fScaleX = CHUD::radarSize.x;
            radar->m_fScaleY = CHUD::radarSize.y;
        }

        ((void (*)()) (g_libGTASA + (VER_x32 ? 0x00437B0C + 1 : 0x51CFF0)))(); // CHud::DrawRadar
        //	GPS::Draw();

        //
        if(!CKeyBoard::m_bEnable)
            ( ( void(*)(bool) )(g_libGTASA + (VER_x32 ? 0x002B0BD8 + 1 : 0x36FB00)) )(false); // CTouchInterface::DrawAll
    }

    ((void (*)(bool)) (g_libGTASA + (VER_x32 ? 0x0054BDD4 + 1 : 0x66B678)))(1u); // CMessages::Display - gametext
    ((void (*)(bool)) (g_libGTASA + (VER_x32 ? 0x005A9120 + 1 : 0x6CCEA0)))(1u); // CFont::RenderFontBuffer
}