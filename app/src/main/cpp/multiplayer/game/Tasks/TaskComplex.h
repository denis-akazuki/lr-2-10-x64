/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Task.h"

class CPedGta;

class CTaskComplex : public CTask {
public:
    CTask* m_pSubTask;

public:
    CTaskComplex();
    ~CTaskComplex() override;

    CTask* GetSubTask() override;
    bool IsSimple() override;
    bool MakeAbortable(CPedGta* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;

    virtual void SetSubTask(CTask* subTask);
    virtual CTask* CreateNextSubTask(CPedGta* ped) = 0;
    virtual CTask* CreateFirstSubTask(CPedGta* ped) = 0;
    virtual CTask* ControlSubTask(CPedGta* ped) = 0;
    // #vtable: 11
};
VALIDATE_SIZE(CTaskComplex, (VER_x32 ? 0xC : 0x18));
