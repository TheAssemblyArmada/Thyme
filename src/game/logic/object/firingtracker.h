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
#pragma once
#include "always.h"
#include "updatemodule.h"

class FiringTrackerModuleData : public ModuleData
{
public:
    FiringTrackerModuleData() {}
    ~FiringTrackerModuleData() override {}
};

class FiringTracker : public UpdateModule
{
    IMPLEMENT_NAMED_POOL(FiringTracker, FiringTrackerPool)

public:
    FiringTracker(Thing *thing, const ModuleData *module_data);

    virtual ~FiringTracker() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual SleepyUpdatePhase Get_Update_Phase() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual UpdateSleepTime Update() override;

    UpdateSleepTime Calc_Time_To_Sleep();
    void Cool_Down();
    int Get_Num_Concecutive_Shots_At_Victim(const Object *victim);
    void Shot_Fired(const Weapon *weapon, ObjectID victim);
    void Speed_Up();

    ObjectID Get_Victim_ID() { return m_victimID; }

private:
    int m_consecutiveShots;
    ObjectID m_victimID;
    unsigned int m_fireFrame;
    unsigned int m_idleFrame;
    unsigned int m_fireSoundFrame;
    int m_audioEvent;
};
