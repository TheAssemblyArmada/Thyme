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
#include "statusdamagehelper.h"
#include "gamelogic.h"

StatusDamageHelper::StatusDamageHelper(Thing *thing, const ModuleData *module_data) :
    ObjectHelper(thing, module_data), m_statusType(OBJECT_STATUS_NONE), m_frame(0)
{
}

StatusDamageHelper::~StatusDamageHelper() {}

NameKeyType StatusDamageHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("StatusDamageHelper");
    return _key;
}

void StatusDamageHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void StatusDamageHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
    xfer->xferUser(&m_statusType, sizeof(m_statusType));
    xfer->xferUnsignedInt(&m_frame);
}

void StatusDamageHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

BitFlags<DISABLED_TYPE_COUNT> StatusDamageHelper::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

UpdateSleepTime StatusDamageHelper::Update()
{
    captainslog_dbgassert(m_frame <= g_theGameLogic->Get_Frame(), "StatusDamageHelper woke up too soon.");
    Clear_Status_Condition();
    return UPDATE_SLEEP_TIME_MAX;
}

void StatusDamageHelper::Clear_Status_Condition()
{
    if (m_statusType != OBJECT_STATUS_NONE) {
        BitFlags<OBJECT_STATUS_COUNT> flags(BitFlags<OBJECT_STATUS_COUNT>::kInit, m_statusType);
        Get_Object()->Clear_Status(flags);
        m_statusType = OBJECT_STATUS_NONE;
        m_frame = 0;
    }
}

void StatusDamageHelper::Do_Status_Damage(ObjectStatusTypes status, float damage)
{
    unsigned int frame = GameMath::Fast_To_Int_Floor(damage);

    if (m_statusType != status) {
        Clear_Status_Condition();
    }

    BitFlags<OBJECT_STATUS_COUNT> flags(BitFlags<OBJECT_STATUS_COUNT>::kInit, status);
    Get_Object()->Set_Status(flags, true);
    m_statusType = status;
    m_frame = g_theGameLogic->Get_Frame() + frame;
    Set_Wake_Frame(Get_Object(), static_cast<UpdateSleepTime>(frame));
}
