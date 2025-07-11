#pragma once
#include "font.h"
#include "Cam.h"
#include <string>

class CFirstPersonCamera
{
    static bool m_bEnabled;
public:
    static void ProcessCameraOnFoot(CCam* pCam, CPedSamp* pPed);
    static void ProcessCameraInVeh(CCam* pCam, CPedSamp* pPed, CVehicle* pVeh);

    static void SetEnabled(bool bEnabled);
    static void Toggle();
    static bool IsEnabled();
};
