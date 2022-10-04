/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Update Interface
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

class CommandButton;
class Coord3D;
class Object;
class SpecialPowerTemplate;
class Waypoint;

enum ScienceType;

enum CommandOption
{
    COMMAND_OPTION_0 = 0,
};

class SpecialPowerUpdateInterface
{
public:
    virtual bool Does_Special_Power_Update_Pass_Science_Test() const = 0;
    virtual ScienceType Get_Extra_Required_Science() const = 0;
    virtual void Initiate_Intent_To_Do_Special_Power(
        const SpecialPowerTemplate *, const Object *, const Coord3D *, const Waypoint *, unsigned int) = 0;
    virtual bool Is_Special_Ability() const = 0;
    virtual bool Is_Special_Power() const = 0;
    virtual bool Is_Active() const = 0;
    virtual CommandOption Get_Command_Option() const = 0;
    virtual bool Does_Special_Power_Have_Overridable_Destination_Active() const = 0;
    virtual bool Does_Special_Power_Have_Overridable_Destination() const = 0;
    virtual void Set_Special_Power_Overridable_Destination(const Coord3D *) = 0;
    virtual bool Is_Power_Currently_In_Use(const CommandButton *) const = 0;
};
