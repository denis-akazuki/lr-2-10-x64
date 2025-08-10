//
// Created on 18.04.2023.
//
#include "game/common.h"
#include "Vehicle.h"
#include "util/patch.h"
#include "game/World.h"
#include "game/Models/ModelInfo.h"
#include "game/Coronas.h"
#include "Camera.h"
#include "net/netgame.h"

void CVehicle::RenderDriverAndPassengers() {
    if(IsRCVehicleModelID())
        return;

    if (pDriver && pDriver->m_nPedState == PEDSTATE_DRIVING) {
        CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x4A6964 + 1 : 0x59D3B8), pDriver);
       // pDriver->Render();
    }

    for (auto& passenger : m_apPassengers) {
        if (passenger && passenger->m_nPedState == PEDSTATE_DRIVING) {
            CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x4A6964 + 1 : 0x59D3B8), passenger);
           // passenger->Render();
        }
    }
}

void CVehicle::SetDriver(CPed* driver) {
    CEntity::ChangeEntityReference(pDriver, driver);

    ApplyTurnForceToOccupantOnEntry(driver);
}

bool CVehicle::AddPassenger(CPed* passenger) {
    ApplyTurnForceToOccupantOnEntry(passenger);

    // Now, find a seat and place them into it
    const auto seats = GetMaxPassengerSeats();

    for(auto & emptySeat : m_apPassengers) {
        emptySeat = passenger;
        CEntity::RegisterReference(emptySeat);
        m_nNumPassengers++;
        return false;
    }

    // No empty seats
    return false;
}

bool CVehicle::AddPassenger(CPed* passenger, uint8 seatIdx) {
    if (m_nVehicleFlags.bIsBus) {
        return AddPassenger(passenger);
    }

    // Check if seat is valid
    if (seatIdx >= m_nMaxPassengers) {
        return false;
    }

    // Check if anyone is already in that seat
    if (m_apPassengers[seatIdx]) {
        return false;
    }

    // Place passenger into seat, and add ref
    m_apPassengers[seatIdx] = passenger;
    CEntity::RegisterReference(m_apPassengers[seatIdx]);
    m_nNumPassengers++;

    return true;
}

void CVehicle::ApplyTurnForceToOccupantOnEntry(CPed* passenger) {
    // Apply some turn force
    switch (m_nVehicleType) {
        case VEHICLE_TYPE_BIKE: {
            ApplyTurnForce(
                    GetUp() * passenger->m_fMass / -50.f,
                    GetForward() / -10.f // Behind the bike
            );
            break;
        }
        default: {
            ApplyTurnForce(
                    CVector{ .0f, .0f, passenger->m_fMass / -5.f },
                    CVector{ CVector2D{passenger->GetPosition() - GetPosition()}, 0.f }
            );
            break;
        }
    }
}

int CVehicle::GetPassengerIndex(const CPed* passenger) {
    for(int i = 0; i <  std::size(m_apPassengers); i++) {
        if(passenger == m_apPassengers[i])
            return i;
    }
    return -1;
}

void CVehicle::AddVehicleUpgrade(int32 modelId) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0058C66C + 1 : 0x6AFF4C), this, modelId);
}

void CVehicle::RemoveVehicleUpgrade(int32 upgradeModelIndex) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x58CC2C + 1 : 0x6B0718), this, upgradeModelIndex);
}

// 0x6D3000
void CVehicle::SetGettingInFlags(uint8 doorId) {
    m_nGettingInFlags |= doorId;
}

// 0x6D3020
void CVehicle::SetGettingOutFlags(uint8 doorId) {
    m_nGettingOutFlags |= doorId;
}

// 0x6D3040
void CVehicle::ClearGettingInFlags(uint8 doorId) {
    m_nGettingInFlags &= ~doorId;
}

// 0x6D3060
void CVehicle::ClearGettingOutFlags(uint8 doorId) {
    m_nGettingOutFlags &= ~doorId;
}

// ----------------------------------- hooks

void RenderDriverAndPassengers_hook(CVehicle *thiz)
{
    thiz->RenderDriverAndPassengers();
}

void SetDriver_hook(CVehicle *thiz, CPed *pPed)
{
    thiz->SetDriver(pPed);
}

bool CVehicle__GetVehicleLightsStatus_hook(CVehicle *pVehicle)
{
    return pVehicle->GetLightsStatus();
}

void (*CVehicle__DoVehicleLights)(CVehicle* thiz, CMatrix *matVehicle, uint32 nLightFlags);
void CVehicle__DoVehicleLights_hook(CVehicle* thiz, CMatrix *matVehicle, uint32 nLightFlags)
{
    uint8_t old = thiz->m_nVehicleFlags.bEngineOn;
    thiz->m_nVehicleFlags.bEngineOn = 1;
    CVehicle__DoVehicleLights(thiz, matVehicle, nLightFlags);
    thiz->m_nVehicleFlags.bEngineOn = old;
}

bool CVehicle::DoTailLightEffect(int32_t lightId, CMatrix* matVehicle, int isRight, int forcedOff, uint32_t nLightFlags, int lightsOn) {

    constexpr int REVERSE_LIGHT_OFFSET = 5;

    auto pModelInfoStart = CModelInfo::GetVehicleModelInfo(m_nModelIndex);

    CVector* m_avDummyPos = pModelInfoStart->m_pVehicleStruct->m_avDummyPos;

    auto v = CVector(m_avDummyPos[1]);

    if (!isRight)
        v.x = -v.x;

    uint8_t alpha = (m_fBreakPedal > 0) ? 200 : 96;
    if (GetLightsStatus() || (m_fBreakPedal > 0 && pDriver)) {
        CCoronas::RegisterCorona(
                (uintptr) &m_placement.m_vPosn.y + 2 * lightId + isRight,
                this,
                100, 0, 0, alpha,
                &v,
                0.65f,
                /*TheCamera.LODDistMultiplier*/ 70.f,
                eCoronaType::CORONATYPE_HEADLIGHT,
                eCoronaFlareType::FLARETYPE_NONE,
                false,
                false,
                0,
                0.0f,
                false,
                0,
                0,
                15.0f,
                false,
                false
        );
    }

    if (m_nCurrentGear == 0 && m_pHandlingData->m_transmissionData.m_fCurrentSpeed < -0.01) {
        CCoronas::RegisterCorona(
                (uintptr) &m_placement.m_vPosn.y + 2 * lightId + isRight + REVERSE_LIGHT_OFFSET,
                this,
                255, 255, 255, 200,
                &v,
                0.70f,
                /*TheCamera.LODDistMultiplier*/ 70.f,
                eCoronaType::CORONATYPE_HEADLIGHT,
                eCoronaFlareType::FLARETYPE_NONE,
                false,
                false,
                0,
                0.0f,
                false,
                0,
                0,
                15.0f,
                false,
                false
        );
    }
    return true;
}

void CVehicle::DoHeadLightBeam(int lightId, CMatrix* matrix, bool isRight)
{
    auto* modelInfo = CModelInfo::GetVehicleModelInfo(m_nModelIndex);
    CVector lightOffset = modelInfo->GetModelDummyPosition(static_cast<eVehicleDummy>(2 * lightId));

    auto* pVehicle = CVehiclePool::FindVehicle(this);
    if (!pVehicle || lightId == 1 && lightOffset.IsZero())
        return;

    if (!isRight) {
        lightOffset.x = -lightOffset.x;
    }

    CVector worldPos;
    worldPos.x = matrix->m_pos.x + matrix->m_right.x * lightOffset.x + matrix->m_forward.x * lightOffset.y + matrix->m_up.x * lightOffset.z;
    worldPos.y = matrix->m_pos.y + matrix->m_right.y * lightOffset.x + matrix->m_forward.y * lightOffset.y + matrix->m_up.y * lightOffset.z;
    worldPos.z = matrix->m_pos.z + matrix->m_right.z * lightOffset.x + matrix->m_forward.z * lightOffset.y + matrix->m_up.z * lightOffset.z;

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    CVector toCamera = TheCamera.GetPosition() - worldPos;
    toCamera.Normalise();

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));
    RwRenderStateSet(rwRENDERSTATESHADEMODE, RWRSTATE(rwSHADEMODEGOURAUD));
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLNONE));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, RWRSTATE(rwALPHATESTFUNCTIONGREATER));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(0));

    float angleMult = 0.15f;
    auto alpha = static_cast<uint8>((1.0f - fabs(DotProduct(toCamera, matrix->m_forward))) * 32.0f);

    CVector beamDir;
    beamDir.x = matrix->m_forward.x - (matrix->m_up.x * angleMult);
    beamDir.y = matrix->m_forward.y - (matrix->m_up.y * angleMult);
    beamDir.z = matrix->m_forward.z - (matrix->m_up.z * angleMult);
    beamDir.Normalise();

    CVector beamRight = CrossProduct(beamDir, toCamera);
    beamRight.Normalise();

    RxObjSpace3DVertex vertices[8];

    uint8_t red, green, blue;
    pVehicle->ProcessHeadlightsColor(red, green, blue);
    RwRGBA whiteColor = {blue, green, red, alpha};
    RwRGBA transparentColor = {blue, green, red, 0};

    CVector baseLeft = worldPos - beamRight * 0.05f;
    CVector baseRight = worldPos + beamRight * 0.05f;

    CVector tipLeft = worldPos + beamDir * 3.0f - beamRight * 0.5f;
    CVector tipRight = worldPos + beamDir * 3.0f + beamRight * 0.5f;

    CVector midBase = worldPos + beamDir * 0.2f;
    CVector midLeft = baseLeft + beamDir * 1.5f;
    CVector midRight = baseRight + beamDir * 1.5f;
    CVector center = worldPos + beamDir * 1.0f;

    RxObjSpace3DVertexSetPreLitColor(&vertices[0], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[0], &baseLeft);

    RxObjSpace3DVertexSetPreLitColor(&vertices[1], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[1], &baseRight);

    RxObjSpace3DVertexSetPreLitColor(&vertices[2], &transparentColor);
    RxObjSpace3DVertexSetPos(&vertices[2], &tipLeft);

    RxObjSpace3DVertexSetPreLitColor(&vertices[3], &transparentColor);
    RxObjSpace3DVertexSetPos(&vertices[3], &tipRight);

    RxObjSpace3DVertexSetPreLitColor(&vertices[4], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[4], &midBase);

    RxObjSpace3DVertexSetPreLitColor(&vertices[5], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[5], &midLeft);

    RxObjSpace3DVertexSetPreLitColor(&vertices[6], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[6], &midRight);

    RxObjSpace3DVertexSetPreLitColor(&vertices[7], &whiteColor);
    RxObjSpace3DVertexSetPos(&vertices[7], &center);

    static RwImVertexIndex indices[] = {
            0, 1, 4,
            1, 3, 4,
            2, 3, 4,
            0, 2, 4,
            0, 5, 7,
            5, 6, 7,
            1, 6, 7,
            0, 1, 7
    };

    if (RwIm3DTransform(vertices, 8, nullptr, rwIM3D_VERTEXRGBA | rwIM3D_VERTEXXYZ)) {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, std::size(indices));
        RwIm3DEnd();
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLBACK));
}

bool DoTailLightEffect_hooked(CVehicle* vehicle, int32_t lightId, CMatrix* matVehicle, int isRight, int forcedOff, uint32_t nLightFlags, int lightsOn) {
    return vehicle->DoTailLightEffect(lightId, matVehicle, isRight, forcedOff, nLightFlags, lightsOn);
}

void DoHeadLightBeam_hooked(CVehicle* vehicle, int lightId, CMatrix* matrix, bool isRight) {
    vehicle->DoHeadLightBeam(lightId, matrix, isRight);
}

void CVehicle::InjectHooks() {
    // var
    CHook::Write(g_libGTASA + (VER_x32 ? 0x675F10 : 0x849EA8), &m_aSpecialColModel);

    CHook::Redirect("_ZN8CVehicle25RenderDriverAndPassengersEv", &RenderDriverAndPassengers_hook);
    CHook::Redirect("_ZN8CVehicle9SetDriverEP4CPed", &SetDriver_hook);

    CHook::Redirect("_ZN8CVehicle17DoTailLightEffectEiR7CMatrixhhjh", &DoTailLightEffect_hooked);
    CHook::InlineHook("_ZN8CVehicle15DoVehicleLightsER7CMatrixj", &CVehicle__DoVehicleLights_hook, &CVehicle__DoVehicleLights);
    CHook::Redirect("_ZN8CVehicle22GetVehicleLightsStatusEv", &CVehicle__GetVehicleLightsStatus_hook);
    CHook::Redirect("_ZN8CVehicle15DoHeadLightBeamEiR7CMatrixh", &DoHeadLightBeam_hooked);
}

bool CVehicle::IsRCVehicleModelID() {
    switch (m_nModelIndex) {
        case 441:
        case 464:
        case 465:
        case 594:
        case 501:
        case 564:
            return true;

        default:
            break;
    }
    return false;
}

bool CVehicle::UsesSiren() {
    switch (m_nModelIndex) {
        case MODEL_FIRETRUK:
        case MODEL_AMBULAN:
        case MODEL_MRWHOOP:
            return true;
        case MODEL_RHINO:
            return false;
        default:
            return IsLawEnforcementVehicle() != false;
    }
}

bool CVehicle::IsLawEnforcementVehicle() const {
    switch (m_nModelIndex) {
        case MODEL_ENFORCER:
        case MODEL_PREDATOR:
        case MODEL_RHINO:
        case MODEL_BARRACKS:
        case MODEL_FBIRANCH:
        case MODEL_COPBIKE:
        case MODEL_FBITRUCK:
        case MODEL_COPCARLA:
        case MODEL_COPCARSF:
        case MODEL_COPCARVG:
        case MODEL_COPCARRU:
        case MODEL_SWATVAN:
            return true;
        default:
            return false;
    }
}
