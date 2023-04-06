/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Upgrade
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
#include "behaviormodule.h"

class UpgradeMuxData
{
private:
    std::vector<Utf8String> m_unk;
    std::vector<Utf8String> m_activationUpgradeNames;
    std::vector<Utf8String> m_conflictingUpgradeNames;
    std::vector<Utf8String> m_removedUpgradeNames;
    FXList *m_fxListUpgrade;
    BitFlags<128> m_activationMask;
    BitFlags<128> m_conflictingMask;
    bool m_requiresAllTriggers;
};

class UpgradeModuleInterface
{
public:
    virtual bool Is_Already_Upgraded() const = 0;
    virtual bool Attempt_Upgrade(BitFlags<128ul> mask) = 0;
    virtual bool Would_Upgrade(BitFlags<128ul> mask) const = 0;
    virtual bool Reset_Upgrade(BitFlags<128ul> mask) = 0;
    virtual bool Is_Sub_Objects_Upgrade() = 0;
    virtual void Force_Refresh_Upgrade() = 0;
    virtual bool Test_Upgrade_Conditions(BitFlags<128ul> mask) const = 0;
};

class UpgradeMux : public UpgradeModuleInterface
{
public:
    virtual bool Is_Already_Upgraded() const override;
    virtual bool Attempt_Upgrade(BitFlags<128ul> mask) override;
    virtual bool Would_Upgrade(BitFlags<128ul> mask) const override;
    virtual bool Reset_Upgrade(BitFlags<128ul> mask) override;
    virtual void Force_Refresh_Upgrade() override;
    virtual bool Test_Upgrade_Conditions(BitFlags<128ul> mask) const override;

    virtual void Upgrade_Implementation() = 0;
    virtual void Get_Upgrade_Activation_Masks(BitFlags<128ul> &activation_mask, BitFlags<128ul> &conflicting_mask) const = 0;
    virtual void Perform_Upgrade_FX() = 0;
    virtual bool Requires_All_Activation_Upgrades() const = 0;
    virtual void Process_Upgrade_Removal() = 0;
    virtual void Upgrade_Mux_CRC(Xfer *xfer);
    virtual void Upgrade_Mux_Xfer(Xfer *xfer);
    virtual void Upgrade_Mux_Load_Post_Process();

private:
    bool m_upgradeExecuted;
};

class UpgradeModuleData : public BehaviorModuleData
{
public:
    virtual ~UpgradeModuleData();

private:
    UpgradeMuxData m_upgradeMuxData;
};

class UpgradeModule : public BehaviorModule, public UpgradeMux
{
    IMPLEMENT_ABSTRACT_POOL(UpgradeModule);

public:
    virtual ~UpgradeModule() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual UpgradeModuleInterface *Get_Upgrade() override;

    virtual void Get_Upgrade_Activation_Masks(
        BitFlags<128ul> &activation_mask, BitFlags<128ul> &conflicting_mask) const override;
    virtual void Perform_Upgrade_FX() override;
    virtual bool Requires_All_Activation_Upgrades() const override;
    virtual void Process_Upgrade_Removal() override;
};
