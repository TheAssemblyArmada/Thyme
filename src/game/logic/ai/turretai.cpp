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
#include "turretai.h"
#include "aiupdate.h"
#include "audiomanager.h"
#include "gamelogic.h"
#include "partitionmanager.h"
#include "randomvalue.h"
#include "terrainlogic.h"

StateReturnType Frame_To_Sleep_Time(unsigned int frame1, unsigned int frame2, unsigned int frame3, unsigned int frame4)
{
    if (frame1 > frame2) {
        frame1 = frame2;
    }

    if (frame1 > frame3) {
        frame1 = frame3;
    }

    if (frame1 > frame4) {
        frame1 = frame4;
    }

    unsigned int frame = g_theGameLogic->Get_Frame();

    if (frame1 > frame) {
        return static_cast<StateReturnType>(frame1 - frame);
    } else {
        return STATE_CONTINUE;
    }
}

TurretState::TurretState(TurretStateMachine *machine, Utf8String name) : State(machine, name) {}

TurretAIAimTurretState::TurretAIAimTurretState(TurretStateMachine *machine) : TurretState(machine, "TurretAIAimTurretState")
{
}

StateReturnType TurretAIAimTurretState::Update()
{
    TurretAI *turret = Get_Turret_AI();
    Object *owner = turret->Get_Owner();
    AIUpdateInterface *owner_update = owner->Get_AI_Update_Interface();

    if (!owner_update) {
        return STATE_FAILURE;
    }

    AIUpdateInterface *target_update = nullptr;
    bool preventing_aim = false;
    Object *target_obj;
    Coord3D target_pos;
    TurretTargetType target = turret->Friend_Get_Turret_Target(target_obj, target_pos, true);
    Object *original_target = target_obj;
    bool not_in_range = false;

    if (target == TARGET_TYPE_NONE) {
        return STATE_FAILURE;
    }

    bool is_goal;
    bool is_able_to_attack;

    if (target != TARGET_TYPE_OBJECT) {
        goto l1;
    }

    is_goal = false;

    if (target_obj != nullptr) {
        if (target_obj == owner_update->Get_Goal_Object()) {
            is_goal = true;
        }
    }

    is_able_to_attack = owner->Is_Able_To_Attack();

    if (is_able_to_attack) {
        CanAttackResult attack =
            owner->Get_Able_To_Attack_Specific_Object(turret->Is_Attack_Forced() ? ATTACK_TYPE_3 : ATTACK_TYPE_2,
                target_obj,
                owner_update->Get_Last_Command_Source(),
                WEAPONSLOT_UNK);
        is_able_to_attack = attack == ATTACK_RESULT_CAN_ATTACK || attack == ATTACK_RESULT_OUT_OF_RANGE;
    }

    not_in_range = !turret->Friend_Is_Any_Weapon_In_Range_Of(target_obj);

    if (target_obj != nullptr && is_able_to_attack && (is_goal || !not_in_range)) {
        if (target_obj->Get_Team() == turret->Get_Victim_Initial_Team()) {
            if (target_obj->Is_KindOf(KINDOF_BRIDGE)) {
                TBridgeAttackInfo info;
                g_theTerrainLogic->Get_Bridge_Attack_Points(target_obj, &info);

                if (g_thePartitionManager->Get_Distance_Squared(owner, &info.m_attackPoint2, FROM_BOUNDINGSPHERE_3D, nullptr)
                    < g_thePartitionManager->Get_Distance_Squared(
                        owner, &info.m_attackPoint1, FROM_BOUNDINGSPHERE_3D, nullptr)) {
                    target_pos = info.m_attackPoint1;
                } else {
                    target_pos = info.m_attackPoint2;
                }
            } else {
                target_pos = *target_obj->Get_Position();
            }

            if (target_obj != nullptr) {
                target_update = target_obj->Get_AI_Update_Interface();
            } else {
                target_update = nullptr;
            }

            if (target_update != nullptr) {
                target_update->Add_Targeter(owner->Get_ID(), true);
            }

            preventing_aim = target_update != nullptr && target_update->Is_Temporarily_Preventing_Aim_Success();
            target_obj = nullptr;
        l1:
            WeaponSlotType slot;
            Weapon *cur_weapon = owner->Get_Current_Weapon(&slot);

            if (cur_weapon == nullptr) {
                captainslog_dbgassert(false, "TurretAIAimTurretState::update - cur_weapon is NULL.");
                return STATE_FAILURE;
            }

            float rate_modifier = 1.0f;
            float relative_angle = g_thePartitionManager->Get_Relative_Angle_2D(owner, &target_pos);
            float desired_angle = relative_angle;
            float sweep_angle = turret->Get_Turret_Fire_Angle_Sweep_For_Weapon_Slot(slot);

            if (sweep_angle > 0.0f && turret->Friend_Is_Sweep_Enabled()) {
                if (turret->Friend_Get_Positive_Sweep()) {
                    desired_angle += sweep_angle;
                } else {
                    desired_angle -= sweep_angle;
                }

                rate_modifier = turret->Get_Turret_Sweep_Speed_Modifier_For_Weapon_Slot(slot);
            }

            bool has_turned_angle = turret->Friend_Turn_Towards_Angle(desired_angle, rate_modifier, 0.035f);

            if (sweep_angle > 0.0f) {
                if (has_turned_angle) {
                    turret->Set_Positive_Sweep(!turret->Friend_Get_Positive_Sweep());
                }

                has_turned_angle =
                    sweep_angle > GameMath::Fabs(Normalize_Angle(relative_angle - turret->Get_Turret_Angle()));
            }

            bool has_turned_pitch = true;

            if (turret->Is_Allows_Pitch()) {
                float pitch = 0.0f;

                if (turret->Get_Fire_Pitch() > 0.0f) {
                    pitch = turret->Get_Fire_Pitch();
                } else {
                    Coord3D vec;
                    g_thePartitionManager->Get_Vector_To(owner, &target_pos, FROM_CENTER_3D, vec);
                    vec.z = vec.z - owner->Get_Geometry_Info().Get_Max_Height_Above_Position() / 2.0f;
                    float pitch2;

                    if (vec.Length() > 0.0f) {
                        pitch2 = GameMath::Asin(vec.z / vec.Length());
                    } else {
                        pitch2 = 0.0f;
                    }

                    pitch = pitch2;

                    if (turret->Get_Min_Physical_Pitch() > pitch) {
                        pitch = turret->Get_Min_Physical_Pitch();
                    }

                    if (turret->Get_Ground_Unit_Pitch() > 0.0f) {
                        bool is_ground_target = target_obj == nullptr;

                        if (target_obj != nullptr && target_obj->Is_KindOf(KINDOF_IMMOBILE)) {
                            is_ground_target = true;
                        }

                        if (target_update != nullptr && target_update->Is_Doing_Ground_Movement()) {
                            is_ground_target = true;
                        }

                        if (is_ground_target) {
                            float range = cur_weapon->Get_Attack_Range(owner);
                            float len = vec.Length();

                            if (range < 1.0f) {
                                range = 1.0f;
                            }

                            pitch = pitch2 + turret->Get_Ground_Unit_Pitch() * (len / range);

                            if (turret->Get_Min_Physical_Pitch() > pitch) {
                                pitch = turret->Get_Min_Physical_Pitch();
                            }
                        }
                    }
                }

                has_turned_pitch = turret->Friend_Turn_Towards_Pitch(pitch, 1.0f);
            }

            if (has_turned_angle && has_turned_pitch
                && (original_target != nullptr && cur_weapon->Is_Within_Attack_Range(owner, original_target)
                    || original_target == nullptr && cur_weapon->Is_Within_Attack_Range(owner, &target_pos))) {
                if (preventing_aim || not_in_range) {
                    return STATE_CONTINUE;
                } else {
                    return STATE_SUCCESS;
                }
            } else {
                return STATE_CONTINUE;
            }
        }
    }

    if (turret->Is_Has_Mood_Target()) {
        turret->Set_Turret_Target_Object(nullptr, false);
    }

    return STATE_FAILURE;
}

void TurretAIAimTurretState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

TurretAIRecenterTurretState::TurretAIRecenterTurretState(TurretStateMachine *machine) :
    TurretState(machine, "TurretAIRecenterTurretState")
{
}

StateReturnType TurretAIRecenterTurretState::Update()
{
    if (Get_Machine_Owner()->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        return STATE_CONTINUE;
    }

    TurretAI *turret = Get_Turret_AI();
    bool has_turned_angle = turret->Friend_Turn_Towards_Angle(turret->Get_Natural_Turret_Angle(), 0.5f, 0.0f);
    bool has_turned_pitch = turret->Friend_Turn_Towards_Pitch(turret->Get_Natural_Turret_Pitch(), 0.5f);

    if (has_turned_angle && has_turned_pitch) {
        return STATE_SUCCESS;
    } else {
        return STATE_CONTINUE;
    }
}

void TurretAIRecenterTurretState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

TurretAIIdleState::TurretAIIdleState(TurretStateMachine *machine) :
    TurretState(machine, "TurretAIIdleState"), m_nextIdleScan(0)
{
}

StateReturnType TurretAIIdleState::On_Enter()
{
    AIUpdateInterface *update = Get_Machine_Owner()->Get_AI_Update_Interface();

    if (update != nullptr) {
        update->Reset_Next_Mood_Check_Time();

        if (update->Get_Current_Turret() == Get_Turret_AI()->Get_Which_Turret()) {
            update->Set_Current_Turret(TURRET_INVALID);
        }
    }

    Reset_Idle_Scan();

    return Frame_To_Sleep_Time(Get_Turret_AI()->Friend_Get_Next_Idle_Mood_Target_Frame(),
        m_nextIdleScan,
        UPDATE_SLEEP_TIME_MAX,
        UPDATE_SLEEP_TIME_MAX);
}

StateReturnType TurretAIIdleState::Update()
{
    if (g_theGameLogic->Get_Frame() >= m_nextIdleScan) {
        return STATE_FAILURE;
    }

    TurretAI *turret = Get_Turret_AI();
    turret->Friend_Check_For_Idle_Mood_Target();
    return Frame_To_Sleep_Time(
        turret->Friend_Get_Next_Idle_Mood_Target_Frame(), m_nextIdleScan, UPDATE_SLEEP_TIME_MAX, UPDATE_SLEEP_TIME_MAX);
}

void TurretAIIdleState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUnsignedInt(&m_nextIdleScan);
}

void TurretAIIdleState::Reset_Idle_Scan()
{
    unsigned int frame = g_theGameLogic->Get_Frame();
    unsigned int max = Get_Turret_AI()->Get_Max_Idle_Scan_Interval();
    unsigned int min = Get_Turret_AI()->Get_Min_Idle_Scan_Interval();
    m_nextIdleScan = Get_Logic_Random_Value(min, max) + frame;
}

TurretAIIdleScanState::TurretAIIdleScanState(TurretStateMachine *machine) :
    TurretState(machine, "TurretAIIdleScanState"), m_desiredAngle(0.0f)
{
}

StateReturnType TurretAIIdleScanState::On_Enter()
{
    float min = Get_Turret_AI()->Get_Min_Idle_Scan_Angle();
    float max = Get_Turret_AI()->Get_Max_Idle_Scan_Angle();

    if (min == 0.0f && max == 0.0f) {
        return STATE_SUCCESS;
    }

    m_desiredAngle = Get_Logic_Random_Value_Real(0.0f, max - min) + min;

    if (Get_Logic_Random_Value(0, 1) == 0) {
        m_desiredAngle = -m_desiredAngle;
    }

    return STATE_CONTINUE;
}

StateReturnType TurretAIIdleScanState::Update()
{
    if (Get_Machine_Owner()->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        return STATE_CONTINUE;
    }

    TurretAI *turret = Get_Turret_AI();
    bool angle = turret->Friend_Turn_Towards_Angle(turret->Get_Natural_Turret_Angle() + m_desiredAngle, 0.5f, 0.0f);
    bool pitch = turret->Friend_Turn_Towards_Pitch(turret->Get_Natural_Turret_Pitch(), 0.5f);

    if (angle && pitch) {
        return STATE_SUCCESS;
    } else {
        return STATE_CONTINUE;
    }
}

void TurretAIIdleScanState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferReal(&m_desiredAngle);
}

TurretAIHoldTurretState::TurretAIHoldTurretState(TurretStateMachine *machine) :
    TurretState(machine, "AIHoldTurretState"), m_nextRecenterTime(0)
{
}

StateReturnType TurretAIHoldTurretState::On_Enter()
{
    m_nextRecenterTime = Get_Turret_AI()->Get_Recenter_Time() + g_theGameLogic->Get_Frame();
    return Frame_To_Sleep_Time(Get_Turret_AI()->Friend_Get_Next_Idle_Mood_Target_Frame(),
        m_nextRecenterTime,
        UPDATE_SLEEP_TIME_MAX,
        UPDATE_SLEEP_TIME_MAX);
}

StateReturnType TurretAIHoldTurretState::Update()
{
    if (g_theGameLogic->Get_Frame() >= m_nextRecenterTime) {
        return STATE_SUCCESS;
    }

    TurretAI *turret = Get_Turret_AI();
    turret->Friend_Check_For_Idle_Mood_Target();
    return Frame_To_Sleep_Time(
        turret->Friend_Get_Next_Idle_Mood_Target_Frame(), m_nextRecenterTime, UPDATE_SLEEP_TIME_MAX, UPDATE_SLEEP_TIME_MAX);
}

void TurretAIHoldTurretState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUnsignedInt(&m_nextRecenterTime);
}

void TurretAI::Set_Turret_Target_Object(Object *victim, bool force_attack)
{
    if (victim == nullptr || victim->Is_Effectively_Dead() || !Is_Owners_Cur_Weapon_On_Turret()) {
        if (!Get_Owner()->Get_AI_Update_Interface()->Are_Turrets_Linked()) {
            victim = nullptr;
        }
    }

    if (victim == nullptr) {
        Remove_Self_As_Targeter();
    }

    m_turretStateMachine->Set_Goal_Object(victim);
    m_targetType = victim != nullptr ? TARGET_TYPE_OBJECT : TARGET_TYPE_NONE;
    m_hasMoodTarget = false;
    m_attackForced = force_attack;

    unsigned int id = m_turretStateMachine->Get_Current_State_ID();

    if (victim != nullptr) {
        if (id != TURRET_AIM_TURRET && id != TURRET_ATTACK_FIRE) {
            m_turretStateMachine->Set_State(TURRET_AIM_TURRET);
        }

        m_victimInitialTeam = victim->Get_Team();
    } else {
        if (id == TURRET_AIM_TURRET || id == TURRET_ATTACK_FIRE) {
            m_turretStateMachine->Set_State(TURRET_HOLD_TURRET);
        }

        m_victimInitialTeam = nullptr;
    }
}

void TurretAI::Friend_Check_For_Idle_Mood_Target()
{
    Object *owner = Get_Owner();
    AIUpdateInterface *update = owner->Get_AI_Update_Interface();

    if ((update->Get_Mood_Matrix_Action_Adjustment(MM_ACTION_IDLE) & 0x10) == 0) {
        Object *target = update->Get_Next_Mood_Target(true, true);

        if (target != nullptr) {
            Set_Turret_Target_Object(target, false);
            owner->Choose_Best_Weapon_For_Target(target, PREFER_MOST_DAMAGE, COMMANDSOURCE_AI);
            m_hasMoodTarget = true;
        }
    }
}

unsigned int TurretAI::Friend_Get_Next_Idle_Mood_Target_Frame() const
{
    const AIUpdateInterface *update = Get_Owner()->Get_AI_Update_Interface();

    if (update != nullptr) {
        return update->Get_Next_Mood_Check_Time();
    } else {
        return g_theGameLogic->Get_Frame();
    }
}

float TurretAI::Get_Turret_Sweep_Speed_Modifier_For_Weapon_Slot(WeaponSlotType wslot) const
{
    return m_data->m_turretSweepSpeed[wslot];
}

float TurretAI::Get_Turret_Fire_Angle_Sweep_For_Weapon_Slot(WeaponSlotType wslot) const
{
    return m_data->m_turretSweep[wslot];
}

TurretTargetType TurretAI::Friend_Get_Turret_Target(Object *&victim_obj, Coord3D &victim_pos, bool reset_if_dead) const
{
    victim_obj = nullptr;
    victim_pos.Zero();

    if (m_targetType == TARGET_TYPE_OBJECT) {
        victim_obj = m_turretStateMachine->Get_Goal_Object();

        if (reset_if_dead && (victim_obj == nullptr || victim_obj->Is_Effectively_Dead())) {
            m_turretStateMachine->Set_Goal_Object(nullptr);
            m_targetType = TARGET_TYPE_NONE;
            m_hasMoodTarget = false;
        }
    } else if (m_targetType == TARGET_TYPE_POSITION) {
        victim_obj = nullptr;
        victim_pos = *m_turretStateMachine->Get_Goal_Position();
    }

    return m_targetType;
}

bool TurretAI::Friend_Turn_Towards_Pitch(float desired_pitch, float rate_modifier)
{
    if (!Is_Allows_Pitch()) {
        return true;
    }

    float desired = Normalize_Angle(desired_pitch);
    float pitch = Get_Turret_Pitch();
    float rate = Get_Pitch_Rate() * rate_modifier;
    float diff = Normalize_Angle(desired - pitch);

    if (rate > GameMath::Fabs(diff)) {
        m_pitch = Normalize_Angle(desired);
    } else {
        if (diff > 0.0f) {
            pitch += rate;
        } else {
            pitch -= rate;
        }

        m_playPitchSound = true;
        m_pitch = Normalize_Angle(pitch);
    }

    return m_pitch == desired;
}

bool TurretAI::Friend_Turn_Towards_Angle(float desired_angle, float rate_modifier, float min_turn_angle)
{
    float desired = Normalize_Angle(desired_angle);
    float angle = Get_Turret_Angle();
    float rate = Get_Turn_Rate() * rate_modifier;
    float diff = Normalize_Angle(desired - angle);
    float new_angle;

    if (rate > GameMath::Fabs(diff)) {
        new_angle = desired;
        Get_Owner()->Clear_Model_Condition_State(MODELCONDITION_TURRET_ROTATE);
    } else {
        if (diff > 0.0f) {
            new_angle = angle + rate;
        } else {
            new_angle = angle - rate;
        }

        Get_Owner()->Set_Model_Condition_State(MODELCONDITION_TURRET_ROTATE);
        m_playRotSound = true;
    }

    m_angle = Normalize_Angle(new_angle);

    if (m_angle != angle) {
        Get_Owner()->React_To_Turret(m_whichTurret, angle, m_pitch);
    }

    return min_turn_angle >= GameMath::Fabs(m_angle - desired);
}

bool TurretAI::Friend_Is_Sweep_Enabled() const
{
    return m_unkFrame1 != 0 && m_unkFrame1 > g_theGameLogic->Get_Frame();
}

bool TurretAI::Friend_Is_Any_Weapon_In_Range_Of(const Object *obj) const
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        const Weapon *weapon = Get_Owner()->Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i));

        if (weapon != nullptr) {
            if (Is_Weapon_Slot_On_Turret(static_cast<WeaponSlotType>(i))) {
                if (weapon->Is_Within_Attack_Range(Get_Owner(), obj)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void TurretAI::Remove_Self_As_Targeter()
{
    if (m_targetType == TARGET_TYPE_OBJECT) {
        if (m_turretStateMachine != nullptr) {
            Object *goal = m_turretStateMachine->Get_Goal_Object();

            if (m_owner != nullptr && goal != nullptr) {
                AIUpdateInterface *update = goal->Get_AI_Update_Interface();

                if (update != nullptr) {
                    update->Add_Targeter(m_owner->Get_ID(), false);
                }
            }
        }
    }
}

bool TurretAI::Is_Weapon_Slot_On_Turret(WeaponSlotType wslot) const
{
    return ((1 << wslot) & m_data->m_turretWeaponSlots) != 0;
}

bool TurretAI::Is_Owners_Cur_Weapon_On_Turret() const
{
    WeaponSlotType wslot;
    return m_owner->Get_Current_Weapon(&wslot) && Is_Weapon_Slot_On_Turret(wslot);
}

void TurretAI::Friend_Notify_State_Machine_Changed()
{
    m_sleepUntil = g_theGameLogic->Get_Frame();
}

void TurretAI::Recenter_Turret()
{
    m_turretStateMachine->Set_State(TURRET_RECENTER_TURRET);
}

TurretStateMachine::TurretStateMachine(TurretAI *tai, Object *obj, Utf8String name) :
    StateMachine(obj, name), m_turretAI(tai)
{
    static StateConditionInfo fire_conditions[2] = {
        StateConditionInfo(Out_Of_Weapon_Range_Object, TURRET_AIM_TURRET, nullptr), StateConditionInfo(nullptr, 0, nullptr)
    };

    Define_State(TURRET_IDLE, new TurretAIIdleState(this), TURRET_IDLE, TURRET_IDLE_SCAN, nullptr);
    Define_State(TURRET_IDLE_SCAN, new TurretAIIdleScanState(this), TURRET_HOLD_TURRET, TURRET_HOLD_TURRET, nullptr);
    Define_State(TURRET_AIM_TURRET, new TurretAIAimTurretState(this), TURRET_ATTACK_FIRE, TURRET_HOLD_TURRET, nullptr);
    Define_State(
        TURRET_ATTACK_FIRE, new AIAttackFireWeaponState(this, tai), TURRET_AIM_TURRET, TURRET_AIM_TURRET, fire_conditions);
    Define_State(TURRET_RECENTER_TURRET, new TurretAIRecenterTurretState(this), TURRET_IDLE, TURRET_IDLE, nullptr);
    Define_State(
        TURRET_HOLD_TURRET, new TurretAIHoldTurretState(this), TURRET_RECENTER_TURRET, TURRET_RECENTER_TURRET, nullptr);
}

void TurretStateMachine::Clear()
{
    StateMachine::Clear();
    TurretAI *turret = Get_Turret_AI();

    if (turret != nullptr) {
        turret->Friend_Notify_State_Machine_Changed();
    }
}

StateReturnType TurretStateMachine::Reset_To_Default_State()
{
    StateReturnType ret = StateMachine::Reset_To_Default_State();
    TurretAI *turret = Get_Turret_AI();

    if (turret != nullptr) {
        turret->Friend_Notify_State_Machine_Changed();
    }

    return ret;
}

StateReturnType TurretStateMachine::Set_State(unsigned int new_state_id)
{
    unsigned int id = Get_Current_State_ID();
    StateReturnType ret = StateMachine::Set_State(new_state_id);
    TurretAI *turret = Get_Turret_AI();

    if (turret != nullptr && id != new_state_id) {
        turret->Friend_Notify_State_Machine_Changed();
    }

    return ret;
}

void TurretStateMachine::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

TurretAI::TurretAI(Object *owner, const TurretAIData *data, WhichTurretType tur) :
    m_data(data),
    m_whichTurret(tur),
    m_owner(owner),
    m_turretStateMachine(nullptr),
    m_unkFrame1(0),
    m_victimInitialTeam(nullptr),
    m_targetType(TARGET_TYPE_NONE),
    m_unkFrame2(-1),
    m_sleepUntil(0),
    m_playRotSound(false),
    m_playPitchSound(false),
    m_positiveSweep(true),
    m_didFire(false),
    m_enabled(!data->m_initiallyDisabled),
    m_firesWhileTurning(data->m_firesWhileTurning),
    m_attackForced(false),
    m_hasMoodTarget(false)
{
    captainslog_relassert(m_data != nullptr, CODE_06, "TurretAI MUST have ModuleData");
    captainslog_relassert(m_data->m_turretWeaponSlots != 0, CODE_06, "TurretAI MUST specify controlled weapon slots!");
    m_angle = Get_Natural_Turret_Angle();
    m_pitch = Get_Natural_Turret_Pitch();
    m_turretStateMachine = new TurretStateMachine(this, m_owner, "TurretStateMachine");
    m_turretStateMachine->Init_Default_State();
    m_turretRotOrPitchSound = *m_owner->Get_Template()->Get_Per_Unit_Sound("TurretMoveLoop");
}

TurretAI::~TurretAI()
{
    Stop_Rot_Or_Pitch_Sound();

    if (m_turretStateMachine != nullptr) {
        m_turretStateMachine->Delete_Instance();
    }
}

void TurretAI::Notify_Fired()
{
    m_didFire = true;
}

void TurretAI::Notify_New_Victim_Chosen(Object *victim)
{
    Set_Turret_Target_Object(victim, false);
}

bool TurretAI::Is_Weapon_Slot_Ok_To_Fire(WeaponSlotType wslot) const
{
    return Get_Owner()->Get_AI_Update_Interface()->Are_Turrets_Linked() || Is_Weapon_Slot_On_Turret(wslot);
}

void TurretAI::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 2;
    xfer->xferVersion(&version, 2);
    xfer->xferSnapshot(m_turretStateMachine);
    xfer->xferReal(&m_angle);
    xfer->xferReal(&m_pitch);
    xfer->xferUnsignedInt(&m_unkFrame1);
    xfer->xferUser(&m_targetType, sizeof(m_targetType));
    xfer->xferUnsignedInt(&m_unkFrame2);
    bool b = m_playRotSound;
    xfer->xferBool(&b);
    m_playRotSound = b;
    b = m_playPitchSound;
    xfer->xferBool(&b);
    m_playPitchSound = b;
    b = m_positiveSweep;
    xfer->xferBool(&b);
    m_positiveSweep = b;
    b = m_didFire;
    xfer->xferBool(&b);
    m_didFire = b;
    b = m_enabled;
    xfer->xferBool(&b);
    m_enabled = b;
    b = m_firesWhileTurning;
    xfer->xferBool(&b);
    m_firesWhileTurning = b;
    b = m_hasMoodTarget;
    xfer->xferBool(&b);
    m_hasMoodTarget = b;

    if (version >= 2) {
        xfer->xferUnsignedInt(&m_sleepUntil);
    }
}

void TurretAI::Load_Post_Process()
{
    const Object *obj = m_turretStateMachine->Get_Goal_Object();

    if (obj != nullptr) {
        m_victimInitialTeam = obj->Get_Team();
    }
}

bool TurretAI::Is_Trying_To_Aim_At_Target(const Object *victim) const
{
    Object *target_obj;
    Coord3D target_pos;
    return m_turretStateMachine->Get_Current_State_ID() == TURRET_AIM_TURRET
        && Friend_Get_Turret_Target(target_obj, target_pos, true) == TARGET_TYPE_OBJECT && target_obj == victim;
}

void TurretAI::Set_Turret_Target_Position(const Coord3D *victim_pos)
{
    if (victim_pos == nullptr || !Is_Owners_Cur_Weapon_On_Turret()) {
        if (!Get_Owner()->Get_AI_Update_Interface()->Are_Turrets_Linked()) {
            victim_pos = nullptr;
        }
    }

    Remove_Self_As_Targeter();

    m_turretStateMachine->Set_Goal_Object(nullptr);

    if (victim_pos != nullptr) {
        m_turretStateMachine->Set_Goal_Position(victim_pos);
    }

    m_targetType = victim_pos != nullptr ? TARGET_TYPE_POSITION : TARGET_TYPE_NONE;
    m_hasMoodTarget = false;

    unsigned int id = m_turretStateMachine->Get_Current_State_ID();

    if (victim_pos != nullptr) {
        if (id != TURRET_AIM_TURRET && id != TURRET_ATTACK_FIRE) {
            m_turretStateMachine->Set_State(TURRET_AIM_TURRET);
        }

        m_victimInitialTeam = nullptr;
    } else {
        if (id == TURRET_AIM_TURRET || id == TURRET_ATTACK_FIRE) {
            m_turretStateMachine->Set_State(TURRET_HOLD_TURRET);
        }

        m_victimInitialTeam = nullptr;
    }
}

bool TurretAI::Is_Turret_In_Neutral_Position() const
{
    if (Get_Owner()->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        return true;
    }

    if (Get_Turret_Angle() == Get_Natural_Turret_Angle()) {
        if (Get_Turret_Pitch() == Get_Natural_Turret_Pitch()) {
            return true;
        }
    }

    return false;
}

UpdateSleepTime TurretAI::Update_Turret_AI()
{
    captainslog_dbgassert(!m_enabled || m_turretStateMachine->Peek_Sleep_Till() == 0
            || m_turretStateMachine->Peek_Sleep_Till() >= m_sleepUntil,
        "Turret Machine is less sleepy than turret");
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (m_sleepUntil != 0 && frame < m_sleepUntil) {
        return static_cast<UpdateSleepTime>(m_sleepUntil - frame);
    }

    UpdateSleepTime time = UPDATE_SLEEP_TIME_MAX;

    if (!m_firesWhileTurning || m_unkFrame2 <= frame) {
        m_playRotSound = false;
        m_playPitchSound = false;
    }

    if (m_enabled || m_turretStateMachine->Get_Current_State_ID() == TURRET_RECENTER_TURRET) {
        m_didFire = false;
        StateReturnType type = m_turretStateMachine->Update_State_Machine();

        if (m_didFire) {
            m_unkFrame1 = frame + 3;
            m_unkFrame2 = frame + 3;
        }

        if (m_playRotSound || m_playPitchSound) {
            Start_Rot_Or_Pitch_Sound();
        } else {
            Stop_Rot_Or_Pitch_Sound();
        }

        if (type > 0) {
            if (static_cast<UpdateSleepTime>(type) < time) {
                time = static_cast<UpdateSleepTime>(type);
            }
        } else {
            time = UPDATE_SLEEP_TIME_MIN;
        }
    }

    m_sleepUntil = time + frame;
    captainslog_dbgassert(!m_enabled || m_turretStateMachine->Peek_Sleep_Till() == 0
            || m_turretStateMachine->Peek_Sleep_Till() >= m_sleepUntil,
        "Turret Machine is less sleepy than turret");
    return time;
}

void TurretAI::Set_Turret_Enabled(bool enable)
{
    if (enable && !m_enabled) {
        m_sleepUntil = g_theGameLogic->Get_Frame();
    }

    m_enabled = enable;
}

void TurretAI::Start_Rot_Or_Pitch_Sound()
{
    if (!m_turretRotOrPitchSound.Is_Currently_Playing()) {
        m_turretRotOrPitchSound.Set_Object_ID(m_owner->Get_ID());
        m_turretRotOrPitchSound.Set_Playing_Handle(g_theAudio->Add_Audio_Event(&m_turretRotOrPitchSound));
    }
}

void TurretAI::Stop_Rot_Or_Pitch_Sound()
{
    if (m_turretRotOrPitchSound.Is_Currently_Playing()) {
        g_theAudio->Remove_Audio_Event(m_turretRotOrPitchSound.Get_Playing_Handle());
    }
}

TurretAIData::TurretAIData() :
    m_turnRate(0.01f),
    m_pitchRate(0.01f),
    m_naturalTurretAngle(0.0f),
    m_naturalTurretPitch(0.0f),
    m_firePitch(0.0f),
    m_minPhysicalPitch(0.0f),
    m_groundUnitPitch(0.0f),
    m_turretWeaponSlots(0),
    m_minIdleScanAngle(0.0f),
    m_maxIdleScanAngle(0.0f),
    m_minIdleScanInterval(9999999),
    m_maxIdleScanInterval(9999999),
    m_recenterTime(60),
    m_initiallyDisabled(false),
    m_firesWhileTurning(false),
    m_isAllowsPitch(false)
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        m_turretSweep[i] = 0.0f;
        m_turretSweepSpeed[i] = 1.0f;
    }
}

static const char *s_theWeaponSlotTypeNames[] = { "PRIMARY", "SECONDARY", "TERTIARY", nullptr };

void TurretAIData::Parse_Turret_Sweep(INI *ini, void *instance, void *store, const void *user_data)
{
    int index = ini->Scan_IndexList(ini->Get_Next_Token(), s_theWeaponSlotTypeNames);
    INI::Parse_Angle_Real(
        ini, instance, static_cast<void *>(&static_cast<TurretAIData *>(store)->m_turretSweep[index]), nullptr);
}

void TurretAIData::Parse_Turret_Sweep_Speed(INI *ini, void *instance, void *store, const void *user_data)
{
    int index = ini->Scan_IndexList(ini->Get_Next_Token(), s_theWeaponSlotTypeNames);
    INI::Parse_Real(
        ini, instance, static_cast<void *>(&static_cast<TurretAIData *>(store)->m_turretSweepSpeed[index]), nullptr);
}

void Parse_TWS(INI *ini, void *instance, void *store, const void *user_data)
{
    for (const char *str = ini->Get_Next_Token(); str != nullptr; str = ini->Get_Next_Token_Or_Null()) {
        *static_cast<unsigned int *>(store) |= 1 << ini->Scan_IndexList(str, s_theWeaponSlotTypeNames);
    }
}

void TurretAIData::Build_Field_Parse(MultiIniFieldParse &p)
{
    static const FieldParse dataFieldParse[] = {
        { "TurretTurnRate", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(TurretAIData, m_turnRate) },
        { "TurretPitchRate", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(TurretAIData, m_pitchRate) },
        { "NaturalTurretAngle", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_naturalTurretAngle) },
        { "NaturalTurretPitch", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_naturalTurretPitch) },
        { "FirePitch", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_firePitch) },
        { "MinPhysicalPitch", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_minPhysicalPitch) },
        { "GroundUnitPitch", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_groundUnitPitch) },
        { "TurretFireAngleSweep", &TurretAIData::Parse_Turret_Sweep, nullptr, 0 },
        { "TurretSweepSpeedModifier", &TurretAIData::Parse_Turret_Sweep_Speed, nullptr, 0 },
        { "ControlledWeaponSlots", &Parse_TWS, nullptr, offsetof(TurretAIData, m_turretWeaponSlots) },
        { "AllowsPitch", &INI::Parse_Bool, nullptr, offsetof(TurretAIData, m_isAllowsPitch) },
        { "MinIdleScanAngle", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_minIdleScanAngle) },
        { "MaxIdleScanAngle", &INI::Parse_Angle_Real, nullptr, offsetof(TurretAIData, m_maxIdleScanAngle) },
        { "MinIdleScanInterval", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TurretAIData, m_minIdleScanInterval) },
        { "MaxIdleScanInterval", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TurretAIData, m_maxIdleScanInterval) },
        { "RecenterTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TurretAIData, m_recenterTime) },
        { "InitiallyDisabled", &INI::Parse_Bool, nullptr, offsetof(TurretAIData, m_initiallyDisabled) },
        { "FiresWhileTurning", &INI::Parse_Bool, nullptr, offsetof(TurretAIData, m_firesWhileTurning) },
        { nullptr, nullptr, nullptr, 0 },
    };

    p.Add(dataFieldParse, 0);
}
