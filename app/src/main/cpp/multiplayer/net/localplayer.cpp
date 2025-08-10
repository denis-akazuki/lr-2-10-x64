#include "../main.h"
#include "../game/game.h"
#include "netgame.h"

#include "../game/scripting.h"

#include "../game/common.h"
#include "..//keyboard.h"
#include "..//chatwindow.h"
#include "../CSettings.h"
#include "../util/CJavaWrapper.h"
#include "java_systems/HUD.h"
#include "java_systems/Inventory.h"
#include "../game/Weapon.h"
#include "../game/Entity/Ped/Ped.h"
#include "../java_systems/SkinShop.h"
#include "../java_systems/Race.h"
#include "Monologue.h"
#include "../game/Camera.h"

extern int iNetModeNormalOnfootSendRate;
extern int iNetModeNormalInCarSendRate;
extern bool bUsedPlayerSlots[];

#define IS_TARGETING(x) ((x) & 128)
#define IS_FIRING(x) ((x) & 4)

#define NETMODE_HEADSYNC_SENDRATE		1000
#define NETMODE_AIM_SENDRATE			100
#define NETMODE_FIRING_SENDRATE			30


#define STATS_UPDATE_TICKS				1000

CLocalPlayer::CLocalPlayer()
{
	m_pPlayerPed = CGame::FindPlayerPed();
	m_bDeathSended = false;

	m_bInRCMode = false;

	m_dwLastSendTick = GetTickCount();
	m_dwLastSendAimTick = GetTickCount();
	m_dwLastSendSpecTick = GetTickCount();
	m_dwLastUpdateHudButtons = GetTickCount();
	m_dwLastAimSendTick = m_dwLastSendTick;
    m_dwTimeLastSendOnFootSync = GetTickCount();
	m_dwLastStatsUpdateTick = GetTickCount();
	m_dwLastUpdateInCarData = GetTickCount();
	m_dwLastUpdatePassengerData = GetTickCount();
	m_dwPassengerEnterExit = GetTickCount();

	m_CurrentVehicle = INVALID_VEHICLE_ID;
	ResetAllSyncAttributes();

	m_bIsSpectating = false;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = 0xFFFFFFFF;

	uint8_t i;
	for (i = 0; i < 13; i++)
	{
		m_byteLastWeapon[i] = 0;
		m_dwLastAmmo[i] = 0;
	}
	//CGame::RequestModel(18646);
}

CLocalPlayer::~CLocalPlayer()
= default;

void CLocalPlayer::ResetAllSyncAttributes()
{
	m_byteCurInterior = 0;
	m_bInRCMode = false;
}

void CLocalPlayer::SendStatsUpdate() const
{
	RakNet::BitStream bsStats;

//	uint32_t wAmmo = m_pPlayerPed->GetCurrentWeaponSlot()->dwAmmo;

	bsStats.Write((BYTE)ID_STATS_UPDATE);
	bsStats.Write(CHUD::iLocalMoney);
	//bsStats.Write(wAmmo);
	bsStats.Write(m_pPlayerPed->drunk_level);
	pNetGame->GetRakClient()->Send(&bsStats, HIGH_PRIORITY, UNRELIABLE, 0);
}

void CLocalPlayer::CheckWeapons()
{
	RakNet::BitStream bs;
	bs.Write((uint8_t) ID_WEAPONS_UPDATE);

	bs.Write((uint16_t)INVALID_PLAYER_ID);
	bs.Write((uint16_t)INVALID_PLAYER_ID);

	bool bMSend = false;

	for (int i = 0; i < MAX_WEAPONS_SLOT; i++) {

		if (m_byteLastWeapon[i] != m_pPlayerPed->m_pPed->m_aWeapons[i].m_nType ||
                m_dwLastAmmo[i] != m_pPlayerPed->m_pPed->m_aWeapons[i].m_nTotalAmmo)
		{
			m_byteLastWeapon[i] = m_pPlayerPed->m_pPed->m_aWeapons[i].m_nType;
			m_dwLastAmmo[i] = m_pPlayerPed->m_pPed->m_aWeapons[i].m_nTotalAmmo;

			bs.Write((uint8_t) 	i);
			bs.Write((uint8_t) 	m_byteLastWeapon[i]);
			bs.Write((uint16_t) m_dwLastAmmo[i]);

            bMSend = true;
		}

	}
	if (bMSend) {
		pNetGame->GetRakClient()->Send(&bs, HIGH_PRIORITY, RELIABLE, 0);
	}
}
extern bool g_uiHeadMoveEnabled;

#include "..//game/CWeaponsOutFit.h"
#include "java_systems/ObjectEditor.h"
#include "java_systems/casino/Chip.h"
#include "java_systems/AucContainer.h"
#include "util/patch.h"
#include "java_systems/AdminRecon.h"
#include "java_systems/Medic.h"
#include "java_systems/Tab.h"
#include "java_systems/DailyReward.h"
#include "java_systems/Styling.h"
#include "java_systems/Dialog.h"
#include "java_systems/TechInspect.h"
#include "java_systems/casino/Baccarat.h"
#include "java_systems/TireShop.h"
#include "java_systems/casino/Dice.h"
#include "java_systems/TheftAuto.h"
#include "java_systems/AutoShop.h"
#include "java_systems/casino/LuckyWheel.h"
#include "java_systems/RadialMenu.h"
#include "java_systems/GuiWrapper.h"
#include "game/World.h"
#include "voice/SpeakerList.h"
#include "voice/Record.h"
#include "voice/Playback.h"
#include "GuiWrapper.h"
#include "game/Tasks/TaskTypes/TaskComplexEnterCarAsDriver.h"

CAMERA_AIM* caAim = new CAMERA_AIM();

CVector lastPos;

uint32_t CLocalPlayer::CalculateAimSendRate(uint16_t wKeys) const {
	uint32_t baseRate = NETMODE_HEADSYNC_SENDRATE;

	if (IS_TARGETING(wKeys)) {
        if (IS_FIRING(wKeys)) {
			baseRate = NETMODE_FIRING_SENDRATE + m_nPlayersInRange;
		} else {
			baseRate = NETMODE_AIM_SENDRATE + m_nPlayersInRange;
		}
	} else if (IS_FIRING(wKeys)) {
		baseRate = GetOptimumOnFootSendRate();
	}

	return static_cast<uint32_t>(baseRate);
}


bool CLocalPlayer::Process()
{
	if(CTimer::GetIsUserPaused())
		return false;

	lastPos = m_pPlayerPed->m_pPed->GetPosition();

	m_pPlayerPed->SetCurrentAim(caAim);
	caAim = m_pPlayerPed->GetCurrentAim();

	uint32_t dwThisTick = GetTickCount();

	if(m_pPlayerPed) {

		if(CGame::m_bRaceCheckpointsEnabled) {
			if(DistanceBetweenPoints2D(CGame::m_vecRaceCheckpointPos, m_pPlayerPed->m_pPed->GetPosition()) <= CGame::m_fRaceCheckpointSize)
			{
				CGame::RaceCheckpointPicked();
			}
		}
		if (m_pPlayerPed->drunk_level) {
			m_pPlayerPed->drunk_level--;
			ScriptCommand(&SET_PLAYER_DRUNKENNESS, m_pPlayerPed->m_bytePlayerNumber,
						  m_pPlayerPed->drunk_level / 100);
		}
		// handle dead
		if (!m_bDeathSended && m_pPlayerPed->IsDead()) {
			ToggleSpectating(false);
			m_pPlayerPed->FlushAttach();
			// reset tasks/anims
			m_pPlayerPed->TogglePlayerControllable(true);

			if (m_pPlayerPed->m_pPed->IsAPassenger()) {

				SendInCarFullSyncData();
			}

			//m_pPlayerPed->SetDead();
			SendDeath();

			m_bDeathSended = true;

			return true;
		}

		if ((dwThisTick - m_dwLastStatsUpdateTick) > STATS_UPDATE_TICKS) {
			SendStatsUpdate();
			m_dwLastStatsUpdateTick = dwThisTick;
		}

		CheckWeapons();
		CWeaponsOutFit::ProcessLocalPlayer(m_pPlayerPed);

		m_pPlayerPed->ProcessSpecialAction(m_pPlayerPed->m_iCurrentSpecialAction);

		// handle interior changing
		if (CGame::currArea != m_byteCurInterior)
			UpdateRemoteInterior(CGame::currArea);

		// SPECTATING
		if (m_bIsSpectating) {
			ProcessSpectating();
		}

		// INCAR
		else if (m_pPlayerPed->m_pPed->IsInVehicle()) {
            MaybeSendExitVehicle();

			m_CurrentVehicle = m_pPlayerPed->GetCurrentSampVehicleID();
			if(!m_pPlayerPed->m_pPed->IsAPassenger())
			{// DRIVER
				if ((dwThisTick - m_dwLastSendTick) > (unsigned int) GetOptimumInCarSendRate()) {
					m_dwLastSendTick = GetTickCount();
					SendInCarFullSyncData();
				}
			}
			else
			{// PASSENGER
				if ((dwThisTick - m_dwLastSendTick) > (unsigned int) GetOptimumInCarSendRate()) {
					m_dwLastSendTick = GetTickCount();
					SendPassengerFullSyncData();
				}
			}
		}
		else if (dwThisTick - m_dwTimeLastSendOnFootSync > GetOptimumOnFootSendRate()) {
            MaybeSendEnterVehicle();

			SendOnFootFullSyncData();
			UpdateSurfing();

			m_CurrentVehicle = INVALID_VEHICLE_ID;
		}


		// aim. onfoot and incar?
//		if ((dwThisTick - m_dwLastHeadUpdate) > 1000 && g_uiHeadMoveEnabled) {
//			CVector LookAt;
//			CAMERA_AIM *Aim = GameGetInternalAim();
//			LookAt = Aim->vecSource + (Aim->vecFront * 20.0f);
//
//			CGame::FindPlayerPed()->ApplyCommandTask("FollowPedSA", 0, 2000, -1, &LookAt, 0,
//														 0.1f, 500, 3, 0);
//			m_dwLastHeadUpdate = dwThisTick;
//		}

		uint16_t lrAnalog, udAnalog;
		uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog);

		if ((dwThisTick - m_dwLastAimSendTick) > CalculateAimSendRate(wKeys)) {
			m_dwLastAimSendTick = dwThisTick;
			SendAimSyncData();
		}

		if ((dwThisTick - m_dwLastUpdateHudButtons) > 100) {
			m_dwLastUpdateHudButtons = GetTickCount();
			// ������ ����/�����/������� ������
			if (!m_pPlayerPed->lToggle ||
				m_pPlayerPed->m_iCurrentSpecialAction == SPECIAL_ACTION_CARRY ||
				!CHUD::bIsShow) {
				if (CHUD::bIsShowEnterExitButt) {
					CHUD::bIsShowEnterExitButt = false;
				}
				if (CHUD::bIsShowLockButt) {
					CHUD::toggleLockButton(false);
				}
			} else if (!m_pPlayerPed->m_pPed->IsInVehicle()) {
				{
					VEHICLEID ClosetVehicleID = CVehiclePool::FindNearestToLocalPlayerPed();

					if (ClosetVehicleID != INVALID_VEHICLE_ID) {
						CVehicleSamp *pVehicle = CVehiclePool::GetAt(ClosetVehicleID);
						if (pVehicle && pVehicle->m_pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f  && !pVehicle->m_pVehicle->IsTrailer()) {
							//if(!pVehicle->m_bIsLocked)
							if (pVehicle->m_pVehicle->m_nDoorLock != CARLOCK_LOCKED) {// ����� �������
								if (!CHUD::bIsShowEnterExitButt) {
									CHUD::bIsShowEnterExitButt = true;
								}
							} else {
								if (CHUD::bIsShowEnterExitButt) {
									CHUD::bIsShowEnterExitButt = false;
								}
							}
							if (!CHUD::bIsShowLockButt) {
								CHUD::toggleLockButton(true);
							}

						} else {
							if (CHUD::bIsShowEnterExitButt) {
								CHUD::bIsShowEnterExitButt = false;
							}
							if (CHUD::bIsShowLockButt) {
								CHUD::toggleLockButton(false);
							}
						}
					} else {
						if (CHUD::bIsShowEnterExitButt) {
							CHUD::bIsShowEnterExitButt = false;
						}
						if (CHUD::bIsShowLockButt) {
							CHUD::toggleLockButton(false);
						}
					}
				}

			} else {// � ������
				if (m_pPlayerPed->m_pPed->IsAPassenger()) {// �� ����������
					if (!CHUD::bIsShowEnterExitButt) {
						CHUD::bIsShowEnterExitButt = true;
					}
					if (CHUD::bIsShowLockButt) {
						CHUD::toggleLockButton(false);
					}
				} else {
					if (!CHUD::bIsShowEnterExitButt) {
						CHUD::bIsShowEnterExitButt = true;
					}

					if (!CHUD::bIsShowLockButt) {
						CHUD::toggleLockButton(true);
					}
				}
			}
		}
	}
	////////////////////////////
	bool needDrawableHud = true;
	bool needDrawableChat = true;

	if(CDialog::bIsShow || CDice::bIsShow || CTab::bIsShow || CAutoShop::bIsShow || CRadialMenu::bIsShow
	   || CLuckyWheel::bIsShow || !m_pPlayerPed || CSkinShop::bIsShow ||
	   CMedic::bIsShow || CInventory::bIsShow || !m_pPlayerPed->m_bIsSpawned || CObjectEditor::bIsToggle || CChip::bIsShow
	   || CAucContainer::bIsShow || CAdminRecon::bIsShow || CHUD::bIsCamEditGui || CDailyReward::bIsShow ||
	   CTechInspect::bIsShow || CBaccarat::bIsShow || m_pPlayerPed->IsDead() || CStyling::bIsShow || CTireShop::bIsShow || CTheftAuto::bIsShow)
	{
		needDrawableHud = false;
	}

	if (CMonologue::bIsShow || CTireShop::bIsShow || CTheftAuto::bIsShow || CStyling::bIsShow || CRadialMenu::bIsShow || CRace::bIsShow) {
		needDrawableChat = false;
	}

	CHUD::toggleAll(needDrawableHud, needDrawableChat);

    return true;
}

void CLocalPlayer::SendDeath()
{
	RakNet::BitStream bsPlayerDeath;

	bsPlayerDeath.Write((uint16)	CPlayerPool::GetLocalPlayer()->lastDamageId);
	bsPlayerDeath.Write((uint8)		CPlayerPool::GetLocalPlayer()->lastDamageWeap);

	pNetGame->GetRakClient()->RPC(&RPC_Death, &bsPlayerDeath, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
}

void CLocalPlayer::GoEnterVehicle(bool passenger)
{
	if (GetTickCount() - m_dwPassengerEnterExit < 1000)
		return;

	VEHICLEID ClosetVehicleID = CVehiclePool::FindNearestToLocalPlayerPed();
	if (ClosetVehicleID != INVALID_VEHICLE_ID)
	{
		CVehicleSamp* pVehicle = CVehiclePool::GetAt(ClosetVehicleID);

		if (pVehicle != nullptr && pVehicle->m_pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f)
		{
			m_pPlayerPed->EnterVehicle(pVehicle->m_dwGTAId, passenger);
			SendEnterVehicleNotification(ClosetVehicleID, passenger);
			m_dwPassengerEnterExit = GetTickCount();
		}
	}

}

void CLocalPlayer::UpdateSurfing() {}

void CLocalPlayer::SendEnterVehicleNotification(VEHICLEID VehicleID, bool bPassenger)
{
    DLOG("SendEnterVehicleNotification");

	RakNet::BitStream bsSend;

	bsSend.Write(VehicleID);
	bsSend.Write((uint8_t)bPassenger);

	pNetGame->GetRakClient()->RPC(&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
}

void CLocalPlayer::MaybeSendExitVehicle() {
    static bool oldExitVehicleState = false;
    bool exitVehicleState = m_pPlayerPed->m_pPed->IsExitingVehicle();

    if(exitVehicleState && !oldExitVehicleState) {
        auto vehicleId = CVehiclePool::FindIDFromGtaPtr(m_pPlayerPed->m_pPed->pVehicle);

        if(vehicleId != INVALID_VEHICLE_ID) {
            RakNet::BitStream bsSend;

            bsSend.Write(vehicleId);
            pNetGame->GetRakClient()->RPC(&RPC_ExitVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
        }

    }
    oldExitVehicleState = exitVehicleState;
}

void CLocalPlayer::MaybeSendEnterVehicle() {
    static bool oldEnterVehicleState = false;

    auto task
        = static_cast<CTaskComplexEnterCarAsDriver*>(m_pPlayerPed->m_pPed->GetTaskManager().CTaskManager::FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_DRIVER));

    bool enterVehicleState = task != nullptr;

    if(enterVehicleState && !oldEnterVehicleState) {
        auto vehicleId = CVehiclePool::FindIDFromGtaPtr(task->GetTarget());

        if(vehicleId != INVALID_VEHICLE_ID)
            SendEnterVehicleNotification(vehicleId, false);
    }
    oldEnterVehicleState = enterVehicleState;
}


void CLocalPlayer::UpdateRemoteInterior(uint8_t byteInterior)
{
	Log("CLocalPlayer::UpdateRemoteInterior %d", byteInterior);

	m_byteCurInterior = byteInterior;
	RakNet::BitStream bsUpdateInterior;
	bsUpdateInterior.Write(byteInterior);
	pNetGame->GetRakClient()->RPC(&RPC_SetInteriorId, &bsUpdateInterior, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

bool CLocalPlayer::Spawn(const CVector pos, float rot)
{
	Log("CLocalPlayer::Spawn");

	Voice::Playback::SetSoundEnable(true);
	Voice::Record::SetMicroEnable(true);
	Voice::SpeakerList::Show();

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

	TheCamera.RestoreWithJumpCut();
	CCamera::SetBehindPlayer();

	//CGame::DisplayWidgets(true);
	//CGame::DisplayHUD(true);
	m_pPlayerPed->TogglePlayerControllable(true);
	
	m_pPlayerPed->SetInitialState();

	m_pPlayerPed->m_bIsSpawned = true;

	CGame::RefreshStreamingAt(pos.x,pos.y);

	//m_pPlayerPed->RestartIfWastedAt(pos, rot);
	//m_pPlayerPed->SetModelIndex(skin);
	//m_pPlayerPed->ClearAllWeapons();
	m_pPlayerPed->ResetDamageEntity();

	//CGame::DisableTrainTraffic();

	if(m_pPlayerPed->m_pPed->IsInVehicle())
		m_pPlayerPed->m_pPed->RemoveFromVehicleAndPutAt(pos);
	else
		m_pPlayerPed->m_pPed->Teleport(pos, false);

	m_pPlayerPed->ForceTargetRotation(rot);

	m_bDeathSended = false;

//	RakNet::BitStream bsSendSpawn;
//	pNetGame->GetRakClient()->RPC(&RPC_Spawn, &bsSendSpawn, SYSTEM_PRIORITY,
//		RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, nullptr);

	return true;
}

uint32_t CLocalPlayer::GetPlayerColor()
{
	return TranslateColorCodeToRGBA(CPlayerPool::GetLocalPlayerID());
}

void CLocalPlayer::SetPlayerColor(uint32_t dwColor)
{
	SetRadarColor(CPlayerPool::GetLocalPlayerID(), dwColor);
}

int CLocalPlayer::GetOptimumOnFootSendRate() const
{
	if(!m_pPlayerPed) return 1000;

	return (iNetModeNormalOnfootSendRate + m_nPlayersInRange * 1.5);
}

uint8_t CLocalPlayer::GetSpecialAction()
{
	if(!m_pPlayerPed) return SPECIAL_ACTION_NONE;

	if(m_pPlayerPed->IsCrouching())
		return SPECIAL_ACTION_DUCK;

	if(m_pPlayerPed->IsActionCarry)
		return SPECIAL_ACTION_CARRY;

	if(m_pPlayerPed->m_pPed->IsEnteringCar())
		return SPECIAL_ACTION_ENTER_VEHICLE;

    if(m_pPlayerPed->m_pPed->IsExitingVehicle())
        return SPECIAL_ACTION_EXIT_VEHICLE;


	return SPECIAL_ACTION_NONE;
}

int CLocalPlayer::GetOptimumInCarSendRate() const
{
	if(!m_pPlayerPed) return 1000;

	return (iNetModeNormalInCarSendRate + m_nPlayersInRange * 1.5);
}

#include "..//chatwindow.h"
#include "game/CarEnterExit.h"
#include "localplayer.h"
#include "game/Camera.h"


void CLocalPlayer::SendOnFootFullSyncData()
{
	RakNet::BitStream bsPlayerSync;

	uint16_t lrAnalog, udAnalog;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog);

	static ONFOOT_SYNC_DATA ofSync {};

	auto matPlayer = m_pPlayerPed->m_pPed->GetMatrix().ToRwMatrix();

	ofSync.lrAnalog = lrAnalog;
	ofSync.udAnalog = udAnalog;
	ofSync.wKeys = wKeys;
	ofSync.vecPos = matPlayer.pos;

	ofSync.quat.SetFromMatrix(&matPlayer);
	ofSync.quat.Normalize();

	if( FloatOffset(ofSync.quat.w, m_LastSendOnFootSync.quat.w) < 0.00001 &&
		FloatOffset(ofSync.quat.x, m_LastSendOnFootSync.quat.x) < 0.00001 &&
		FloatOffset(ofSync.quat.y, m_LastSendOnFootSync.quat.y) < 0.00001 &&
		FloatOffset(ofSync.quat.z, m_LastSendOnFootSync.quat.z) < 0.00001)
	{
		ofSync.quat.Set(m_LastSendOnFootSync.quat);
	}

	ofSync.byteHealth = (uint8_t)m_pPlayerPed->GetHealth();
	ofSync.byteArmour = (uint8_t)m_pPlayerPed->GetArmour();

	uint8_t exKeys = GetPlayerPed()->GetExtendedKeys();
	ofSync.byteCurrentWeapon = (exKeys << 6) | (ofSync.byteCurrentWeapon & 0x3F);
	ofSync.byteCurrentWeapon ^= (ofSync.byteCurrentWeapon ^ GetPlayerPed()->GetCurrentWeapon()) & 0x3F;

	ofSync.byteSpecialAction = GetSpecialAction();

	ofSync.vecMoveSpeed = m_pPlayerPed->m_pPed->GetMoveSpeed();

	ofSync.vecSurfOffsets = 0.0f;

	ofSync.wSurfInfo = 0;

	ofSync.dwAnimation = 0;

	if((GetTickCount() - m_dwTimeLastSendOnFootSync) > 500 || memcmp(&m_LastSendOnFootSync, &ofSync, sizeof(ONFOOT_SYNC_DATA)) != 0)
	{
        m_dwTimeLastSendOnFootSync = GetTickCount();

		bsPlayerSync.Write((uint8_t)ID_PLAYER_SYNC);
		bsPlayerSync.Write((char*)&ofSync, sizeof(ONFOOT_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsPlayerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		m_LastSendOnFootSync = ofSync;
	}
}

void CLocalPlayer::SendInCarFullSyncData()
{
	RakNet::BitStream bsVehicleSync;
	if(!m_pPlayerPed || !m_pPlayerPed->m_pPed)return;

	RwMatrix matPlayer;
	CVector vecMoveSpeed;

	uint16_t lrAnalog, udAnalog;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog);

	CVehicleSamp *pVehicle = m_pPlayerPed->GetCurrentVehicle();
	if(!pVehicle || !pVehicle->m_pVehicle)return;

	INCAR_SYNC_DATA icSync;
	memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

	VEHICLEID vehicleid = m_pPlayerPed->GetCurrentSampVehicleID();
	if(vehicleid == INVALID_VEHICLE_ID)return;

	icSync.VehicleID = vehicleid;

	icSync.lrAnalog = lrAnalog;
	icSync.udAnalog = udAnalog;
	icSync.wKeys = wKeys;

	pVehicle->m_pVehicle->GetMatrix(&matPlayer);
	vecMoveSpeed = pVehicle->m_pVehicle->GetMoveSpeed();

	icSync.quat.SetFromMatrix(&matPlayer);
	icSync.quat.Normalize();

	if(	FloatOffset(icSync.quat.w, m_InCarData.quat.w) < 0.00001 &&
		FloatOffset(icSync.quat.x, m_InCarData.quat.x) < 0.00001 &&
		FloatOffset(icSync.quat.y, m_InCarData.quat.y) < 0.00001 &&
		FloatOffset(icSync.quat.z, m_InCarData.quat.z) < 0.00001) {

		icSync.quat.Set(m_InCarData.quat);
	}

	// pos
	icSync.vecPos = matPlayer.pos;

	// move speed
	icSync.vecMoveSpeed = vecMoveSpeed;

	icSync.fCarHealth = pVehicle->GetHealth();
	icSync.bytePlayerHealth = (uint8_t)m_pPlayerPed->GetHealth();
	icSync.bytePlayerArmour = (uint8_t)m_pPlayerPed->GetArmour();

	icSync.HydraThrustAngle = pVehicle->m_iTurnState;

	icSync.TrailerID = 0;

	auto pTrailer = pVehicle->m_pVehicle->m_pTrailer;
	if (pTrailer != nullptr)
	{
		if (pTrailer->m_pTractor == pVehicle->m_pVehicle) {
			uint16_t trailerId = CVehiclePool::FindIDFromGtaPtr(pTrailer);
			icSync.TrailerID = trailerId;
		}
	}
    if (icSync.TrailerID && icSync.TrailerID < MAX_VEHICLES)
	{
		RwMatrix matTrailer;
		TRAILER_SYNC_DATA trSync;
		CVehicleSamp* pTrailer = CVehiclePool::GetAt(icSync.TrailerID);

		if (pTrailer && pTrailer->m_pVehicle)
		{
			pTrailer->m_pVehicle->GetMatrix(&matTrailer);
			trSync.trailerID = icSync.TrailerID;

			trSync.vecPos = matTrailer.pos;

			CQuaternion syncQuat;
			syncQuat.SetFromMatrix(&matTrailer);
			trSync.quat = syncQuat;

			trSync.vecMoveSpeed = pTrailer->m_pVehicle->GetMoveSpeed();
			trSync.vecTurnSpeed = pTrailer->m_pVehicle->GetTurnSpeed();

			RakNet::BitStream bsTrailerSync;
			bsTrailerSync.Write((BYTE)ID_TRAILER_SYNC);
			bsTrailerSync.Write((char*)& trSync, sizeof(TRAILER_SYNC_DATA));
			pNetGame->GetRakClient()->Send(&bsTrailerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
		}
	}
	// send
	if( (GetTickCount() - m_dwLastUpdateInCarData) > 500 || memcmp(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA))) {
		m_dwLastUpdateInCarData = GetTickCount();
		bsVehicleSync.Write((uint8_t) ID_VEHICLE_SYNC);
		bsVehicleSync.Write((char *) &icSync, sizeof(INCAR_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsVehicleSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		memcpy(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA));
	}
}

void CLocalPlayer::SendPassengerFullSyncData()
{
	RakNet::BitStream bsPassengerSync;

	uint16_t lrAnalog, udAnalog;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog);
	PASSENGER_SYNC_DATA psSync;

	psSync.VehicleID = m_pPlayerPed->GetCurrentSampVehicleID();

	if(psSync.VehicleID == INVALID_VEHICLE_ID) return;

	psSync.lrAnalog = lrAnalog;
	psSync.udAnalog = udAnalog;
	psSync.wKeys = wKeys;
	psSync.bytePlayerHealth = (uint8_t)m_pPlayerPed->GetHealth();
	psSync.bytePlayerArmour = (uint8_t)m_pPlayerPed->GetArmour();

	psSync.byteSeatFlags = m_pPlayerPed->m_pPed->GetSampSeatId();

	psSync.byteDriveBy = 0;//m_bPassengerDriveByMode;

	psSync.byteCurrentWeapon = m_pPlayerPed->GetCurrentWeapon();//m_pPlayerPed->GetCurrentWeapon();

	psSync.vecPos = m_pPlayerPed->m_pPed->GetPosition();

	// send
	if((GetTickCount() - m_dwLastUpdatePassengerData) > 500 || memcmp(&m_PassengerData, &psSync, sizeof(PASSENGER_SYNC_DATA)))
	{
		m_dwLastUpdatePassengerData = GetTickCount();

		bsPassengerSync.Write((uint8_t)ID_PASSENGER_SYNC);
		bsPassengerSync.Write((char*)&psSync, sizeof(PASSENGER_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsPassengerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		memcpy(&m_PassengerData, &psSync, sizeof(PASSENGER_SYNC_DATA));
	}
}

void CLocalPlayer::SendAimSyncData() const
{
    AIM_SYNC_DATA aimSync;

    CAMERA_AIM* caAim = m_pPlayerPed->GetCurrentAim();

    aimSync.byteCamMode = m_pPlayerPed->GetCameraMode();
    aimSync.vecAimf = caAim->vecFront;

    aimSync.vecAimPos = caAim->vecSource;

    aimSync.fAimZ = m_pPlayerPed->GetAimZ();
    aimSync.aspect_ratio = /*GameGetAspectRatio() * */ 255.0f;
	aimSync.byteCamExtZoom = static_cast<unsigned char>(m_pPlayerPed->GetCameraExtendedZoom() * 63.0f) & 63;
//    aimSync.byteCamExtZoom = (uint8_t)(m_pPlayerPed->GetCameraExtendedZoom() * 63.0f);

    CWeapon* pwstWeapon = m_pPlayerPed->GetCurrentWeaponSlot();
    aimSync.byteWeaponState = pwstWeapon->m_nState;

	if (pwstWeapon->m_nState == 2)
		aimSync.byteWeaponState = 3;
	else
		aimSync.byteWeaponState = (pwstWeapon->dwAmmoInClip > 1) ? 2 : pwstWeapon->dwAmmoInClip;

	#if VER_LR
	aimSync.m_bKeyboardOpened = CKeyBoard::m_bEnable;
	#endif

    RakNet::BitStream bsAimSync;
    bsAimSync.Write((char)ID_AIM_SYNC);
    bsAimSync.Write((char*)&aimSync, sizeof(AIM_SYNC_DATA));
    pNetGame->GetRakClient()->Send(&bsAimSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}

void CLocalPlayer::ProcessSpectating()
{
	CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

	RakNet::BitStream bsSpectatorSync;
	SPECTATOR_SYNC_DATA spSync;

	uint16_t lrAnalog, udAnalog;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog);


	spSync.vecPos = TheCamera.m_mCameraMatrix.GetPosition();

	spSync.lrAnalog = lrAnalog;
	spSync.udAnalog = udAnalog;
	spSync.wKeys = wKeys;

	if((GetTickCount() - m_dwLastSendSpecTick) > GetOptimumOnFootSendRate())
	{
		m_dwLastSendSpecTick = GetTickCount();
		bsSpectatorSync.Write((uint8_t)ID_SPECTATOR_SYNC);
		bsSpectatorSync.Write((char*)&spSync, sizeof(SPECTATOR_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsSpectatorSync, HIGH_PRIORITY, UNRELIABLE, 0);

		if((GetTickCount() - m_dwLastSendAimTick) > (GetOptimumOnFootSendRate() * 2))
		{
			m_dwLastSendAimTick = GetTickCount();
			
		}
	}

	m_pPlayerPed->SetHealth(100.0f);
	m_pPlayerPed->m_pPed->Teleport(
            CVector(spSync.vecPos.x, spSync.vecPos.y, spSync.vecPos.z + 20.0f),
            false
            );

	// handle spectate player left the server
	if(m_byteSpectateType == SPECTATE_TYPE_PLAYER)
	{
		if(!CPlayerPool::GetAt(m_SpectateID)) {
			m_byteSpectateType = SPECTATE_TYPE_NONE;
			m_bSpectateProcessed = false;
			return;
		}
	}

	// handle spectate player is no longer active (ie Died)
	if(m_byteSpectateType == SPECTATE_TYPE_PLAYER &&
			CPlayerPool::GetAt(m_SpectateID) &&
		(!CPlayerPool::GetSpawnedPlayer(m_SpectateID) ||
				CPlayerPool::GetAt(m_SpectateID)->GetState() == PLAYER_STATE_WASTED))
	{
		m_byteSpectateType = SPECTATE_TYPE_NONE;
		m_bSpectateProcessed = false;
	}

	if(m_bSpectateProcessed) return;

	if(m_byteSpectateType == SPECTATE_TYPE_NONE)
	{
		GetPlayerPed()->m_pPed->RemoveFromVehicle();
        CCamera::SetPosition(50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f);
        CCamera::LookAtPoint(60.0f, 60.0f, 50.0f, 2);
		m_bSpectateProcessed = true;
	}
	else if(m_byteSpectateType == SPECTATE_TYPE_PLAYER)
	{
		CPedSamp *pPlayerPed = nullptr;

		if(CPlayerPool::GetSpawnedPlayer(m_SpectateID))
		{
			pPlayerPed = CPlayerPool::GetAt(m_SpectateID)->GetPlayerPed();
			if(pPlayerPed)
			{
                CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

                TheCamera.TakeControl(pPlayerPed->m_pPed, static_cast<eCamMode>(m_byteSpectateMode), eSwitchType::JUMPCUT, 1);
				m_bSpectateProcessed = true;
			}
		}
	}
	else if(m_byteSpectateType == SPECTATE_TYPE_VEHICLE) {
        CVehicleSamp *pVehicle = nullptr;

        pVehicle = CVehiclePool::GetAt((VEHICLEID) m_SpectateID);
        if (pVehicle) {
            CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

            TheCamera.TakeControl(pVehicle->m_pVehicle, static_cast<eCamMode>(m_byteSpectateMode), eSwitchType::JUMPCUT, 1);
            m_bSpectateProcessed = true;
        }
    }
}

void CLocalPlayer::ToggleSpectating(bool bToggle)
{
	m_bIsSpectating = bToggle;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = 0xFFFFFFFF;
	m_bSpectateProcessed = false;
}

void CLocalPlayer::SpectatePlayer(PLAYERID playerId)
{
	if(CPlayerPool::GetSpawnedPlayer(playerId))
	{
		if(CPlayerPool::GetAt(playerId)->GetState() != PLAYER_STATE_NONE &&
				CPlayerPool::GetAt(playerId)->GetState() != PLAYER_STATE_WASTED)
		{
			m_byteSpectateType = SPECTATE_TYPE_PLAYER;
			m_SpectateID = playerId;
			m_bSpectateProcessed = false;
		}
	}
}

void CLocalPlayer::SpectateVehicle(VEHICLEID VehicleID)
{
	if (CVehiclePool::GetAt(VehicleID))
	{
		m_byteSpectateType = SPECTATE_TYPE_VEHICLE;
		m_SpectateID = VehicleID;
		m_bSpectateProcessed = false;
	}
}

