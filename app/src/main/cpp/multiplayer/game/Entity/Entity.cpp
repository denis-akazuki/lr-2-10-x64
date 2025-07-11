//
// Created on 20.04.2023.
//
#include "../common.h"

#include "Entity.h"
#include "../RW/rwcore.h"
#include "../RwHelper.h"
#include "game/RW/rpskin.h"
#include "util/patch.h"
#include "game/Models/ModelInfo.h"
#include "game/References.h"
#include "game/Pools.h"
#include "game/game.h"
#include "chatwindow.h"
#include "game/Collision/ColStore.h"
#include "game/Camera.h"

void CEntity::UpdateRwFrame()
{
    if (!m_pRwObject)
        return;

    RwFrameUpdateObjects(static_cast<RwFrame*>(rwObjectGetParent(m_pRwObject)));
}

//void CEntity::DeleteRwObject()
//{
//    if(!*(uintptr*)this) return;
//
//    (( void (*)(CEntity*))(*(void**)(*(uintptr*)this + (VER_x32 ? 0x24 : 0x24*2))))(this);
//}

void CEntity::SetInterior(int interiorId, bool needRefresh)
{
    m_nAreaCode = static_cast<eAreaCodes>(interiorId);
    
    if ( this == CGame::FindPlayerPed()->m_pPed )
    {
        CGame::currArea = interiorId;

        auto pos = GetPosition();
        CColStore::RequestCollision(&pos, m_nAreaCode);

        if(interiorId == 0) {
            CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00420898 + 1 : 0x504194), false);
           // CTimeCycle::StopExtraColour(0);
        }

    }

}

void CEntity::UpdateRpHAnim() {
    if (const auto atomic = GetFirstAtomic(m_pRwClump)) {
        if (RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)) && !m_bDontUpdateHierarchy) {
            RpHAnimHierarchyUpdateMatrices(GetAnimHierarchyFromSkinClump(m_pRwClump));
        }
    }
}

float CEntity::GetDistanceFromPoint(float X, float Y, float Z) const
{
    CVector vec(X, Y, Z);

    return DistanceBetweenPoints(GetPosition(), vec);
}

float CEntity::GetDistanceFromLocalPlayerPed() const
{
    auto pLocalPlayerPed = CGame::FindPlayerPed();

    return DistanceBetweenPoints(GetPosition(), pLocalPlayerPed->m_pPed->GetPosition());
}

float CEntity::GetDistanceFromCamera()
{
    if(!this)
        return 0;

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    return DistanceBetweenPoints(GetPosition(), TheCamera.GetPosition());
}

bool CEntity::IsInCurrentArea() const
{
    return m_nAreaCode == CGame::currArea;
}

void CEntity::UpdateRW() {
    if (!m_pRwObject)
        return;

    auto parentMatrix = GetModellingMatrix();
    if (m_matrix)
        m_matrix->UpdateRwMatrix(parentMatrix);
    else
        m_placement.UpdateRwMatrix(parentMatrix);
}

RwMatrix* CEntity::GetModellingMatrix() {
    if (!m_pRwObject)
        return nullptr;

    return RwFrameGetMatrix(RwFrameGetParent(m_pRwObject));
}

CColModel* CEntity::GetColModel() const {
    if (IsVehicle()) {
        const auto veh = static_cast<const CVehicleGta*>(this);
        if (veh->m_vehicleSpecialColIndex > -1) {
            return &CVehicleGta::m_aSpecialColModel[veh->m_vehicleSpecialColIndex];
        }
    }

    return CModelInfo::GetModelInfo(m_nModelIndex)->GetColModel();
}

CVector CEntity::TransformFromObjectSpace(const CVector& offset)
{
    auto result = CVector();
    if (m_matrix) {
        result = *m_matrix * offset;
        return result;
    }

    CUtil::TransformPoint(result, m_placement, offset);
    return result;
}

// 0x533560
CVector* CEntity::TransformFromObjectSpace(CVector& outPos, const CVector& offset)
{
    auto result = TransformFromObjectSpace(offset);
    outPos = result;
    return &outPos;
}

void CEntity::SetCollisionChecking(bool bCheck)
{
    m_bCollisionProcessed = bCheck;
}


CBaseModelInfo* CEntity::GetModelInfo() const {
    return CModelInfo::GetModelInfo(m_nModelIndex);
}

CVector* CEntity::GetBoundCentre(CVector* pOutCentre)
{
    auto mi = CModelInfo::GetModelInfo(m_nModelIndex);
    const auto& colCenter = mi->GetColModel()->GetBoundCenter();
    return TransformFromObjectSpace(*pOutCentre, colCenter);
}

// 0x534290
void CEntity::GetBoundCentre(CVector& outCentre) {
    TransformFromObjectSpace(outCentre, GetColModel()->GetBoundCenter());
}

CVector CEntity::GetBoundCentre()
{
    CVector v;
    GetBoundCentre(v);
    return v;
}

bool CEntity::GetIsTouching(CEntity* entity)
{
    CVector thisVec;
    GetBoundCentre(thisVec);

    CVector otherVec;
    entity->GetBoundCentre(otherVec);

    auto fThisRadius = CModelInfo::GetModelInfo(m_nModelIndex)->GetColModel()->GetBoundRadius();
    auto fOtherRadius = CModelInfo::GetModelInfo(entity->m_nModelIndex)->GetColModel()->GetBoundRadius();

    return (thisVec - otherVec).Magnitude() <= (fThisRadius + fOtherRadius);
}

void CEntity::RegisterReference(CEntity** entity)
{
    if (IsBuilding() && !m_bIsTempBuilding && !m_bIsProcObject && !m_nIplIndex)
        return;

    auto refs = m_pReferences;
    while (refs) {
        if (refs->m_ppEntity == entity) {
            return;
        }
        refs = refs->m_pNext;
    }

    if (!m_pReferences && !CReferences::pEmptyList) {
        auto iPedsSize = GetPedPoolGta()->GetSize();
        for (int32 i = 0; i < iPedsSize; ++i) {
            auto ped = GetPedPoolGta()->GetAt(i);
            if (ped) {
                ped->PruneReferences();
                if (CReferences::pEmptyList)
                    break;
            }

        }

        if (!CReferences::pEmptyList) {
            auto iVehsSize = GetVehiclePoolGta()->GetSize();
            for (int32 i = 0; i < iVehsSize; ++i) {
                auto vehicle = GetVehiclePoolGta()->GetAt(i);
                if (vehicle) {
                    vehicle->PruneReferences();
                    if (CReferences::pEmptyList)
                        break;
                }

            }
        }

        if (!CReferences::pEmptyList) {
            auto iObjectsSize = GetObjectPoolGta()->GetSize();
            for (int32 i = 0; i < iObjectsSize; ++i) {
                auto obj = GetObjectPoolGta()->GetAt(i);
                if (obj) {
                    obj->PruneReferences();
                    if (CReferences::pEmptyList)
                        break;
                }
            }
        }
    }

    if (CReferences::pEmptyList) {
        auto pEmptyRef = CReferences::pEmptyList;
        CReferences::pEmptyList = pEmptyRef->m_pNext;
        pEmptyRef->m_pNext = m_pReferences;
        m_pReferences = pEmptyRef;
        pEmptyRef->m_ppEntity = entity;
    }
}

void CEntity::ResolveReferences()
{
    auto refs = m_pReferences;
    while (refs) {
        if (*refs->m_ppEntity == this)
            *refs->m_ppEntity = nullptr;

        refs = refs->m_pNext;
    }

    refs = m_pReferences;
    if (!refs)
        return;

    refs->m_ppEntity = nullptr;
    while (refs->m_pNext)
        refs = refs->m_pNext;

    refs->m_pNext = CReferences::pEmptyList;
    CReferences::pEmptyList = m_pReferences;
    m_pReferences = nullptr;
}

void CEntity::PruneReferences()
{
    if (!m_pReferences)
        return;

    auto refs = m_pReferences;
    auto ppPrev = &m_pReferences;
    while (refs) {
        if (*refs->m_ppEntity == this) {
            ppPrev = &refs->m_pNext;
            refs = refs->m_pNext;
        }
        else {
            auto refTemp = refs->m_pNext;
            *ppPrev = refs->m_pNext;
            refs->m_pNext = CReferences::pEmptyList;
            CReferences::pEmptyList = refs;
            refs->m_ppEntity = nullptr;
            refs = refTemp;
        }
    }
}

void CEntity::CleanUpOldReference(CEntity** entity)
{
    if (!m_pReferences)
        return;

    auto refs = m_pReferences;
    auto ppPrev = &m_pReferences;
    while (refs->m_ppEntity != entity) {
        ppPrev = &refs->m_pNext;
        refs = refs->m_pNext;
        if (!refs)
            return;
    }

    *ppPrev = refs->m_pNext;
    refs->m_pNext = CReferences::pEmptyList;
    refs->m_ppEntity = nullptr;
    CReferences::pEmptyList = refs;
}

bool CEntity::DoesNotCollideWithFlyers()
{
    auto mi = CModelInfo::GetModelInfo(m_nModelIndex);
    return mi->SwaysInWind() || mi->bDontCollideWithFlyer;
}

CEntity::CEntity() : CPlaceable() {
    m_nStatus = STATUS_ABANDONED;
    m_nType = ENTITY_TYPE_NOTHING;

    m_nFlags = 0;
    m_bIsVisible = true;
    m_bBackfaceCulled = true;

    m_nScanCode = 0;
    m_nAreaCode = eAreaCodes::AREA_CODE_NORMAL_WORLD;
    m_nModelIndex = 0xFFFF;
    m_pRwObject = nullptr;
    m_nIplIndex = 0;
    m_nRandomSeed = CGeneral::GetRandomNumber();
    m_pReferences = nullptr;
    m_pStreamingLink = nullptr;
    m_nNumLodChildren = 0;
    m_nNumLodChildrenRendered = 0;
    m_pLod = nullptr;
}

CEntity::~CEntity()
{
    if (m_pLod)
        m_pLod->m_nNumLodChildren--;

    CEntity::DeleteRwObject();
    CEntity::ResolveReferences();
}

void CEntity::Add()
{
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3ED8D8 + 1 : 0x4CD574), this);
}

// ------------- hooks

inline void CleanUpOldReference_hook(CEntity *thiz, CEntity** entity) {
    thiz->CleanUpOldReference(entity);
}

inline void ResolveReferences_hook(CEntity *thiz) {
    thiz->ResolveReferences();
}

inline void PruneReferences_hook(CEntity* thiz) {
    thiz->PruneReferences();
}

inline void RegisterReference_hook(CEntity* thiz, CEntity** entity) {
    thiz->RegisterReference(entity);
}

void CEntity::InjectHooks() {
}

void CEntity::Add(const CRect* rect) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3ED8FC + 1 : 0x4CD5C0), this, rect);
}

void CEntity::Remove() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3EDBE8 + 1 : 0x4CD888), this);
}

void CEntity::SetModelIndex(uint32 index) {

}

void CEntity::SetModelIndexNoCreate(uint32 index) {

}

void CEntity::SetIsStatic(bool isStatic) {

}

void CEntity::CreateRwObject() {

}

CRect CEntity::GetBoundRect() {
    return CRect();
}

void CEntity::ProcessControl() {

}

void CEntity::ProcessCollision() {

}

void CEntity::DeleteRwObject() {

}

void CEntity::ProcessShift() {

}

bool CEntity::TestCollision(bool bApplySpeed) {
    return false;
}

void CEntity::Teleport(CVector destination, bool resetRotation) {

}

void CEntity::SpecialEntityPreCollisionStuff(struct CPhysical *colPhysical, bool bIgnoreStuckCheck, bool &bCollisionDisabled, bool &bCollidedEntityCollisionIgnored, bool &bCollidedEntityUnableToMove, bool &bThisOrCollidedEntityStuck) {

}

uint8 CEntity::SpecialEntityCalcCollisionSteps(bool &bProcessCollisionBeforeSettingTimeStep, bool &unk2) {
    return 0;
}

void CEntity::PreRender() {

}

void CEntity::Render() {

}

bool CEntity::SetupLighting() {
    return false;
}

void CEntity::RemoveLighting(bool bRemove) {

}

void CEntity::FlagToDestroyWhenNextProcessed() {

}
