//
// Created on 15.10.2023.
//

#pragma once

#include "common.h"

// Thanks to Wesser for radar-related things
enum eBlipAppearance : uint8 {
    BLIP_FLAG_FRIEND, // It selects BLIP_COLOUR_BLUE. If unset together with BLIP_FLAG_THREAT, any color.
    BLIP_FLAG_THREAT,  // It selects BLIP_COLOUR_RED. If unset together with BLIP_FLAG_FRIEND, any color.
    BLIP_FLAG_UNK,

    BLIP_FLAG_NUM // Add above this
};

enum eBlipType : uint8 {
    BLIP_NONE,          // 0
    BLIP_CAR,           // 1
    BLIP_CHAR,          // 2
    BLIP_OBJECT,        // 3
    BLIP_COORD,         // 4 - Checkpoint.
    BLIP_CONTACT_POINT, // 5 - Sphere.
    BLIP_SPOTLIGHT,     // 6
    BLIP_PICKUP,        // 7
    BLIP_AIRSTRIP       // 8
};

enum eBlipDisplay : uint8 {
    BLIP_DISPLAY_NEITHER,    // 0
    BLIP_DISPLAY_MARKERONLY, // 1
    BLIP_DISPLAY_BLIPONLY,   // 2
    BLIP_DISPLAY_BOTH        // 3
};

// See <https://www.dropbox.com/s/oi3i4f0qsbe7z10/blip_marker_colors.html> to view these colors. (TODO: dead link, fix it.)
enum eBlipColour : uint32 {
    BLIP_COLOUR_RED,        // 0
    BLIP_COLOUR_GREEN,      // 1
    BLIP_COLOUR_BLUE,       // 2
    BLIP_COLOUR_WHITE,      // 3
    BLIP_COLOUR_YELLOW,     // 4
    BLIP_COLOUR_REDCOPY,    // 5 - What? It was BLIP_COLOUR_PURPLE.
    BLIP_COLOUR_BLUECOPY,   // 6 - Why? It was BLIP_COLOUR_CYAN.
    BLIP_COLOUR_THREAT,     // 7 - If BLIP_FLAG_FRIENDLY is not set (by default) it is BLIP_COLOUR_RED, else BLIP_COLOUR_BLUE.
    BLIP_COLOUR_DESTINATION // 8 - Default color.
};

// https://wiki.multitheftauto.com/index.php?title=Radar_Blips
enum eRadarSprite : int8 {
    RADAR_SPRITE_PLAYER_INTEREST = -5,
    RADAR_SPRITE_THREAT          = -4,
    RADAR_SPRITE_FRIEND          = -3,
    RADAR_SPRITE_OBJECT          = -2,
    RADAR_SPRITE_DESTINATION     = -1,
    RADAR_SPRITE_NONE            = 0,
    RADAR_SPRITE_WHITE,         // 1
    RADAR_SPRITE_CENTRE,        // 2
    RADAR_SPRITE_MAP_HERE,      // 3
    RADAR_SPRITE_NORTH,         // 4
    RADAR_SPRITE_AIRYARD,       // 5
    RADAR_SPRITE_AMMUGUN,       // 6
    RADAR_SPRITE_BARBERS,       // 7
    RADAR_SPRITE_BIGSMOKE,      // 8
    RADAR_SPRITE_BOATYARD,      // 9
    RADAR_SPRITE_BURGERSHOT,    // 10
    RADAR_SPRITE_BULLDOZER,     // 11
    RADAR_SPRITE_CATALINAPINK,  // 12
    RADAR_SPRITE_CESARVIAPANDO, // 13 - What? R* mistype?
    RADAR_SPRITE_CHICKEN,       // 14
    RADAR_SPRITE_CJ,            // 15
    RADAR_SPRITE_CRASH1,        // 16
    RADAR_SPRITE_DINER,         // 17
    RADAR_SPRITE_EMMETGUN,      // 18
    RADAR_SPRITE_ENEMYATTACK,   // 19
    RADAR_SPRITE_FIRE,          // 20
    RADAR_SPRITE_GIRLFRIEND,    // 21
    RADAR_SPRITE_HOSTPITAL,     // 22 - Again?
    RADAR_SPRITE_LOGOSYNDICATE, // 23
    RADAR_SPRITE_MADDOG,        // 24
    RADAR_SPRITE_MAFIACASINO,   // 25
    RADAR_SPRITE_MCSTRAP,       // 26
    RADAR_SPRITE_MODGARAGE,     // 27
    RADAR_SPRITE_OGLOC,         // 28
    RADAR_SPRITE_PIZZA,         // 29
    RADAR_SPRITE_POLICE,        // 30
    RADAR_SPRITE_PROPERTYG,     // 31
    RADAR_SPRITE_PROPERTYR,     // 32
    RADAR_SPRITE_RACE,          // 33
    RADAR_SPRITE_RYDER,         // 34
    RADAR_SPRITE_SAVEGAME,      // 35
    RADAR_SPRITE_SCHOOL,        // 36
    RADAR_SPRITE_QMARK,         // 37
    RADAR_SPRITE_SWEET,         // 38
    RADAR_SPRITE_TATTOO,        // 39
    RADAR_SPRITE_THETRUTH,      // 40
    RADAR_SPRITE_WAYPOINT,      // 41
    RADAR_SPRITE_TORENORANCH,   // 42
    RADAR_SPRITE_TRIADS,        // 43
    RADAR_SPRITE_TRIADSCASINO,  // 44
    RADAR_SPRITE_TSHIRT,        // 45
    RADAR_SPRITE_WOOZIE,        // 46
    RADAR_SPRITE_ZERO,          // 47
    RADAR_SPRITE_DATEDISCO,     // 48
    RADAR_SPRITE_DATEDRINK,     // 49
    RADAR_SPRITE_DATEFOOD,      // 50
    RADAR_SPRITE_TRUCK,         // 51
    RADAR_SPRITE_CASH,          // 52
    RADAR_SPRITE_FLAG,          // 53
    RADAR_SPRITE_GYM,           // 54
    RADAR_SPRITE_IMPOUND,       // 55
    RADAR_SPRITE_LIGHT,         // 56
    RADAR_SPRITE_RUNWAY,        // 57
    RADAR_SPRITE_GANGB,         // 58
    RADAR_SPRITE_GANGP,         // 59
    RADAR_SPRITE_GANGY,         // 60
    RADAR_SPRITE_GANGN,         // 61
    RADAR_SPRITE_GANGG,         // 62
    RADAR_SPRITE_SPRAY,         // 63
    RADAR_SPRITE_TORENO         // 64
};

enum eRadarTraceHeight : uint8 {
    RADAR_TRACE_LOW,   // 0 Up-pointing Triangle ?
    RADAR_TRACE_HIGH,  // 1 Down-pointing Triangle ?
    RADAR_TRACE_NORMAL // 2 Box ?
};

/*
struct tBlipHandle {
    uint16 arrayIndex;
    uint16 number;
};
VALIDATE_SIZE(tBlipHandle, 0x4);
*/

using tBlipHandle = uint32; // TODO: Use struct above

class CRadar {

public:
    static void DrawLegend(int32 x, int32 y, eRadarSprite blipType);
    static float LimitRadarPoint(CVector2D& point);
    static void LimitToMap(float* x, float* y);
    static uint8 CalculateBlipAlpha(float distance);
    static void TransformRadarPointToScreenSpace(CVector2D& out, const CVector2D& in);
    static void TransformRealWorldPointToRadarSpace(CVector2D& out, const CVector2D& in);

    static void DrawCoordBlip(int32 blipIndex, bool isSprite);
    static void DrawEntityBlip(int32 blipIndex, uint8 arg1);
    static void ClearActualBlip(int32 blipIndex);
//    static void ClearActualBlip(tRadarTrace& trace);
//    static void ClearBlipForEntity(eBlipType blipType, int32 entityHandle);
//    static void ClearBlipForEntity(CPedGta* ped);
    static void ClearBlip(tBlipHandle blip);
    static void SetupAirstripBlips();
    static void DrawBlips();
};
