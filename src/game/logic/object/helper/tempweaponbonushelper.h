/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Temp Weapon Bonus Helper
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

class TempWeaponBonusHelperModuleData : public ModuleData
{
public:
    TempWeaponBonusHelperModuleData() {}
    ~TempWeaponBonusHelperModuleData() override {}
};

class TempWeaponBonusHelper : public ObjectHelper
{
    IMPLEMENT_POOL(TempWeaponBonusHelper)

public:
    TempWeaponBonusHelper(Thing *thing, const ModuleData *module_data);

    virtual ~TempWeaponBonusHelper() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual UpdateSleepTime Update() override;

    void Clear_Temp_Weapon_Bonus();
    void Do_Temp_Weapon_Bonus(WeaponBonusConditionType bonus, unsigned int frame);

private:
    WeaponBonusConditionType m_bonusType;
    unsigned int m_frame;
};
