/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Module Interface
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

class SpecialPowerTemplate;

// mac: 0096BC88
class SpecialPowerModuleInterface
{
public:
    virtual bool Is_Module_For_Power(const SpecialPowerTemplate *sptemplate) const = 0;
    virtual bool Is_Ready() const = 0;
    virtual float Get_Percent_Ready() const = 0;
    virtual unsigned int Get_Ready_Frame() const = 0;
    virtual int Get_Power_Name() const = 0; // to be completed
    virtual SpecialPowerTemplate *Get_SpecialPowerTemplate() const = 0;
    virtual ScienceType Get_Required_Science() const = 0;
    virtual void On_Special_Power_Creation() = 0;
    virtual void Set_Ready_Frame(unsigned int) = 0;
    virtual void Pause_Countdown(bool) = 0;
    virtual int Do_Special_Power(unsigned int) = 0; // to be completed
    virtual int Do_Special_Power_At_Object(Object *, unsigned int) = 0; // to be completed
    virtual int Do_Special_Power_At_Location(const Coord3D *, float, unsigned int) = 0; // to be completed
    virtual int Do_Special_Power_Using_Waypoints(const Waypoint *, unsigned int) = 0; // to be completed
    virtual int Mark_Special_Power_Triggered(const Coord3D *) = 0; // to be completed
    virtual int Start_Power_Recharge() = 0; // to be completed
    virtual int Get_Initiate_Sound() const = 0; // to be completed
    virtual int Is_Script_Only() const = 0; // to be completed
    virtual int Get_Reference_ThingTemplate() const = 0; // to be completed
};
