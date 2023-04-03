/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Overcharge Behavior
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
#include "damagemodule.h"
#include "updatemodule.h"

class OverchargeBehaviorInterface
{
public:
    virtual void Toggle() = 0;
    virtual void Enable(bool enable) = 0;
    virtual bool Is_Overcharge_Active() = 0;
};

class OverchargeBehavior : public UpdateModule, public DamageModuleInterface, public OverchargeBehaviorInterface
{
    IMPLEMENT_POOL(OverchargeBehavior);

public:
    virtual ~OverchargeBehavior() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Delete() override;
    virtual void On_Capture(Player *player1, Player *player2) override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual DamageModuleInterface *Get_Damage() override;
    virtual OverchargeBehaviorInterface *Get_Overcharge_Behavior_Interface() override;

    virtual UpdateSleepTime Update() override;

    virtual void On_Damage(DamageInfo *info) override;
    virtual void On_Healing(DamageInfo *info) override;
    virtual void On_Body_Damage_Change_State(
        const DamageInfo *info, BodyDamageType old_type, BodyDamageType new_type) override;

    virtual void Toggle() override;
    virtual void Enable(bool enable) override;
    virtual bool Is_Overcharge_Active() override;

private:
    bool m_overchargeActive;
};
