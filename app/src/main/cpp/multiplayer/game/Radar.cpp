//
// Created on 15.10.2023.
//

#include "Radar.h"
#include "../util/patch.h"

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
