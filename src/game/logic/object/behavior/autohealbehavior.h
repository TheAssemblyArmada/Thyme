/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Auto Heal Behavior
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
#include "particlesysmanager.h"
#include "updatemodule.h"
#include "upgrademodule.h"

class AutoHealBehavior : public UpdateModule, public UpgradeMux, public DamageModuleInterface
{
    IMPLEMENT_POOL(AutoHealBehavior)

public:
    virtual ~AutoHealBehavior() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual DamageModuleInterface *Get_Damage() override;
    virtual UpgradeModuleInterface *Get_Upgrade() override;

    virtual UpdateSleepTime Update() override;
    virtual DisabledBitFlags Get_Disabled_Types_To_Process() const override;

    virtual bool Is_Sub_Objects_Upgrade() override;
    virtual void Upgrade_Implementation() override;
    virtual void Get_Upgrade_Activation_Masks(
        BitFlags<128ul> &activation_mask, BitFlags<128ul> &conflicting_mask) const override;
    virtual void Perform_Upgrade_FX() override;
    virtual bool Requires_All_Activation_Upgrades() const override;
    virtual void Process_Upgrade_Removal() override;

    virtual void On_Damage(DamageInfo *info) override;
    virtual void On_Healing(DamageInfo *info) override;
    virtual void On_Body_Damage_Change_State(
        const DamageInfo *info, BodyDamageType old_type, BodyDamageType new_type) override;

    void Undo_Upgrade();

private:
    ParticleSystemID m_particleSystemID;
    unsigned int m_healingFrame;
    bool m_healingStopped;
};
