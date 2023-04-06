/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Power Plant Upgrade
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
#include "upgrademodule.h"

class PowerPlantUpgrade : public UpgradeModule
{
    IMPLEMENT_POOL(PowerPlantUpgrade)

public:
    virtual ~PowerPlantUpgrade() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Delete() override;
    virtual void On_Capture(Player *player1, Player *player2) override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Is_Sub_Objects_Upgrade() override;
    virtual void Upgrade_Implementation() override;
};
