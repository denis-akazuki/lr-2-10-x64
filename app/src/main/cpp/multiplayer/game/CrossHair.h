//
// Created on 10.11.2023.
//

#pragma once

#include "RW/RenderWare.h"
#include "sprite2d.h"
#include "Camera.h"

class CCrossHair {
public:
    static inline CSprite2d* pCircleTex;

public:
    static void Init();
    static void Render();
    static bool IsCircleCrosshairMode(eCamMode mode);
};
