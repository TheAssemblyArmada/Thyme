/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "always.h"
#include "object.h"
#include "updatemodule.h"

enum CommandOption
{
    COMMAND_OPTION_UNK,
};

class SpecialPowerUpdateInterface
{
public:
    virtual bool Does_Special_Power_Update_Pass_Science_Test() const = 0;
    virtual ScienceType Get_Extra_Required_Science() const = 0;
    virtual void Initiate_Intent_To_Do_Special_Power(const SpecialPowerTemplate *special_power_template,
        const Object *target_obj,
        const Coord3D *target_pos,
        const Waypoint *waypoint,
        unsigned int options) = 0;
    virtual bool Is_Special_Ability() const = 0;
    virtual bool Is_Special_Power() const = 0;
    virtual bool Is_Active() const = 0;
    virtual CommandOption Get_Command_Option() const = 0;
    virtual bool Does_Special_Power_Have_Overridable_Destination_Active() const = 0;
    virtual bool Does_Special_Power_Have_Overridable_Destination() const = 0;
    virtual void Set_Special_Power_Overridable_Destination(const Coord3D *destination) = 0;
    virtual bool Is_Power_Currently_In_Use(const CommandButton *button) const = 0;
};

class SpecialPowerUpdateModule : public UpdateModule, public SpecialPowerUpdateInterface
{
    IMPLEMENT_ABSTRACT_POOL(SpecialPowerUpdateModule)

public:
    virtual ~SpecialPowerUpdateModule() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Does_Special_Power_Update_Pass_Science_Test() const override;
    virtual ScienceType Get_Extra_Required_Science() const override;
};
