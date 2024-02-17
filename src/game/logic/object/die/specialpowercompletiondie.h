/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Power Completion Die
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "diemodule.h"

class SpecialPowerCompletionDie : public DieModule
{
    IMPLEMENT_POOL(SpecialPowerCompletionDie)

public:
    virtual ~SpecialPowerCompletionDie() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void On_Die(DamageInfo *damage_info) override;

    void Notify_Script_Engine();
    void Set_Creator(ObjectID id);

private:
    ObjectID m_creatorID;
    bool m_creatorSet;
};
