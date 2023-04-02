/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Subdual Damage Helper
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

class SubdualDamageHelperModuleData : public ModuleData
{
public:
    SubdualDamageHelperModuleData() {}
    ~SubdualDamageHelperModuleData() override {}
};

class SubdualDamageHelper : public ObjectHelper
{
    IMPLEMENT_POOL(SubdualDamageHelper)

public:
    SubdualDamageHelper(Thing *thing, const ModuleData *module_data);

    virtual ~SubdualDamageHelper() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual UpdateSleepTime Update() override;

    void Notify_Subdual_Damage(float damage);

private:
    unsigned int m_healRate;
};
