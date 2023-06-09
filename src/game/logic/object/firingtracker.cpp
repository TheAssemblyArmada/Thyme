/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Firing Tracker
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "firingtracker.h"
#include "audiomanager.h"
#include "gamelogic.h"

FiringTracker::FiringTracker(Thing *thing, const ModuleData *module_data) :
    UpdateModule(thing, module_data),
    m_consecutiveShots(0),
    m_victimID(INVALID_OBJECT_ID),
    m_fireFrame(0),
    m_idleFrame(0),
    m_fireSoundFrame(0),
    m_audioEvent(1)
{
    Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MAX);
}

FiringTracker::~FiringTracker()
{
    g_theAudio->Remove_Audio_Event(m_audioEvent);
    m_audioEvent = 1;
}

NameKeyType FiringTracker::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("FiringTracker");
    return _key;
}

SleepyUpdatePhase FiringTracker::Get_Update_Phase() const
{
    return SLEEPY_UPDATE_PHASE_3;
}

void FiringTracker::CRC_Snapshot(Xfer *xfer)
{
    UpdateModule::CRC_Snapshot(xfer);
}

void FiringTracker::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    UpdateModule::Xfer_Snapshot(xfer);
    xfer->xferInt(&m_consecutiveShots);
    xfer->xferObjectID(&m_victimID);
    xfer->xferUnsignedInt(&m_fireFrame);
}

void FiringTracker::Load_Post_Process()
{
    UpdateModule::Load_Post_Process();
}

BitFlags<DISABLED_TYPE_COUNT> FiringTracker::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

UpdateSleepTime FiringTracker::Update()
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (m_idleFrame != 0 && frame >= m_idleFrame) {
        Get_Object()->Reload_All_Ammo(true);
        m_idleFrame = 0;
    }

    if (m_fireSoundFrame != 0 && frame >= m_fireSoundFrame) {
        g_theAudio->Remove_Audio_Event(m_audioEvent);
        m_audioEvent = 1;
        m_fireSoundFrame = 0;
    }

    if (m_fireFrame == 0 || frame <= m_fireFrame) {
        return Calc_Time_To_Sleep();
    }

    m_fireFrame = frame + 30;
    Cool_Down();
    return static_cast<UpdateSleepTime>(30);
}

UpdateSleepTime FiringTracker::Calc_Time_To_Sleep()
{
    if (m_fireSoundFrame == 0 && m_fireFrame == 0 && m_idleFrame == 0) {
        return UPDATE_SLEEP_TIME_MAX;
    }

    unsigned int frame = g_theGameLogic->Get_Frame();
    UpdateSleepTime ret = UPDATE_SLEEP_TIME_MAX;

    if (m_fireSoundFrame != 0) {
        if (m_fireSoundFrame > frame) {
            if (m_fireSoundFrame - frame < UPDATE_SLEEP_TIME_MAX) {
                ret = static_cast<UpdateSleepTime>(m_fireSoundFrame - frame);
            }
        } else {
            ret = UPDATE_SLEEP_TIME_MIN;
        }
    }

    if (m_fireFrame != 0) {
        if (m_fireFrame > frame) {
            if (m_fireFrame - frame < ret) {
                ret = static_cast<UpdateSleepTime>(m_fireFrame - frame);
            }
        } else {
            ret = UPDATE_SLEEP_TIME_MIN;
        }
    }

    if (m_idleFrame != 0) {
        if (m_idleFrame > frame) {
            if (m_idleFrame - frame < ret) {
                return static_cast<UpdateSleepTime>(m_idleFrame - frame);
            }
        } else {
            return UPDATE_SLEEP_TIME_MIN;
        }
    }

    return ret;
}

void FiringTracker::Cool_Down()
{
    BitFlags<MODELCONDITION_COUNT> must_be_clear;
    BitFlags<MODELCONDITION_COUNT> must_be_set;
    Object *obj = Get_Object();

    if (obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST)
        || obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN)) {
        must_be_set.Set(MODELCONDITION_CONTINUOUS_FIRE_SLOW, true);
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST);
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_FAST, true);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_MEAN, true);
    } else {
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST);
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_FAST, true);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_MEAN, true);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_SLOW, true);
    }

    obj->Clear_And_Set_Model_Condition_Flags(must_be_clear, must_be_set);
    m_consecutiveShots = 0;
    m_victimID = INVALID_OBJECT_ID;
}

void FiringTracker::Speed_Up()
{
    BitFlags<MODELCONDITION_COUNT> must_be_clear;
    BitFlags<MODELCONDITION_COUNT> must_be_set;
    Object *obj = Get_Object();

    if (obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST)
        || obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN)) {
        AudioEventRTS sound(*obj->Get_Template()->Get_Per_Unit_Sound("VoiceRapidFire"));
        sound.Set_Object_ID(obj->Get_ID());
        g_theAudio->Add_Audio_Event(&sound);
        obj->Set_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST);
        must_be_set.Set(MODELCONDITION_CONTINUOUS_FIRE_FAST, true);
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_MEAN, true);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_SLOW, true);
    } else {
        obj->Set_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN);
        must_be_set.Set(MODELCONDITION_CONTINUOUS_FIRE_MEAN, true);
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_FAST, true);
        must_be_clear.Set(MODELCONDITION_CONTINUOUS_FIRE_SLOW, true);
    }

    obj->Clear_And_Set_Model_Condition_Flags(must_be_clear, must_be_set);
}

int FiringTracker::Get_Num_Concecutive_Shots_At_Victim(const Object *victim)
{
    if (victim == nullptr) {
        return 0;
    }

    if (victim->Get_ID() == m_victimID) {
        return m_consecutiveShots;
    }

    return 0;
}

void FiringTracker::Shot_Fired(const Weapon *weapon, ObjectID victim)
{
    unsigned int frame = g_theGameLogic->Get_Frame();
    Object *obj = Get_Object();
    Object *victim_obj = g_theGameLogic->Find_Object_By_ID(victim);

    if (victim_obj != nullptr && victim_obj->Get_Status(OBJECT_STATUS_FAERIE_FIRE)) {
        if (!obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE)) {
            obj->Set_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE);
        }
    } else if (obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE)) {
        obj->Clear_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE);
    }

    if (victim == m_victimID) {
        m_consecutiveShots++;
    } else {
        if (frame >= m_fireFrame) {
            m_consecutiveShots = 1;
        } else {
            m_consecutiveShots++;
        }

        m_victimID = victim;
    }

    unsigned int reload = weapon->Get_Auto_Reload_When_Idle();

    if (reload != 0) {
        m_idleFrame = reload + frame;
    }

    unsigned int cost = weapon->Get_Continuous_Fire_Cost_Frames();

    if (cost != 0) {
        m_fireFrame = cost + weapon->Get_Next_Shot();
    } else {
        m_fireFrame = 0;
    }

    int mean_shots_needed = weapon->Get_Continuous_Fire_One_Shots_Needed();
    int fast_shots_needed = weapon->Get_Continuous_Fire_Two_Shots_Needed();

    if (obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN)) {
        if (m_consecutiveShots >= mean_shots_needed) {
            if (m_consecutiveShots > fast_shots_needed) {
                Speed_Up();
            }
        } else {
            Cool_Down();
        }
    } else if (obj->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST)) {
        if (m_consecutiveShots < fast_shots_needed) {
            Cool_Down();
        }
    } else if (m_consecutiveShots > mean_shots_needed) {
        Speed_Up();
    }

    unsigned int loop = weapon->Get_Fire_Sound_Loop_Time();

    if (loop != 0) {
        if (m_fireSoundFrame == 0 || !g_theAudio->Is_Currently_Playing(m_audioEvent)) {
            AudioEventRTS sound(*weapon->Get_Fire_Sound());
            sound.Set_Object_ID(Get_Object()->Get_ID());
            m_audioEvent = g_theAudio->Add_Audio_Event(&sound);
        }

        m_fireSoundFrame = loop + frame;
    } else {
        AudioEventRTS sound(*weapon->Get_Fire_Sound());
        sound.Set_Object_ID(Get_Object()->Get_ID());
        g_theAudio->Add_Audio_Event(&sound);
        m_fireSoundFrame = 0;
    }

    Set_Wake_Frame(obj, Calc_Time_To_Sleep());
}
