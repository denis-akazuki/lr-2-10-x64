//
// Created on 10.11.2023.
//

#ifndef RUSSIA_CROSSHAIR_H
#define RUSSIA_CROSSHAIR_H


#include "RW/RenderWare.h"
#include "sprite2d.h"

class CCrossHair {
public:
    static inline CSprite2d* pCircleTex;
    static inline bool m_UsedCrossHair{};

public:
    static void Init();
    static void Render();
};


#endif //RUSSIA_CROSSHAIR_H
