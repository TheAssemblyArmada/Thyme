/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "behaviormodule.h"
#include "bitflags.h"

enum SleepyUpdatePhase
{
    UPDATE_UNK,
};

enum UpdateSleepTime
{
    SLEEPTIME_UNK,
};

class UpdateModuleData : public BehaviorModuleData
{
public:
    UpdateModuleData() {}
    virtual ~UpdateModuleData() {}

    static void Build_Field_Parse(MultiIniFieldParse &p) {}
};

class UpdateModuleInterface
{
public:
    virtual UpdateSleepTime Update() = 0;
    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const = 0;
};

class UpdateModule : public BehaviorModule, public UpdateModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(UpdateModule)

protected:
    virtual ~UpdateModule() override;

public:
    UpdateModule(Thing *thing, ModuleData *module_data);

    virtual SleepyUpdatePhase Get_Update_Phase() const;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual UpdateModuleInterface *Get_Update() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;

    void Set_Index_In_Logic(int index) { m_indexInLogic = index; }
    int Get_Index_In_Logic() { return m_indexInLogic; }

    unsigned int Get_Sleep_Time() { return m_update >> 2; }
    void Set_Sleep_Time(unsigned int sleep);

    unsigned int Get_Update_Value() { return m_update; }

    UpdateSleepTime Get_Wake_Frame() const;
    void Set_Wake_Frame(Object *obj, UpdateSleepTime sleep);
    UpdateSleepTime Frame_To_Sleep_Time(unsigned int frame1, unsigned int frame2, unsigned int frame3, unsigned int frame4);

    static ModuleData *New_Module_Data(INI *ini);

private:
    unsigned int m_update;
    int m_indexInLogic;
};

inline bool Compare_Update_Modules(UpdateModule *a, UpdateModule *b)
{
    captainslog_dbgassert(a && b, "these may no longer be null");
    return b->Get_Update_Value() < a->Get_Update_Value();
}
