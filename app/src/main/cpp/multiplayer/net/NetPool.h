//
// Created on 29.01.2024.
//

#pragma once

#include "game/common.h"


template<class T>
class CNetPool {
public:
    static inline std::unordered_map<uint32, T> list {};

public:
    static T GetAt(uint32 id)
    {
        auto it = list.find(id);
        return it != list.end() ? it->second : nullptr;
    }

    static std::vector<uint32> GetAllIds()
    {
        std::vector<uint32> keysToDelete;

        for (auto& element : list) {
            keysToDelete.push_back(element.first);
        }

        return keysToDelete;
    }
};
