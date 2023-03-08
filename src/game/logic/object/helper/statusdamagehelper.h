/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Status Damage Helper
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

class StatusDamageHelperModuleData : public ModuleData
{
public:
    StatusDamageHelperModuleData() {}
    ~StatusDamageHelperModuleData() override {}
};

class StatusDamageHelper : public ObjectHelper
{
    IMPLEMENT_POOL(StatusDamageHelper)

public:
    StatusDamageHelper(Thing *thing, const ModuleData *module_data);

    virtual ~StatusDamageHelper() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual UpdateSleepTime Update() override;

    void Clear_Status_Condition();
    void Do_Status_Damage(ObjectStatusTypes status, float damage);

private:
    ObjectStatusTypes m_statusType;
    unsigned int m_frame;
};
