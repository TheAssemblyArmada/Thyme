/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Die Module
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

class DieModuleInterface
{
public:
    virtual void On_Die(DamageInfo *damage_info) = 0;
};

class DieModule : public BehaviorModule, public DieModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(DieModule)

public:
    virtual ~DieModule() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual DieModuleInterface *Get_Die() override;
};
