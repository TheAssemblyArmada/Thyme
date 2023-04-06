/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Damage Module
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

class DamageModuleInterface
{
public:
    virtual void On_Damage(DamageInfo *info) = 0;
    virtual void On_Healing(DamageInfo *info) = 0;
    virtual void On_Body_Damage_Change_State(const DamageInfo *info, BodyDamageType old_type, BodyDamageType new_type) = 0;
};

class DamageModule : public BehaviorModule, public DamageModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(DamageModule)

public:
    virtual ~DamageModule() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual DamageModuleInterface *Get_Damage() override;
};
