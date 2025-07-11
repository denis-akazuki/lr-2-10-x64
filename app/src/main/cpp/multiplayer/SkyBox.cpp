#include "../net/netgame.h"
#include "../gui/gui.h"

#include "../CSettings.h"
#include "SkyBox.h"
#include "game/Models/ModelInfo.h"
#include "game/game.h"
#include "game/Clock.h"
#include "game/Camera.h"
#include "game/TimeCycle.h"
#include "game/Entity/Ped/Ped.h"
#include "app/app_light.h"

extern CGUI *pGUI;

void CSkyBox::Init()
{
    if (!CModelInfo::GetModelInfo(SKYBOX_OBJECT_ID)) {
        Log("Error CSkyBox::Init. No mode %d", SKYBOX_OBJECT_ID);
        assert(0);
    }
    m_pSkyObject = CreateObjectScaled(SKYBOX_OBJECT_ID, 0.8f);
}

void CSkyBox::Process() {
    if(!m_pSkyObject)
        return CSkyBox::Init();

    RwMatrix matrix;

    m_pSkyObject->m_pEntity->GetMatrix(&matrix);

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    matrix.pos = TheCamera.GetPosition();

    CVector axis{0.0f, 0.0f, 1.0f};
    RwMatrixRotate(&matrix, &axis, m_fRotSpeed * CTimer::ms_fTimeScale);

    m_bNeedRender = true;

    ReTexture();
    
    m_pSkyObject->m_pEntity->Remove();

    m_pSkyObject->m_pEntity->SetMatrix((CMatrix&)matrix);
    m_pSkyObject->m_pEntity->UpdateRW();
    m_pSkyObject->m_pEntity->UpdateRwFrame();

    m_pSkyObject->m_pEntity->Add();

    CUtil::RenderEntity(m_pSkyObject->m_pEntity);

    m_bNeedRender = false;
}

CObject* CSkyBox::CreateObjectScaled(int iModel, float fScale)
{
    if (!pNetGame)
        return nullptr;

    CModelInfo::GetModelInfo(iModel)->m_nRefCount = 899;

    CVector vecRot(0.f, 0.f, 0.f);
    CVector vecScale(fScale);

    auto *object = new CObject(iModel, 0.0f, 0.0f, 0.0f, vecRot, 0.0f);

    object->m_pEntity->m_bUsesCollision = false;

    object->m_pEntity->Remove();

    RwMatrix matrix;
    object->m_pEntity->GetMatrix(&matrix);

    RwMatrixScale(&matrix, &vecScale);

    object->m_pEntity->SetMatrix((CMatrix&)matrix);
    object->m_pEntity->UpdateRW();
    object->m_pEntity->UpdateRwFrame();

    object->m_pEntity->Add();
    object->m_pEntity->m_bUsesCollision = false;

    return object;
}

void CSkyBox::ReTexture()
{
    int iHours = CClock::GetGameClockHours();

    if (m_dwChangeTime != iHours)
    {
        m_dwChangeTime = iHours;

        if (iHours >= 0 && iHours < 6 || iHours > 18 ) { // ночь
            SetTexture("skybox_1");
        } else if (iHours >= 6 && iHours < 8) { // рассвет
            SetTexture("skybox_2");
        } else if (iHours >= 8 && iHours < 11) {
            SetTexture("skybox_3");
        } else if (iHours >= 11 && iHours <= 15) {
            SetTexture("skybox_4");
        } else if (iHours == 16) {
            SetTexture("skybox_5");
        } else if (iHours == 17) {
            SetTexture("skybox_6");
        } else if (iHours == 18) { // закат
            SetTexture("skybox_7");
        }
//        else if (iHours == 20) { // закат 2
//            SetTexture("skybox_8");
//        }


    }
    // ---

    auto pAtomic = m_pSkyObject->m_pEntity->m_pRwObject;

    if (!pAtomic || !pAtomic->parent)
        return;

    DeActivateDirectional();
    SetFullAmbient();
    SetAmbientColours();

    RwFrameForAllObjects((RwFrame*)pAtomic->parent, RwFrameForAllObjectsCallback, m_pSkyObject);
}

RwObject* CSkyBox::RwFrameForAllObjectsCallback(RwObject* object, void* data)
{
    if (*(uint8_t*)object != 1)
        return object;

    RpAtomic* pAtomic = (RpAtomic*)object;
    if(!pAtomic)
        return object;

    RpGeometry* pGeom = pAtomic->geometry;
    if (!pGeom)
        return object;

    int numMats = pGeom->matList.numMaterials;
    if (numMats > 16)
        numMats = 16;

    for (int i = 0; i < numMats; i++)
    {
        RpMaterial* pMaterial = pGeom->matList.materials[i];
        if (!pMaterial)
            continue;

        if (m_pTex) {
            pMaterial->texture = m_pTex;
            pMaterial->color = RwRGBA{
                    static_cast<RwUInt8>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed),
                    static_cast<RwUInt8>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen),
                    static_cast<RwUInt8>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue),
                    150
            };
        }

    }

    return object;
}

void CSkyBox::SetTexture(const char *texName)
{
    if (texName == nullptr)
        return;

    if(m_pTex) {
        RwTextureDestroy(m_pTex);
    }

    m_pTex = CUtil::LoadTextureFromDB("gta3", texName);
}

void CSkyBox::SetRotSpeed(float speed)
{
    m_fRotSpeed = speed;
}

bool CSkyBox::IsNeedRender()
{
    return m_bNeedRender;
}

CObject *CSkyBox::GetSkyObject()
{
    return m_pSkyObject;
}

