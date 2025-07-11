//
// Created on 16.09.2023.
//

#pragma once

#include "Vehicle.h"

class CAutomobile : public CVehicleGta {
#if VER_x32
    uint8 pad0[0x3E8];
#else
    uint8 pad0[0x470];
#endif

public:
    bool BreakTowLink();
};

VALIDATE_SIZE(CAutomobile, (VER_x32 ? 0x99C : 0xBC8));
