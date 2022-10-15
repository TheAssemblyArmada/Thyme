/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Turret AI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "aistates.h"
#include "statemachine.h"
#include "updatemodule.h"

class TurretAI;

enum TurretTargetType
{
    TARGET_TYPE_NONE = 0,
    TARGET_TYPE_OBJECT,
    TARGET_TYPE_POSITION,
};

enum TurretStateType
{
    TURRET_IDLE,
    TURRET_IDLE_SCAN,
    TURRET_AIM_TURRET,
    TURRET_ATTACK_FIRE,
    TURRET_RECENTER_TURRET,
    TURRET_HOLD_TURRET,
};

class TurretStateMachine : public StateMachine
{
    IMPLEMENT_POOL(TurretStateMachine);

public:
    TurretStateMachine(TurretAI *tai, Object *obj, Utf8String name);
    ~TurretStateMachine() override {}
    virtual void Clear() override;
    virtual StateReturnType Reset_To_Default_State() override;
    virtual StateReturnType Set_State(unsigned int new_state_id) override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    TurretAI *Get_Turret_AI() const { return m_turretAI; }
    unsigned int Peek_Sleep_Till() const { return m_sleepTill; }

private:
    TurretAI *m_turretAI;
};

class TurretAIData : public MemoryPoolObject
{
    IMPLEMENT_POOL(TurretAIData)

public:
    TurretAIData();
    ~TurretAIData() override {}

    static void Parse_Turret_Sweep(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Turret_Sweep_Speed(INI *ini, void *instance, void *store, const void *user_data);
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    float m_turnRate;
    float m_pitchRate;
    float m_naturalTurretAngle;
    float m_naturalTurretPitch;
    float m_turretSweep[WEAPONSLOT_COUNT];
    float m_turretSweepSpeed[WEAPONSLOT_COUNT];
    float m_firePitch;
    float m_minPhysicalPitch;
    float m_groundUnitPitch;
    unsigned int m_turretWeaponSlots;
    float m_minIdleScanAngle;
    float m_maxIdleScanAngle;
    unsigned int m_minIdleScanInterval;
    unsigned int m_maxIdleScanInterval;
    unsigned int m_recenterTime;
    bool m_initiallyDisabled;
    bool m_firesWhileTurning;
    bool m_isAllowsPitch;
    friend class TurretAI;
};

class TurretAI : public MemoryPoolObject, public SnapShot, public NotifyWeaponFiredInterface
{
    IMPLEMENT_POOL(TurretAI)

public:
#ifdef GAME_DLL
    TurretAI *Hook_Ctor(Object *owner, const TurretAIData *data, WhichTurretType tur)
    {
        return new (this) TurretAI(owner, data, tur);
    }
#endif
    TurretAI(Object *owner, const TurretAIData *data, WhichTurretType tur);

    virtual ~TurretAI() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Notify_Fired() override;
    virtual void Notify_New_Victim_Chosen(Object *victim) override;
    virtual bool Is_Weapon_Slot_Ok_To_Fire(WeaponSlotType wslot) const override;
    virtual bool Is_Attacking_Object() const override { return m_targetType == TARGET_TYPE_OBJECT; }
    virtual const Coord3D *Get_Original_Victim_Pos() const override { return nullptr; }

    bool Friend_Turn_Towards_Angle(float desired_angle, float rate_modifier, float unk);
    bool Friend_Turn_Towards_Pitch(float desired_pitch, float rate_modifier);
    TurretTargetType Friend_Get_Turret_Target(Object *&victim_obj, Coord3D &victim_pos, bool reset_if_dead) const;
    void Friend_Notify_State_Machine_Changed();
    bool Friend_Is_Any_Weapon_In_Range_Of(const Object *obj) const;
    bool Friend_Is_Sweep_Enabled() const;
    unsigned int Friend_Get_Next_Idle_Mood_Target_Frame() const;
    void Friend_Check_For_Idle_Mood_Target();

    float Get_Turret_Fire_Angle_Sweep_For_Weapon_Slot(WeaponSlotType wslot) const;
    float Get_Turret_Sweep_Speed_Modifier_For_Weapon_Slot(WeaponSlotType wslot) const;

    bool Is_Owners_Cur_Weapon_On_Turret() const;
    bool Is_Weapon_Slot_On_Turret(WeaponSlotType wslot) const;
    bool Is_Turret_In_Neutral_Position() const;
    bool Is_Trying_To_Aim_At_Target(const Object *victim) const;

    void Remove_Self_As_Targeter();
    void Recenter_Turret();
    UpdateSleepTime Update_Turret_AI();
    void Start_Rot_Or_Pitch_Sound();
    void Stop_Rot_Or_Pitch_Sound();

    void Set_Turret_Target_Object(Object *victim, bool force_attack);
    void Set_Turret_Target_Position(const Coord3D *victim_pos);
    void Set_Turret_Enabled(bool enable);

    float Get_Turret_Angle() const { return m_angle; }
    float Get_Turret_Pitch() const { return m_pitch; }
    float Get_Turn_Rate() const { return m_data->m_turnRate; }
    float Get_Natural_Turret_Angle() const { return m_data->m_naturalTurretAngle; }
    float Get_Natural_Turret_Pitch() const { return m_data->m_naturalTurretPitch; }
    const Object *Get_Owner() const { return m_owner; }
    Object *Get_Owner() { return m_owner; }
    float Get_Pitch_Rate() const { return m_data->m_pitchRate; }
    float Get_Min_Physical_Pitch() const { return m_data->m_minPhysicalPitch; }
    float Get_Fire_Pitch() const { return m_data->m_firePitch; }
    float Get_Ground_Unit_Pitch() const { return m_data->m_groundUnitPitch; }
    const Team *Get_Victim_Initial_Team() { return m_victimInitialTeam; }
    unsigned int Get_Min_Idle_Scan_Interval() const { return m_data->m_minIdleScanInterval; }
    unsigned int Get_Max_Idle_Scan_Interval() const { return m_data->m_maxIdleScanInterval; }
    WhichTurretType Get_Which_Turret() const { return m_whichTurret; }
    float Get_Min_Idle_Scan_Angle() const { return m_data->m_minIdleScanAngle; }
    float Get_Max_Idle_Scan_Angle() const { return m_data->m_maxIdleScanAngle; }
    unsigned int Get_Recenter_Time() const { return m_data->m_recenterTime; }

    bool Is_Enabled() const { return m_enabled; }
    bool Is_Allows_Pitch() const { return m_data->m_isAllowsPitch; }
    bool Is_Attack_Forced() const { return m_attackForced; }
    bool Friend_Get_Positive_Sweep() const { return m_positiveSweep; }
    bool Is_Has_Mood_Target() const { return m_hasMoodTarget; }

    void Set_Positive_Sweep(bool set) { m_positiveSweep = set; }

private:
    const TurretAIData *m_data;
    WhichTurretType m_whichTurret;
    Object *m_owner;
    TurretStateMachine *m_turretStateMachine;
    float m_angle;
    float m_pitch;
    AudioEventRTS m_turretRotOrPitchSound;
    unsigned int m_unkFrame1;
    const Team *m_victimInitialTeam;
    mutable TurretTargetType m_targetType;
    unsigned int m_unkFrame2;
    unsigned int m_sleepUntil;
    bool m_playRotSound : 1;
    bool m_playPitchSound : 1;
    bool m_positiveSweep : 1;
    bool m_didFire : 1;
    bool m_enabled : 1;
    bool m_firesWhileTurning : 1;
    bool m_attackForced : 1;
    mutable bool m_hasMoodTarget : 1;
};

class TurretState : public State
{
    IMPLEMENT_ABSTRACT_POOL(TurretState)

public:
    TurretState(TurretStateMachine *machine, Utf8String name);

    virtual ~TurretState() override {}

    TurretAI *Get_Turret_AI() { return static_cast<TurretStateMachine *>(Get_Machine())->Get_Turret_AI(); }
};

class TurretAIAimTurretState : public TurretState
{
    IMPLEMENT_POOL(TurretAIAimTurretState)

public:
    TurretAIAimTurretState(TurretStateMachine *machine);

    virtual ~TurretAIAimTurretState() override {}
    virtual StateReturnType On_Enter() override { return STATE_CONTINUE; }
    virtual void On_Exit(StateExitType status) override {}
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
};

class TurretAIRecenterTurretState : public TurretState
{
    IMPLEMENT_POOL(TurretAIRecenterTurretState)

public:
    TurretAIRecenterTurretState(TurretStateMachine *machine);

    virtual ~TurretAIRecenterTurretState() override {}
    virtual StateReturnType On_Enter() override { return STATE_CONTINUE; }
    virtual void On_Exit(StateExitType status) override {}
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
};

class TurretAIIdleState : public TurretState
{
    IMPLEMENT_POOL(TurretAIIdleState)

public:
    TurretAIIdleState(TurretStateMachine *machine);

    virtual ~TurretAIIdleState() override {}
    virtual StateReturnType On_Enter() override;
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset_Idle_Scan();

private:
    unsigned int m_nextIdleScan;
};

class TurretAIIdleScanState : public TurretState
{
    IMPLEMENT_POOL(TurretAIIdleScanState)

public:
    TurretAIIdleScanState(TurretStateMachine *machine);

    virtual ~TurretAIIdleScanState() override {}
    virtual StateReturnType On_Enter() override;
    virtual void On_Exit(StateExitType status) override {}
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    float m_desiredAngle;
};

class TurretAIHoldTurretState : public TurretState
{
    IMPLEMENT_POOL(TurretAIHoldTurretState)

public:
    TurretAIHoldTurretState(TurretStateMachine *machine);

    virtual ~TurretAIHoldTurretState() override {}
    virtual StateReturnType On_Enter() override;
    virtual void On_Exit(StateExitType status) override {}
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    unsigned int m_nextRecenterTime;
};
