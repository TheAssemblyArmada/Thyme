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
#pragma once
#include "always.h"
#include "objecthelper.h"

class ObjectWeaponStatusHelperModuleData : public ModuleData
{
public:
    ObjectWeaponStatusHelperModuleData() {}
    ~ObjectWeaponStatusHelperModuleData() override {}
};

class ObjectWeaponStatusHelper : public ObjectHelper
{
    IMPLEMENT_NAMED_POOL(ObjectWeaponStatusHelper, ObjectWeaponStatusHelperPool)

public:
    ObjectWeaponStatusHelper(Thing *thing, const ModuleData *module_data);

    ~ObjectWeaponStatusHelper() override;
    NameKeyType Get_Module_Name_Key() const override;
    SleepyUpdatePhase Get_Update_Phase() const override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    UpdateSleepTime Update() override;
};
