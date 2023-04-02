/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Repulsor Helper
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objectrepulsorhelper.h"

ObjectRepulsorHelper::ObjectRepulsorHelper(Thing *thing, const ModuleData *module_data) : ObjectHelper(thing, module_data) {}

ObjectRepulsorHelper::~ObjectRepulsorHelper() {}

NameKeyType ObjectRepulsorHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("ObjectRepulsorHelper");
    return _key;
}

void ObjectRepulsorHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void ObjectRepulsorHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
}

void ObjectRepulsorHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

UpdateSleepTime ObjectRepulsorHelper::Update()
{
    BitFlags<OBJECT_STATUS_COUNT> flags(BitFlags<OBJECT_STATUS_COUNT>::kInit, OBJECT_STATUS_REPULSOR);
    Get_Object()->Clear_Status(flags);
    return UPDATE_SLEEP_TIME_MAX;
}
