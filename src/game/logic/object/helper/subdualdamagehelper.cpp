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
#include "subdualdamagehelper.h"
#include "bodymodule.h"
#include "damage.h"

SubdualDamageHelper::SubdualDamageHelper(Thing *thing, const ModuleData *module_data) :
    ObjectHelper(thing, module_data), m_healRate(0)
{
}

SubdualDamageHelper::~SubdualDamageHelper() {}

NameKeyType SubdualDamageHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("SubdualDamageHelper");
    return _key;
}

void SubdualDamageHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void SubdualDamageHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
    xfer->xferUnsignedInt(&m_healRate);
}

void SubdualDamageHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

BitFlags<DISABLED_TYPE_COUNT> SubdualDamageHelper::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

UpdateSleepTime SubdualDamageHelper::Update()
{
    BodyModuleInterface *body = Get_Object()->Get_Body_Module();

    if (--m_healRate) {
        return UPDATE_SLEEP_TIME_MIN;
    } else {
        m_healRate = body->Get_Subdual_Damage_Heal_Rate();
        DamageInfo info;
        info.m_in.m_damageType = DAMAGE_SUBDUAL_UNRESISTABLE;
        info.m_in.m_amount = -body->Get_Subdual_Damage_Heal_Amount();
        body->Attempt_Damage(&info);

        if (body->Has_Any_Subdual_Damage()) {
            return UPDATE_SLEEP_TIME_MIN;
        } else {
            return UPDATE_SLEEP_TIME_MAX;
        }
    }
}

void SubdualDamageHelper::Notify_Subdual_Damage(float damage)
{
    if (damage > 0.0f) {
        BodyModuleInterface *body = Get_Object()->Get_Body_Module();
        m_healRate = body->Get_Subdual_Damage_Heal_Rate();
        Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MIN);
    }
}
