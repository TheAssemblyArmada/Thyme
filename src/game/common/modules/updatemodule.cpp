/**
 * @file
 *
 * @author xezon
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
#include "updatemodule.h"

#include "gamelogic.h"
#include "thing.h"
#include <captainslog.h>

UpdateModule::UpdateModule(Thing *thing, const ModuleData *module_data) :
    BehaviorModule(thing, module_data), m_updatePhase(0u), m_indexInLogic(-1)
{
}

UpdateModule::~UpdateModule()
{
    captainslog_dbgassert(m_indexInLogic == -1, "Destroying an UpdateModule still in the logic list");
}

SleepyUpdatePhase UpdateModule::Get_Update_Phase() const
{
    return SLEEPY_UPDATE_PHASE_2;
}

void UpdateModule::CRC_Snapshot(Xfer *xfer)
{
    BehaviorModule::CRC_Snapshot(xfer);
}

void UpdateModule::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1u;
    xfer->xferVersion(&version, 1u);
    BehaviorModule::Xfer_Snapshot(xfer);

    xfer->xferUnsignedInt(&m_updatePhase);

    if (xfer->Get_Mode() == XFER_LOAD) {

        m_updatePhase &= UPDATE_PHASE_FRAME_MASK;
        m_updatePhase |= Get_Update_Phase();

        if (m_updatePhase == 0) {
            Encode_Frame(g_theGameLogic->Get_Frame());
        }
        m_indexInLogic = -1;
    }
}

void UpdateModule::Load_Post_Process()
{
    BehaviorModule::Load_Post_Process();
}

UpdateModuleInterface *UpdateModule::Get_Update()
{
    return static_cast<UpdateModuleInterface *>(this);
}

DisabledBitFlags UpdateModule::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_NONE;
}

void UpdateModule::Set_Index_In_Logic(int index)
{
    m_indexInLogic = index;
}

int UpdateModule::Get_Index_In_Logic()
{
    return m_indexInLogic;
}

void UpdateModule::Encode_Frame(unsigned int frame)
{
    if (frame > UPDATE_SLEEP_TIME_MAX) {
        frame = UPDATE_SLEEP_TIME_MAX;
    }
    m_updatePhase = Get_Update_Phase();
    m_updatePhase |= (frame << SLEEPY_UPDATE_PHASE_2);
}

unsigned int UpdateModule::Decode_Frame() const
{
    return m_updatePhase >> SLEEPY_UPDATE_PHASE_2;
}

UpdateSleepTime UpdateModule::Get_Wake_Frame() const
{
    unsigned int cur_frame = g_theGameLogic->Get_Frame();
    unsigned int wake_frame = Decode_Frame();

    if (cur_frame < wake_frame) {
        return UpdateSleepTime(wake_frame - cur_frame);
    }

    return UPDATE_SLEEP_TIME_MIN;
}

void UpdateModule::Set_Wake_Frame(Object *object, UpdateSleepTime frame)
{
    unsigned int cur_frame = g_theGameLogic->Get_Frame();
    g_theGameLogic->Friend_Awaken_Update_Module(object, this, frame + cur_frame);
}

UpdateSleepTime UpdateModule::Frame_To_Sleep_Time(
    unsigned int frame1, unsigned int frame2, unsigned int frame3, unsigned int frame4)
{
    captainslog_dbgassert(
        frame1 != 0 && frame2 != 0 && frame3 != 0 && frame4 != 0, "Frame_To_Sleep_Time: Should not pass zero frames");

    if (frame1 > frame2) {
        frame1 = frame2;
    }

    if (frame1 > frame3) {
        frame1 = frame3;
    }

    if (frame1 > frame4) {
        frame1 = frame4;
    }

    unsigned int cur_frame = g_theGameLogic->Get_Frame();

    if (frame1 > cur_frame) {
        return static_cast<UpdateSleepTime>(frame1 - cur_frame);
    }

    if (frame1 == cur_frame) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    captainslog_dbgassert(0, "Frame_To_Sleep_Time: frame is in the past");
    return UPDATE_SLEEP_TIME_MIN;
}

ModuleData *UpdateModule::Friend_New_Module_Data(INI *ini)
{
    UpdateModuleData *data = new UpdateModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, UpdateModuleData::Build_Field_Parse);
    }

    return data;
}

int UpdateModule::Get_Interface_Mask()
{
    return MODULEINTERFACE_UPDATE;
}

bool UpdateModule::Compare_Update_Modules(UpdateModule *a, UpdateModule *b)
{
    captainslog_dbgassert(a != nullptr && b != nullptr, "UpdateModule must not be null");

    return b->Get_Raw_Update_Value() < a->Get_Raw_Update_Value();
}

unsigned int UpdateModule::Get_Raw_Update_Value() const
{
    return m_updatePhase;
}
