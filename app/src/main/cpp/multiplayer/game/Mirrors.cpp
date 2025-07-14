//
// Created by traw-GG on 14.07.2025.
//
#include "Mirrors.h"
#include "game.h"
#include "net/netgame.h"
#include "Camera.h"
#include "Scene.h"
#include "TimeCycle.h"
#include "app/app.h"
#include "World.h"
#include "Mobile/MobileSettings/MobileSettings.h"

void CMirrors::RenderReflections()
{
    auto carReflections = CMobileSettings::ms_MobileSettings[MS_CarReflections].value;
    if (carReflections < 2)
        return;

    if (pNetGame) {
        auto pPed = CPlayerPool::GetLocalPlayer()->GetPlayerPed();
        if (pPed && pPed->m_pPed) {
            pPed->m_pPed->m_bIsVisible = false;
        }
        for (const auto& pair : CPlayerPool::spawnedPlayers) {
            const auto pPed = pair.second->GetPlayerPed();
            if (pPed && pPed->m_pPed && !pPed->m_pPed->m_bRemoveFromWorld) {
                pPed->m_pPed->m_bIsVisible = false;
            }
        }
        for (auto* pEntity : CObject::objectToIdMap) {
            if (pEntity && !pEntity->m_bRemoveFromWorld) {
                pEntity->m_bIsVisible = false;
            }
        }
    }

    CHook::CallFunction<void>("_ZN8CMirrors12CreateBufferEv");
    if (!FindPlayerPed(-1) || !reflBuffer[0])
        return;

    static float reflectionRadius = 0.0f;
    static bool radiusInitialized = false;

    if (!radiusInitialized) {
        reflectionRadius = 60.0f;
        radiusInitialized = true;
    }

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    TheCamera.m_sphereMapRadius = reflectionRadius * reflectionRadius;

    auto originalMirrorType = TypeOfMirror;
    TypeOfMirror = MIRROR_TYPE_SPHERE_MAP;

    RwRaster* originalFrameBuffer = Scene.m_pRwCamera->frameBuffer;
    RwRaster* originalZBuffer = Scene.m_pRwCamera->zBuffer;
    const float originalFarPlane = Scene.m_pRwCamera->farPlane;
    const float originalFogPlane = Scene.m_pRwCamera->fogPlane;

    Scene.m_pRwCamera->frameBuffer = reflBuffer[0];
    Scene.m_pRwCamera->zBuffer = reflBuffer[1];

    CHook::CallFunction<void>("_Z22emu_SetRenderingSpherePfh", nullptr, 1);

    RwRGBA skyColor(
            std::max<uint16_t>(CTimeCycle::m_CurrentColours.m_nSkyTopRed, 64u),
            std::max<uint16_t>(CTimeCycle::m_CurrentColours.m_nSkyTopGreen, 64u),
            std::max<uint16_t>(CTimeCycle::m_CurrentColours.m_nSkyTopBlue, 64u),
            255
    );

    RwCameraClear(Scene.m_pRwCamera, &skyColor, rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if (CHook::CallFunction<RwBool>("_Z19RsCameraBeginUpdateP8RwCamera", Scene.m_pRwCamera))
    {
        Scene.m_pRwCamera->farPlane = reflectionRadius;
        Scene.m_pRwCamera->fogPlane = reflectionRadius * 0.75f;

        CVector reflectionPos;
        if (*(bool*)(g_libGTASA + (VER_x32 ? 0x9421C5 : 0xBA8925))) {
            reflectionPos = TheCamera.GetPosition();
        } else {
            FindPlayerPed(-1)->GetBonePosition(&reflectionPos, BONE_NECK, false);
            reflectionPos.z -= 2.0f;
        }

        bRenderingReflection = true;
        CHook::CallFunction<void>("_Z21emu_SetCameraPositionPf", &reflectionPos.x);
        DefinedState();

        CHook::CallFunction<void>("_ZN9CRenderer23ConstructReflectionListEv");
        CHook::CallFunction<void>("_Z11RenderSceneb", false);

        bRenderingReflection = false;
        RwCameraEndUpdate(Scene.m_pRwCamera);
    }

    CHook::CallFunction<void>("_Z22emu_SetRenderingSpherePfh", nullptr, 0);
    Scene.m_pRwCamera->frameBuffer = originalFrameBuffer;
    Scene.m_pRwCamera->zBuffer = originalZBuffer;
    TypeOfMirror = originalMirrorType;

    TheCamera.m_sphereMapRadius = 0.0f;
    Scene.m_pRwCamera->farPlane = originalFarPlane;
    Scene.m_pRwCamera->fogPlane = originalFogPlane;

    if (pNetGame) {
        auto pPed = CPlayerPool::GetLocalPlayer()->GetPlayerPed();
        if (pPed && pPed->m_pPed) {
            pPed->m_pPed->m_bIsVisible = true;
        }
        for (const auto& pair : CPlayerPool::spawnedPlayers) {
            const auto pPed = pair.second->GetPlayerPed();
            if (pPed && pPed->m_pPed && !pPed->m_pPed->m_bRemoveFromWorld) {
                pPed->m_pPed->m_bIsVisible = true;
            }
        }
        for (auto* pEntity : CObject::objectToIdMap) {
            if (pEntity && !pEntity->m_bRemoveFromWorld) {
                pEntity->m_bIsVisible = true;
            }
        }
    }
}

void CMirrors::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x679504 : 0x850A28), &reflBuffer);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x677EB8 : 0x84DDA0), &TypeOfMirror);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x6763E8 : 0x84A848), &bRenderingReflection);

    CHook::Redirect("_ZN8CMirrors17RenderReflectionsEv", &RenderReflections);
}