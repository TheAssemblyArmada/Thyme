/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Action Manager
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
#include "gametype.h"
#include "object.h"
#include "subsysteminterface.h"

class SpecialPowerTemplate;
class Coord3D;
class Player;

enum CanEnterType
{
    CAN_ENTER_0,
    CAN_ENTER_1,
    CAN_ENTER_2,
};

class ActionManager : public SubsystemInterface
{
public:
    ActionManager() {}
    virtual ~ActionManager() override {}
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    bool Can_Enter_Object(const Object *obj, const Object *object_to_enter, CommandSourceType source, CanEnterType type);
    bool Can_Do_Special_Power(
        const Object *obj, const SpecialPowerTemplate *sp_template, CommandSourceType source, unsigned int i, bool b);
    bool Can_Do_Special_Power_At_Location(const Object *obj,
        const Coord3D *loc,
        CommandSourceType source,
        const SpecialPowerTemplate *sp_template,
        const Object *object_in_way,
        unsigned int i,
        bool b);
    bool Can_Do_Special_Power_At_Object(const Object *obj,
        const Object *target,
        CommandSourceType source,
        const SpecialPowerTemplate *sp_template,
        unsigned int i,
        bool b);
    bool Can_Get_Repaired_At(const Object *obj, const Object *repair_dest, CommandSourceType source);
    bool Can_Transfer_Supplies_At(const Object *obj, const Object *transfer_dest);
    bool Can_Get_Healed_At(const Object *obj, const Object *heal_dest, CommandSourceType source);
    bool Can_Repair_Object(const Object *obj, const Object *object_to_repair, CommandSourceType source);
    bool Can_Resume_Construction_Of(const Object *obj, const Object *object_being_constructed, CommandSourceType source);
    bool Can_Convert_Object_To_Car_Bomb(const Object *obj, const Object *object_to_convert, CommandSourceType source);
    bool Can_Disable_Vehicle_Via_Hacking(
        const Object *obj, const Object *object_to_hack, CommandSourceType source, bool check_for_power);
    bool Can_Steal_Cash_Via_Hacking(const Object *obj, const Object *object_to_hack, CommandSourceType source);
    bool Can_Disable_Building_Via_Hacking(const Object *obj, const Object *object_to_hack, CommandSourceType source);
    bool Can_Fire_Weapon(const Object *obj, WeaponSlotType wslot, CommandSourceType source);
    bool Can_Player_Garrison(const Player *player, const Object *object_to_garrison, CommandSourceType source);
    bool Can_Override_Special_Power_Destination(
        const Object *obj, const Coord3D *destination, SpecialPowerType power, CommandSourceType source);
    bool Can_Make_Object_Defector(const Object *obj, const Object *object_to_defect, CommandSourceType source);
    bool Can_Sabotage_Building(const Object *obj, const Object *object_to_sabotage, CommandSourceType source);
    bool Can_Hijack_Vehicle(const Object *obj, const Object *object_to_hijack, CommandSourceType source);
    bool Can_Fire_Weapon_At_Location(const Object *obj,
        const Coord3D *location,
        CommandSourceType source,
        WeaponSlotType wslot,
        const Object *object_to_interact_with);
    bool Can_Fire_Weapon_At_Object(
        const Object *obj, const Object *object_to_interact_with, CommandSourceType source, WeaponSlotType wslot);
    bool Can_Capture_Building(const Object *obj, const Object *object_to_capture, CommandSourceType source);
    CanAttackResult Get_Can_Attack_Object(
        const Object *obj, const Object *object_to_attack, CommandSourceType source, AbleToAttackType attack);
    bool Can_Dock_At(const Object *obj, const Object *dock_dest, CommandSourceType source);
    bool Can_Snipe_Vehicle(const Object *obj, const Object *vehicle_to_snipe, CommandSourceType source);
};

#ifdef GAME_DLL
extern ActionManager *&g_theActionManager;
#else
extern ActionManager *g_theActionManager;
#endif
