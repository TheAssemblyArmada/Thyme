/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Update Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpowerupdatemodule.h"

#include "object.h"
#include "player.h"
#include "science.h"
#include "thing.h"

// wb: 00A7C7D0
SpecialPowerUpdateModule::SpecialPowerUpdateModule(Thing *thing, const ModuleData *module_data) :
    UpdateModule(thing, module_data)
{
}

// wb: 00A7C8EE
void SpecialPowerUpdateModule::CRC_Snapshot(Xfer *xfer)
{
    UpdateModule::CRC_Snapshot(xfer);
}

// wb: 00A7C907
void SpecialPowerUpdateModule::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    UpdateModule::Xfer_Snapshot(xfer);
}

// wb: 00A7C941
void SpecialPowerUpdateModule::Load_Post_Process()
{
    UpdateModule::Load_Post_Process();
}

// wb: 00A7C89D
bool SpecialPowerUpdateModule::Does_Special_Power_Update_Pass_Science_Test() const
{
    const ScienceType required_science = Get_Extra_Required_Science();
    bool pass = true;

    if (required_science != SCIENCE_INVALID) {
        Player *player = Get_Object()->Get_Controlling_Player();
        captainslog_assert(player != nullptr);

        if (!player->Has_Science(required_science)) {
            pass = false;
        }
    }

    return pass;
}

// wb: 0096E1A0
ScienceType SpecialPowerUpdateModule::Get_Extra_Required_Science() const
{
    return SCIENCE_INVALID;
}
