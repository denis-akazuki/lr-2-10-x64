/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Core/Vector.h"

struct CPedGta;
struct CVehicleGta;
struct CTask;

class CCarEnterExit {
public:
//    static const float& ms_fMaxSpeed_CanDragPedOut;
//    static const float& ms_fMaxSpeed_PlayerCanDragPedOut;
//
//    static inline bool& ms_bPedOffsetsCalculated = *(bool*)0xC18C20;
//    static inline CVector& ms_veCPedGtaGetUpAnimOffset = *(CVector*)0xC18C3C;
//    static inline CVector& ms_veCPedGtaBedLAnimOffset = *(CVector*)0xC18C54;
//    static inline CVector& ms_veCPedGtaBedRAnimOffset = *(CVector*)0xC18C60;
//    static inline CVector& ms_veCPedGtaDeskAnimOffset = *(CVector*)0xC18C6C;
//    static inline CVector& ms_veCPedGtaChairAnimOffset = *(CVector*)0xC18C78;
//    static inline CVector& ms_veCPedGtaQuickDraggedOutCarAnimOffset = *(CVector*)0xC18C48;

public:
    static void InjectHooks();

    static void AddInCarAnim(const CVehicleGta* vehicle, CPedGta* ped, bool bAsDriver);
    static bool CarHasDoorToClose(const CVehicleGta* vehicle, int32 doorId);
    static bool CarHasDoorToOpen(const CVehicleGta* vehicle, int32 doorId);
    static bool CarHasOpenableDoor(const CVehicleGta* vehicle, int32 doorId_UnusedArg, const CPedGta* ped);
    static bool CarHasPartiallyOpenDoor(const CVehicleGta* vehicle, int32 doorId);
    static int32 ComputeDoorFlag(const CVehicleGta* vehicle, int32 doorId, bool bCheckVehicleType);
    static int32 ComputeOppositeDoorFlag(const CVehicleGta* vehicle, int32 doorId, bool bCheckVehicleType);
    static int32 ComputePassengerIndexFromCarDoor(const CVehicleGta* vehicle, int32 doorId);
    static CPedGta* ComputeSlowJackedPed(const CVehicleGta* vehicle, int32 doorId);
    static int32 ComputeTargetDoorToEnterAsPassenger(const CVehicleGta* vehicle, int32 nPassengerNum);
    static int32 ComputeTargetDoorToExit(const CVehicleGta* vehicle, const CPedGta* ped);
    static bool GetNearestCarDoor(const CPedGta* ped, const CVehicleGta* vehicle, CVector& outPos, int32& doorId);
    static bool GetNearestCarPassengerDoor(const CPedGta* ped, const CVehicleGta* vehicle, CVector* outVec, int32* doorId, bool CheckIfOccupiedTandemSeat, bool CheckIfDoorIsEnterable, bool CheckIfRoomToGetIn);
    static CVector GetPositionToOpenCarDoor(const CVehicleGta* vehicle, int32 doorId);
    static bool IsCarDoorInUse(const CVehicleGta* vehicle, int32 firstDoorId, int32 secondDoorId);
    static bool IsCarDoorReady(const CVehicleGta* vehicle, int32 doorId);
    static bool IsCarQuickJackPossible(const CVehicleGta* vehicle, int32 doorId, const CPedGta* ped);
    static bool IsCarSlowJackRequired(const CVehicleGta* vehicle, int32 doorId);
    static bool IsClearToDriveAway(const CVehicleGta* outVehicle);
    static bool IsPathToDoorBlockedByVehicleCollisionModel(const CPedGta* ped, const CVehicleGta* vehicle, const CVector& pos);
    static bool IsPedHealthy(CPedGta* vehicle);
    static bool IsPlayerToQuitCarEnter(const CPedGta* ped, const CVehicleGta* vehicle, int32 startTime, CTask* task);
    static bool IsRoomForPedToLeaveCar(const CVehicleGta* vehicle, int32 doorId, CVector* pos = nullptr);
    static bool IsVehicleHealthy(const CVehicleGta* vehicle);
    static bool IsVehicleStealable(const CVehicleGta* vehicle, const CPedGta* ped);
    static void MakeUndraggedDriverPedLeaveCar(const CVehicleGta* vehicle, const CPedGta* ped);
    static void MakeUndraggedPassengerPedsLeaveCar(const CVehicleGta* targetVehicle, const CPedGta* draggedPed, const CPedGta* ped);
    static void QuitEnteringCar(CPedGta* ped, CVehicleGta* vehicle, int32 doorId, bool bCarWasBeingJacked);
    static void RemoveCarSitAnim(const CPedGta* ped);
    static void RemoveGetInAnims(const CPedGta* ped);
    static void SetAnimOffsetForEnterOrExitVehicle();
    static bool SetPedInCarDirect(CPedGta* ped, CVehicleGta* vehicle, int32 seatNumber, bool bAsDriver = false);
};
