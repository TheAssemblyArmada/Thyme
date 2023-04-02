/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object SMC Helper
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objectsmchelper.h"

ObjectSMCHelper::ObjectSMCHelper(Thing *thing, const ModuleData *module_data) : ObjectHelper(thing, module_data) {}

ObjectSMCHelper::~ObjectSMCHelper() {}

NameKeyType ObjectSMCHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("ObjectSMCHelper");
    return _key;
}

void ObjectSMCHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void ObjectSMCHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
}

void ObjectSMCHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

UpdateSleepTime ObjectSMCHelper::Update()
{
    Get_Object()->Clear_Special_Model_Condition_States();
    return UPDATE_SLEEP_TIME_MAX;
}
