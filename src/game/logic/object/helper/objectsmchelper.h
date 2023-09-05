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
#pragma once
#include "always.h"
#include "objecthelper.h"

class ObjectSMCHelperModuleData : public ModuleData
{
public:
    ObjectSMCHelperModuleData() {}
    ~ObjectSMCHelperModuleData() override {}
};

class ObjectSMCHelper : public ObjectHelper
{
    IMPLEMENT_NAMED_POOL(ObjectSMCHelper, ObjectSMCHelperPool)

public:
    ObjectSMCHelper(Thing *thing, const ModuleData *module_data);

    ~ObjectSMCHelper() override;
    NameKeyType Get_Module_Name_Key() const override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    UpdateSleepTime Update() override;
};
