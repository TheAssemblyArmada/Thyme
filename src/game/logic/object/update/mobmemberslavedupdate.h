/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mob Member Slaved Update
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
#include "color.h"
#include "updatemodule.h"

class MobMemberSlavedUpdate : public UpdateModule, public SlavedUpdateInterface
{
    IMPLEMENT_POOL(MobMemberSlavedUpdate)

public:
    virtual ~MobMemberSlavedUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual SlavedUpdateInterface *Get_Slaved_Update_Interface() override;
    virtual UpdateSleepTime Update() override;
    virtual ObjectID Get_Slaver_ID() const override;
    virtual void On_Enslave(const Object *slaver) override;
    virtual void On_Slaver_Die(const DamageInfo *info) override;
    virtual void On_Slaver_Damage(const DamageInfo *info) override;
    virtual bool Is_Self_Tasking() const override;

private:
    ObjectID m_slaverObjectID;
    int m_updateTimer;
    int m_unk; // enum of some kind
    RGBColor m_color;
    ObjectID m_currentVictimID;
    float m_squirrelliness;
    bool m_isSelfTasking;
    int m_catchUpCrisisBailTimer;
};
