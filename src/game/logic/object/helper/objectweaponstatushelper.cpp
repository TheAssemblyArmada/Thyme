/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Weapon Status Helper
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objectweaponstatushelper.h"

ObjectWeaponStatusHelper::ObjectWeaponStatusHelper(Thing *thing, const ModuleData *module_data) :
    ObjectHelper(thing, module_data)
{
    captainslog_dbgassert(
        Get_Object()->Get_Template()->Can_Possibly_Have_Any_Weapon(), "should not be instantiated if we have no weapon");
    Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MIN);
}

ObjectWeaponStatusHelper::~ObjectWeaponStatusHelper() {}

NameKeyType ObjectWeaponStatusHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("ObjectWeaponStatusHelper");
    return _key;
}

SleepyUpdatePhase ObjectWeaponStatusHelper::Get_Update_Phase() const
{
    return SLEEPY_UPDATE_PHASE_3;
}

void ObjectWeaponStatusHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void ObjectWeaponStatusHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
}

void ObjectWeaponStatusHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

UpdateSleepTime ObjectWeaponStatusHelper::Update()
{
    Get_Object()->Adjust_Model_Condition_For_Weapon_Status();
    return UPDATE_SLEEP_TIME_MIN;
}
