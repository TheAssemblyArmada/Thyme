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
    ~OverchargeBehavior() override;
    NameKeyType Get_Module_Name_Key() const override;
    void On_Delete() override;
    void On_Capture(Player *player1, Player *player2) override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    DamageModuleInterface *Get_Damage() override;
    OverchargeBehaviorInterface *Get_Overcharge_Behavior_Interface() override;

    UpdateSleepTime Update() override;

    void On_Damage(DamageInfo *info) override;
    void On_Healing(DamageInfo *info) override;
    void On_Body_Damage_Change_State(const DamageInfo *info, BodyDamageType old_type, BodyDamageType new_type) override;

    void Toggle() override;
    void Enable(bool enable) override;
    bool Is_Overcharge_Active() override;

private:
    bool m_overchargeActive;
};
