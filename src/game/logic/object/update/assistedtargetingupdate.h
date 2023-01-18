/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Assisted Targeting Update
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
#include "updatemodule.h"

class AssistedTargetingUpdate : public UpdateModule
{
    IMPLEMENT_POOL(AssistedTargetingUpdate);

public:
    virtual ~AssistedTargetingUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual UpdateSleepTime Update() override;

    bool Is_Free_To_Assist() const;
    void Assist_Attack(const Object *source_obj, Object *target_obj);

private:
    ThingTemplate *m_laserFromAssisted;
    ThingTemplate *m_laserToTarget;
};
