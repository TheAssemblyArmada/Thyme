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
#include "tempweaponbonushelper.h"
#include "drawable.h"
#include "gamelogic.h"

TempWeaponBonusHelper::TempWeaponBonusHelper(Thing *thing, const ModuleData *module_data) :
    ObjectHelper(thing, module_data), m_bonusType(WEAPONBONUSCONDITION_INVALID), m_frame(0)
{
}

TempWeaponBonusHelper::~TempWeaponBonusHelper() {}

NameKeyType TempWeaponBonusHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("TempWeaponBonusHelper");
    return _key;
}

void TempWeaponBonusHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void TempWeaponBonusHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
    xfer->xferUser(&m_bonusType, sizeof(m_bonusType));
    xfer->xferUnsignedInt(&m_frame);
}

void TempWeaponBonusHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

BitFlags<DISABLED_TYPE_COUNT> TempWeaponBonusHelper::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

UpdateSleepTime TempWeaponBonusHelper::Update()
{
    captainslog_dbgassert(m_frame <= g_theGameLogic->Get_Frame(), "TempWeaponBonusHelper woke up too soon.");
    Clear_Temp_Weapon_Bonus();
    return UPDATE_SLEEP_TIME_MAX;
}

void TempWeaponBonusHelper::Clear_Temp_Weapon_Bonus()
{
    if (m_bonusType != WEAPONBONUSCONDITION_INVALID) {
        Get_Object()->Clear_Weapon_Bonus_Condition(m_bonusType);
        m_bonusType = WEAPONBONUSCONDITION_INVALID;
        m_frame = 0;

        if (Get_Object()->Get_Drawable()) {
            Get_Object()->Get_Drawable()->Clear_Draw_Bit(Drawable::DRAW_BIT_WEAPON_BONUS);
        }
    }
}

void TempWeaponBonusHelper::Do_Temp_Weapon_Bonus(WeaponBonusConditionType bonus, unsigned int frame)
{
    if (m_bonusType != bonus) {
        Clear_Temp_Weapon_Bonus();
    }

    Get_Object()->Set_Weapon_Bonus_Condition(bonus);
    m_bonusType = bonus;
    m_frame = frame + g_theGameLogic->Get_Frame();

    if (Get_Object()->Get_Drawable()) {
        Get_Object()->Get_Drawable()->Set_Draw_Bit(Drawable::DRAW_BIT_WEAPON_BONUS);
    }

    Set_Wake_Frame(Get_Object(), static_cast<UpdateSleepTime>(frame));
}
