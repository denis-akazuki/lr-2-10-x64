//
// Created on 15.10.2023.
//

#include "Radar.h"
#include "../util/patch.h"
#include "World.h"
#include "net/netgame.h"
#include "Widgets/TouchInterface.h"
#include "game.h"
#include "Mobile/MobileMenu/MobileMenu.h"

tBlipHandle CRadar::SetCoordBlip(eBlipType type, CVector posn, eBlipColour color, eBlipDisplay blipDisplay, const char* scriptName) {
    if (pNetGame && !strncmp(scriptName, "CODEWAY", 7)) {
        float findZ = CWorld::FindGroundZForCoord(posn.x, posn.y);
        findZ += 1.5f;

        RakNet::BitStream bsSend;
        bsSend.Write(posn.x);
        bsSend.Write(posn.y);
        bsSend.Write(findZ);
        pNetGame->GetRakClient()->RPC(&RPC_MapMarker, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
    }

    auto index = FindTraceNotTrackingBlipIndex();
    if (index == -1)
        return -1;

    auto& t = ms_RadarTrace[index];

    t.m_vPosition        = posn;
    t.m_nBlipDisplayFlag = blipDisplay;
    t.m_nBlipType        = type;
    t.m_nColour          = BLIP_COLOUR_DESTINATION;
    t.m_fSphereRadius    = 1.f;
    t.m_nEntityHandle    = 0;
    t.m_nBlipSize        = 1;
    t.m_nBlipSprite      = eRadarSprite::RADAR_SPRITE_NONE;
    t.m_bBright          = true;
    t.m_bTrackingBlip    = true;
    t.m_pEntryExit       = nullptr;

    return GetNewUniqueBlipIndex(index);
}

tBlipHandle CRadar::GetNewUniqueBlipIndex(int32 index) {
    auto& trace = ms_RadarTrace[index];
    if (trace.m_nCounter >= std::numeric_limits<uint16>::max() - 1)
        trace.m_nCounter = 1; // Wrap back to 1
    else
        trace.m_nCounter++; // Increment
    return index | (trace.m_nCounter << 16);
}

int32 CRadar::FindTraceNotTrackingBlipIndex() {
    for (size_t i = 0; i < ms_RadarTrace.size(); ++i) {
        if (!ms_RadarTrace[i].m_bTrackingBlip) {
            return static_cast<int32>(i);
        }
    }
    return -1;
}

/*
 * @brief Clear a blip
 */
void CRadar::ClearBlip(tBlipHandle blip) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x004429E8 + 1 : 0x527C60), blip);
}

/*!
 * @brief Transforms a real coordinate to radar coordinate.
 */
void CRadar::TransformRealWorldPointToRadarSpace(CVector2D& out, const CVector2D& in) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0043F6E4 + 1 : 0x524AD0), out, in);
}

/*!
 * @brief Transforms a radar point to screen.
 * @brief Unhooked by default for now. Causes `DrawRadarSection` to crash.
 */
void CRadar::TransformRadarPointToScreenSpace(CVector2D& out, const CVector2D& in) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0043F62C + 1 : 0x524A30), out, in);
}

/*!
 * @brief Limits a 2D vector to the radar. (which is a unit circle)
 * @brief This function does not effect the vector if the map is being drawn.
 * @param point The vector to be limited.
 * @returns Magnitude of the vector before limiting.
 */
float CRadar::LimitRadarPoint(CVector2D &point) {
    return CHook::CallFunction<float>(g_libGTASA + (VER_x32 ? 0x0043F760 + 1 : 0x524B2C), point);
}

void CRadar::LimitToMap(float* x, float* y) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x004424E4 + 1 : 0x5277EC), x, y);
}

bool CRadar::DisplayThisBlip(eRadarSprite spriteId, int8 priority) {
    return CGame::currArea == AREA_CODE_NORMAL_WORLD && !FindPlayerPed(-1)->m_nAreaCode
           || spriteId <= 0x34 && ((1LL << spriteId) & 0x1012100200001FLL) != 0;
}

void CRadar::LimitToMap(float& x, float& y) {
    const auto mobileMenu = CMobileMenu::GetMobileMenu();
    const auto zoom = (mobileMenu->screenStack.numEntries || mobileMenu->pendingScreen)
                      ? mobileMenu->NEW_MAP_SCALE
                      : 140.0f;

    {
        const auto min = SCREEN_STRETCH_X(mobileMenu->MAP_OFFSET_X - zoom);
        const auto max = SCREEN_STRETCH_X(mobileMenu->MAP_OFFSET_X + zoom);
        x = std::clamp(x, min, max);
    }

    {
        const auto min = SCREEN_STRETCH_Y(mobileMenu->MAP_OFFSET_Y - zoom);
        const auto max = SCREEN_STRETCH_Y(mobileMenu->MAP_OFFSET_Y + zoom);
        y = std::clamp(y, min, max);
    }
}

void Limit(float& x, float& y) {
    const auto mobileMenu = CMobileMenu::GetMobileMenu();
    if (mobileMenu->DisplayingMap) {
        x = SCREEN_STRETCH_Y(x);
        y = SCREEN_STRETCH_Y(y);

        CRadar::LimitToMap(x, y);
    }
}

void CRadar::DrawRadarSprite(eRadarSprite spriteId, float x, float y, uint8 alpha) {
    Limit(x, y);

    const auto widget = CTouchInterface::m_pWidgets[WidgetIDs::WIDGET_RADAR];
    float size = widget ? fabsf(widget->m_RectScreen.right - widget->m_RectScreen.left) * 0.1f : 10.0f;

    if (DisplayThisBlip(spriteId, -99)) {
        RadarBlipSprites[(size_t)spriteId].Draw(
                CRect{ x - size, y - size, x + size, y + size },
                { 255, 255, 255, alpha }
        );
    }
}

CVector tRadarTrace::GetWorldPos() const {
    CVector pos = { m_vPosition };
    if (m_pEntryExit) {
        CHook::CallFunction<void>("_ZN10CEntryExit33GetPositionRelativeToOutsideWorldER7CVector", m_pEntryExit, &pos);
    }
    return pos;
}

std::pair<CVector2D, CVector2D> tRadarTrace::GetRadarAndScreenPos(float* radarPointDist) const {
    const auto world = GetWorldPos();
    auto radar = CRadar::TransformRealWorldPointToRadarSpace(world);
    const auto dist = CRadar::LimitRadarPoint(radar);
    if (radarPointDist) {
        *radarPointDist = dist;
    }

    const auto screen = CRadar::TransformRadarPointToScreenSpace(radar);
    return std::make_pair(radar, screen);
}

CVector2D CRadar::TransformRealWorldPointToRadarSpace(const CVector2D& in) {
    CVector2D diff = in - vec2DRadarOrigin;
    CVector2D scaled(diff.x / m_radarRange, diff.y / m_radarRange);
    return CachedRotateClockwise(scaled);
}

CVector2D CRadar::TransformRadarPointToScreenSpace(const CVector2D& in) {
    auto mobileMenu = CMobileMenu::GetMobileMenu();
    if (mobileMenu->DisplayingMap) {
        return {
                mobileMenu->MAP_OFFSET_X + mobileMenu->NEW_MAP_SCALE * in.x,
                mobileMenu->MAP_OFFSET_Y - mobileMenu->NEW_MAP_SCALE * in.y
        };
    } else {
        return {
                SCREEN_STRETCH_X(94.0f) / 2.0f + SCREEN_STRETCH_X(40.0f) + SCREEN_STRETCH_X(94.0f * in.x) / 2.0f,
                SCREEN_STRETCH_FROM_BOTTOM(104.0f) + SCREEN_STRETCH_Y(76.0f) / 2.0f - SCREEN_STRETCH_Y(76.0f * in.y) / 2.0f
        };
    }
}

void CRadar::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x6773C4 : 0x84C7D0), &ms_RadarTrace);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x6776F0 : 0x84CE18), &RadarBlipSprites);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x676E00 : 0x84BC60), &m_radarRange);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x677E3C : 0x84DCA8), &vec2DRadarOrigin);

    CHook::Redirect("_ZN6CRadar12SetCoordBlipE9eBlipType7CVectorj12eBlipDisplayPc", &SetCoordBlip);
    CHook::Redirect("_ZN6CRadar15DrawRadarSpriteEtffh", &DrawRadarSprite);
}
