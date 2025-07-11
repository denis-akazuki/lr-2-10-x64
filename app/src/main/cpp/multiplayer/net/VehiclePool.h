#pragma once

#include "../game/common.h"
#include "../game/vehicle.h"
#include "../game/playerped.h"
#include "NetPool.h"

#pragma pack(push, 1)
struct NewVehiclePacket
{
	VEHICLEID 	VehicleID;
	uint32 		iVehicleType;
	CVector		vecPos;
	float 		fRotation;
	uint8_t		aColor1;
	uint8_t		aColor2;
	float		fHealth;
	uint8_t		byteInterior;
	uint32_t	dwDoorDamageStatus;
	uint32_t 	dwPanelDamageStatus;
	uint8_t		byteLightDamageStatus;
	uint8_t		byteTireDamageStatus;
	uint8_t		byteAddSiren;
	uint8_t		byteModSlots[14];
	uint8_t	  	bytePaintjob;
	uint32 	    cColor1;
    uint32	    cColor2;
};
#pragma pack(pop)

class CVehiclePool : public CNetPool<CVehicle*>
{
public:
	static void Init();
	static void Free();

	static void Process();

	static bool New(NewVehiclePacket* pNewVehicle);
	static bool Delete(VEHICLEID VehicleID);

	static VEHICLEID FindIDFromGtaPtr(CEntity * pGtaVehicle);
	static VEHICLEID FindIDFromRwObject(RwObject* pRWObject);
	static int FindGtaIDFromID(VEHICLEID ID);
	static CVehicle *FindSampPointerFromRwObject(RwObject *pRWObject);

	static void AssignSpecialParamsToVehicle(VEHICLEID VehicleID, uint8_t byteObjective, uint8_t byteDoorsLocked);

	static int FindNearestToLocalPlayerPed();

	static CVehicle *GetVehicleFromTrailer(CVehicle *pTrailer);
	static CVehicle *FindVehicle(CVehicleGta *pGtaVehicle);
};