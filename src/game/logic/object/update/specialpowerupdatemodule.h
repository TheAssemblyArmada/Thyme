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
#pragma once

#include "specialpowerupdateinterface.h"
#include "updatemodule.h"

class Thing;
class ModuleData;

class SpecialPowerUpdateModule : public UpdateModule, public SpecialPowerUpdateInterface
{
    IMPLEMENT_ABSTRACT_POOL(SpecialPowerUpdateModule)

protected:
    SpecialPowerUpdateModule(Thing *thing, const ModuleData *module_data);
    virtual ~SpecialPowerUpdateModule() override {}

public:
    // Snapshot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    //~Snapshot

    // SpecialPowerUpdateInterface
    virtual bool Does_Special_Power_Update_Pass_Science_Test() const override;
    virtual ScienceType Get_Extra_Required_Science() const override;
    //~SpecialPowerUpdateInterface
};
