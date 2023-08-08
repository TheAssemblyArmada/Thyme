/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Ability Update
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
#include "specialpowerupdatemodule.h"

class SpecialAbilityUpdate : public SpecialPowerUpdateModule
{
    IMPLEMENT_POOL(SpecialAbilityUpdate)

public:
    ~SpecialAbilityUpdate() override;
    NameKeyType Get_Module_Name_Key() const override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    SpecialPowerUpdateInterface *Get_Special_Power_Update_Interface() override;

    UpdateSleepTime Update() override;

    void Initiate_Intent_To_Do_Special_Power(const SpecialPowerTemplate *special_power_template,
        const Object *target_obj,
        const Coord3D *target_pos,
        const Waypoint *waypoint,
        unsigned int options) override;
    bool Is_Special_Ability() const override;
    bool Is_Special_Power() const override;
    bool Is_Active() const override;
    CommandOption Get_Command_Option() const override;
    bool Does_Special_Power_Have_Overridable_Destination_Active() const override;
    bool Does_Special_Power_Have_Overridable_Destination() const override;
    void Set_Special_Power_Overridable_Destination(const Coord3D *destination) override;
    bool Is_Power_Currently_In_Use(const CommandButton *button) const override;

    SpecialPowerType Get_Special_Power_Type() const;
};
