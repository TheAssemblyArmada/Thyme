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
#include "disabledtypes.h"
#include "moduleinfo.h"

enum SleepyUpdatePhase : uint32_t
{
    SLEEPY_UPDATE_PHASE_1 = 1,
    SLEEPY_UPDATE_PHASE_2 = 2,
    SLEEPY_UPDATE_PHASE_3 = 3,
};

enum UpdateSleepTime : uint32_t
{
    // this is a frame number
    UPDATE_SLEEP_TIME_MIN = 1u,
    UPDATE_SLEEP_TIME_MAX = (~0u >> SLEEPY_UPDATE_PHASE_2),
};

class UpdateModuleData : public BehaviorModuleData
{
public:
    UpdateModuleData() {}
    virtual ~UpdateModuleData() override {}

    static void Build_Field_Parse(MultiIniFieldParse &p) {}
};

class UpdateModuleInterface
{
public:
    virtual UpdateSleepTime Update() = 0;
    virtual DisabledBitFlags Get_Disabled_Types_To_Process() const = 0;
};

class UpdateModule : public BehaviorModule, public UpdateModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(UpdateModule)

    enum : uint32_t
    {
        UPDATE_PHASE_FRAME_MASK = (~(1u << SLEEPY_UPDATE_PHASE_2) + 1u)
    };

protected:
    UpdateModule(Thing *thing, const ModuleData *module_data);
    virtual ~UpdateModule() override;

public:
    virtual SleepyUpdatePhase Get_Update_Phase() const;

    // Snapshot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    //~Snapshot

    // BehaviorModule
    virtual UpdateModuleInterface *Get_Update() override;
    //~BehaviorModule

    // UpdateModuleInterface
    virtual DisabledBitFlags Get_Disabled_Types_To_Process() const override;
    //~UpdateModuleInterface

    // Indexing is currently used by GameLogic class.
    void Set_Index_In_Logic(int index);
    int Get_Index_In_Logic();

    void Encode_Frame(unsigned int frame);
    unsigned int Decode_Frame() const;

    // #TODO Uses enum here for some reason. Could probably just be primitive type.
    UpdateSleepTime Get_Wake_Frame() const;
    void Set_Wake_Frame(Object *object, UpdateSleepTime frame);

    UpdateSleepTime Frame_To_Sleep_Time(unsigned int frame1, unsigned int frame2, unsigned int frame3, unsigned int frame4);

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static int Get_Interface_Mask();
    static bool Compare_Update_Modules(UpdateModule *a, UpdateModule *b);

private:
    unsigned int Get_Raw_Update_Value() const;

private:
    unsigned int m_updatePhase;
    int m_indexInLogic;
};
