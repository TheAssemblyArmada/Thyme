/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Helper
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objecthelper.h"
#include "gamelogic.h"

ObjectHelper::ObjectHelper(Thing *thing, const ModuleData *module_data) : UpdateModule(thing, module_data)
{
    Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MAX);
}

void ObjectHelper::CRC_Snapshot(Xfer *xfer)
{
    UpdateModule::CRC_Snapshot(xfer);
}

void ObjectHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    UpdateModule::Xfer_Snapshot(xfer);
}

void ObjectHelper::Load_Post_Process()
{
    UpdateModule::Load_Post_Process();
}

void ObjectHelper::Sleep_Until(unsigned int frame)
{
    if (!Get_Object()->Get_Status_Bits().Test(OBJECT_STATUS_DESTROYED)) {
        if (frame == 0 || frame == UPDATE_SLEEP_TIME_MAX) {
            Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MAX);
        } else {
            Set_Wake_Frame(Get_Object(), static_cast<UpdateSleepTime>(frame - g_theGameLogic->Get_Frame()));
        }
    }
}
