/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Open Contain
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
class OpenContain;
class Player;

enum ObjectEnterExitType
{
    ENTER_EXIT_UNK
};

enum EvacDisposition
{
    EVAC_DISPOSITION_UNK
};

class ContainModuleInterface
{
public:
    virtual OpenContain *As_Open_Contain() = 0;
    virtual void Contain_React_To_Transform_Change() = 0;
    virtual bool Is_Garrisonable() const = 0;
    virtual bool Is_Bustable() const = 0;
    virtual bool Is_Special_Zero_Slot_Container() const = 0;
    virtual bool Is_Heal_Contain() const = 0;
    virtual bool Is_Tunnel_Contain() const = 0;
    virtual bool Is_Rider_Change_Contain() const = 0;
    virtual bool Is_Immune_To_Clear_Building_Attacks() const = 0;
    virtual bool Is_Special_Overlord_Style_Container() const = 0;
    virtual bool Is_Any_Rider_Attacking() const = 0;

    virtual void Client_Visible_Contained_Flash_As_Selected() = 0;
    virtual void On_Object_Wants_To_Enter_Or_Exit(Object *obj, ObjectEnterExitType type) = 0;
    virtual bool Has_Objects_Wanting_To_Enter_Or_Exit() const = 0;
    virtual Player const *Get_Apparent_Controlling_Player(Player const *player) const = 0;
    virtual void Recalc_Apparent_Controlling_Player() = 0;
    virtual void On_Containing(Object *rider, bool b) = 0;
    virtual void On_Removing(Object *rider) = 0;
    virtual void On_Capture(Player *old_owner, Player *new_owner) = 0;
    virtual void On_Selling() = 0;

    virtual int Get_Contain_Max() const = 0;
    virtual ExitInterface *Get_Contain_Exit_Interface() = 0;
    virtual void Order_All_Passengers_To_Exit(CommandSourceType type, bool b) = 0;
    virtual void Order_All_Passengers_To_Idle(CommandSourceType type) = 0;
    virtual void Order_All_Passengers_To_Hack_Internet(CommandSourceType type) = 0;
    virtual void Mark_All_Passengers_Detected() = 0;
    virtual bool Is_Valid_Container_For(Object const *obj, bool check_capacity) const = 0;

    virtual void Add_To_Contain(Object *rider) = 0;
    virtual void Add_To_Contain_List(Object *rider) = 0;
    virtual void Remove_From_Contain(Object *rider, bool expose_stealth_units) = 0;
    virtual void Remove_All_Contained(bool expose_stealth_units) = 0;
    virtual void Kill_All_Contained() = 0;
    virtual void Harm_And_Force_Exit_All_Contained(DamageInfo *info) = 0;

    virtual bool Is_Enclosing_Container_For(Object const *rider) const = 0;
    virtual bool Is_Passenger_Allowed_To_Fire(ObjectID object) const = 0;
    virtual void Set_Passenger_Allowed_To_Fire(bool fire) = 0;
    virtual void Set_Override_Destination(Coord3D const *pos) = 0;
    virtual bool Is_Displayed_On_Control_Bar() const = 0;
    virtual int Get_Extra_Slots_In_Use() = 0;
    virtual bool Is_Kick_Out_On_Capture() = 0;

    virtual void Iterate_Contained(void (*func)(Object *, void *), void *user_data, bool reverse) = 0;
    virtual unsigned int Get_Contain_Count() const = 0;
    virtual std::list<Object *> const *Get_Contained_Items_List() const = 0;
    virtual Object const *Friend_Get_Rider() const = 0;
    virtual float Get_Contained_Items_Mass() const = 0;
    virtual unsigned int Get_Stealth_Units_Contained() const = 0;
    virtual bool Calc_Best_Garrison_Position(Coord3D *pos, Coord3D const *garrison) = 0;

    virtual bool Attempt_Best_Fire_Point_Position(Object *obj, Weapon *weapon, Coord3D const *pos) = 0;
    virtual bool Attempt_Best_Fire_Point_Position(Object *obj, Weapon *weapon, Object *obj2) = 0;
    virtual unsigned short Get_Player_Who_Entered() const = 0;
    virtual void Process_Damage_To_Contained(float damage) = 0;
    virtual Object const *Get_Closest_Rider(Coord3D const *pos) = 0;
    virtual void Enable_Load_Sounds(bool enable) = 0;

    virtual void Set_Evac_Disposition(EvacDisposition evac) = 0;
    virtual bool Is_Weapon_Bonus_Passed_To_Passengers() const = 0;
    virtual unsigned int Get_Weapon_Bonus_Passed_To_Passengers() const = 0;
    virtual bool Get_Container_Pips_To_Show(int &max, int &count);
};

enum ExitDoorType
{
    EXIT_DOOR_UNK,
};

class ExitInterface
{
public:
    virtual bool Is_Exit_Busy() const = 0;
    virtual ExitDoorType Reserve_Door_For_Exit(const ThingTemplate *tmplate, Object *obj) = 0;
    virtual void Exit_Object_Via_Door(Object *obj, ExitDoorType type) = 0;
    virtual void Exit_Object_Via_Budding(Object *obj, Object *obj2) = 0;
    virtual void Unreserve_Door_For_Exit(ExitDoorType type) = 0;
    virtual void Exit_Object_In_A_Hurry(Object *obj) {}
    virtual void Set_Rally_Point(const Coord3D *point) = 0;
    virtual const Coord3D *Get_Rally_Point() = 0;
    virtual bool Use_Spawn_Rally_Point() { return false; }

    virtual bool Get_Natural_Rally_Point(Coord3D &coord, bool b)
    {
        coord.x = 0.0f;
        coord.y = 0.0f;
        coord.z = 0.0f;
        return false;
    }

    virtual bool Get_Exit_Position(Coord3D &coord)
    {
        coord.x = 0.0f;
        coord.y = 0.0f;
        coord.z = 0.0f;
        return false;
    }
};
