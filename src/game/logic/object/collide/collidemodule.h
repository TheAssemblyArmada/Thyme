/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Collide Module
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

class CollideModule : public BehaviorModule, public CollideModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(CollideModule)

public:
    ~CollideModule() override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    CollideModuleInterface *Get_Collide() override;

    bool Would_Like_To_Collide_With(Object const *other) override;
    bool Is_Hijacked_Vehicle_Crate_Collide() override;
    bool Is_Sabotage_Building_Crate_Collide() override;
    bool Is_Car_Bomb_Crate_Collide() override;
    bool Is_Railroad() override;
    bool Is_Salvage_Crate_Collide() override;
};
