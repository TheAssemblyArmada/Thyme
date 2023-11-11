/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief OCL Update
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

class OCLUpdate : public UpdateModule
{
    IMPLEMENT_POOL(OCLUpdate)

public:
    virtual ~OCLUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual UpdateSleepTime Update() override;
    virtual DisabledBitFlags Get_Disabled_Types_To_Process() const override;

    float Get_Countdown_Percent() const;
    unsigned int Get_Remaining_Frames() const;

private:
    unsigned int m_creationTimerEndFrame;
    unsigned int m_creationTimerStartFrame;
    bool m_isAIControlled;
    int m_color;
};
