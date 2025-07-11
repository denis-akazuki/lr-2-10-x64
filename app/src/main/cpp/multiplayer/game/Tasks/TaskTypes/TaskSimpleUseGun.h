#pragma once

#include "../TaskSimple.h"
#include "Enums/eGunCommand.h"
#include "Animation/AnimBlendAssociation.h"
#include "WeaponInfo.h"

class  CTaskSimpleUseGun : public CTaskSimple {
public:
    static constexpr auto Type = TASK_SIMPLE_USE_GUN;

    CTaskSimpleUseGun(CEntity* targetEntity, CVector vecTarget = {}, eGunCommand nCommand = eGunCommand::AIM, uint16 nBurstLength = 1, bool bAimImmediate = false);
    ~CTaskSimpleUseGun() override;

    CTask* Clone() const override { return new CTaskSimpleUseGun(m_TargetEntity, m_TargetPos, m_LastCmd, m_BurstLength, m_IsAimImmediate); }
    eTaskType GetTaskType() const override { return Type; };
    bool MakeAbortable(CPedGta* ped, eAbortPriority priority = ABORT_PRIORITY_URGENT, const CEvent* event = nullptr) override;
    bool ProcessPed(CPedGta* ped) override;
    bool SetPedPosition(CPedGta* ped) override;

    void AbortIK(CPedGta* ped);
    void AimGun(CPedGta* ped);

    void ClearAnim(CPedGta* ped);

    bool ControlGun(CPedGta* ped, CEntity* target, eGunCommand cmd);
    bool ControlGunMove(const CVector2D& moveSpeed);

    static void FinishGunAnimCB(CAnimBlendAssociation* anim, void* data);

    void FireGun(CPedGta* ped, bool); // Originally returned a bool, but since the return value isn't used anywhere, we're going to make it void...
    bool PlayerPassiveControlGun();
    void RemoveStanceAnims(CPedGta* ped, float x);
    static bool RequirePistolWhip(CPedGta* ped, CEntity* entity);
    void Reset(CPedGta* ped, CEntity* targetEntity, CVector targetPos, eGunCommand, int16 burstLength);

    void SetMoveAnim(CPedGta* ped);
    void StartAnim(CPedGta* ped);
    void StartCountDown(uint8 numIdleFrames, char isMax);

    auto GetLastGunCommand() const { return m_LastCmd; }

#if ANDROID
    void CreateTask();
    void Serialize();
#endif

public:
    static void InjectHooks();

public:
    bool                   m_IsFinished{};
    bool                   m_IsInControl{ true };
    bool                   m_HasMoveControl{};
    bool                   m_HasFiredGun{};
    bool                   m_IsLOSBlocked{};
    bool                   m_IsFiringGunRightHandThisFrame : 1{};
    bool                   m_IsFiringGunLeftHandThisFrame : 1{};
    bool                   m_SkipAim{};                           //!< Don't aim (So no crosshair on HUD, etc)
    eGunCommand            m_NextCmd{};                           //!< Active command
    eGunCommand            m_LastCmd{ eGunCommand::UNKNOWN };     //!< Previous command
    CVector2D              m_MoveCmd{ 0.f, 0.f };          //!< Where we should be moving towards
    CEntity::Ref           m_TargetEntity{};                      //!< Entity we're aiming for (If set, `m_TargetPos` is ignored)
    CVector                m_TargetPos{};                         //!< Either a position, or a direction vector (?) - Only used if `m_TargetEntity` is not set.
    CAnimBlendAssociation* m_Anim{};                              //!< Animation for the current command (Reloading, Firing, etc)
    CWeaponInfo*           m_WeaponInfo{};                        //!< Ped active weapon's info
    uint16                 m_BurstLength{};                       //!< How many bullets to fire in a burst (For eGunCommand::FIREBURST)
    uint16                 m_BurstShots{};                        //!< How many bullets have been fired this burst (For eGunCommand::FIREBURST)
    uint8                  m_CountDownFrames{ 0xFF };             //!< Some countdown (Maybe for burst?)
    bool                   m_IsArmIKInUse{};                      //!< Are we using arm IK (PointArmAt)
    bool                   m_IsLookIKInUse{};                     //!< Are we using look at IK (LookAt)
    bool                   m_IsAimImmediate{};                    //!< How fast the aim animation should be (?)
};
