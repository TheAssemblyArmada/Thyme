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
    ~AutoHealBehavior() override;
    NameKeyType Get_Module_Name_Key() const override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    DamageModuleInterface *Get_Damage() override;
    UpgradeModuleInterface *Get_Upgrade() override;

    UpdateSleepTime Update() override;
    DisabledBitFlags Get_Disabled_Types_To_Process() const override;

    bool Is_Sub_Objects_Upgrade() override;
    void Upgrade_Implementation() override;
    void Get_Upgrade_Activation_Masks(BitFlags<128ul> &activation_mask, BitFlags<128ul> &conflicting_mask) const override;
    void Perform_Upgrade_FX() override;
    bool Requires_All_Activation_Upgrades() const override;
    void Process_Upgrade_Removal() override;

    void On_Damage(DamageInfo *info) override;
    void On_Healing(DamageInfo *info) override;
    void On_Body_Damage_Change_State(const DamageInfo *info, BodyDamageType old_type, BodyDamageType new_type) override;

    void Undo_Upgrade();

private:
    ParticleSystemID m_particleSystemID;
    unsigned int m_healingFrame;
    bool m_healingStopped;
};
