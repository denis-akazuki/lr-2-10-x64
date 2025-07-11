//
// Created on 20.09.2023.
//

#ifndef RUSSIA_WEATHER_H
#define RUSSIA_WEATHER_H


#include "common.h"

enum eWeatherRegion : int16 {
    WEATHER_REGION_DEFAULT = 0,
    WEATHER_REGION_LA = 1,
    WEATHER_REGION_SF = 2,
    WEATHER_REGION_LV = 3,
    WEATHER_REGION_DESERT = 4
};

class CWeather {
public:
    static inline float     Wind;
    static inline CVector   WindDir;
    static inline float     Foggyness_SF;
    static inline float     Foggyness;
    static inline float     CloudCoverage;
    static inline float     Rainbow;
    static inline float     ExtraSunnyness;

public:
    static void InjectHooks();
};


#endif //RUSSIA_WEATHER_H
