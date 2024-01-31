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
#include "actionmanager.h"
#include "behaviormodule.h"
#include "object.h"
#include "player.h"
#include "specialpower.h"
#include "terrainlogic.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ActionManager *g_theActionManager;
#endif

bool ActionManager::Can_Enter_Object(
    const Object *obj, const Object *object_to_enter, CommandSourceType source, CanEnterType type)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType, CanEnterType>(
        PICK_ADDRESS(0x00496500, 0x008E06EC), this, obj, object_to_enter, source, type);
#else
    return false;
#endif
}

bool ActionManager::Can_Do_Special_Power(
    const Object *obj, const SpecialPowerTemplate *sp_template, CommandSourceType source, unsigned int i, bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const SpecialPowerTemplate *,
        CommandSourceType,
        unsigned int,
        bool>(PICK_ADDRESS(0x00497A70, 0x008E202D), this, obj, sp_template, source, i, b);
#else
    return false;
#endif
}

bool ActionManager::Can_Do_Special_Power_At_Location(const Object *obj,
    const Coord3D *loc,
    CommandSourceType source,
    const SpecialPowerTemplate *sp_template,
    const Object *object_in_way,
    unsigned int i,
    bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const Coord3D *,
        CommandSourceType,
        const SpecialPowerTemplate *,
        const Object *,
        unsigned int,
        bool>(PICK_ADDRESS(0x004972F0, 0x008E1907), this, obj, loc, source, sp_template, object_in_way, i, b);
#else
    return false;
#endif
}

bool ActionManager::Can_Do_Special_Power_At_Object(const Object *obj,
    const Object *target,
    CommandSourceType source,
    const SpecialPowerTemplate *sp_template,
    unsigned int i,
    bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const Object *,
        CommandSourceType,
        const SpecialPowerTemplate *,
        unsigned int,
        bool>(PICK_ADDRESS(0x00497530, 0x008E1AEB), this, obj, target, source, sp_template, i, b);
#else
    return false;
#endif
}

bool ActionManager::Can_Get_Repaired_At(const Object *obj, const Object *repair_dest, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00495E20, 0x008DFDAB), this, obj, repair_dest, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Transfer_Supplies_At(const Object *obj, const Object *transfer_dest)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *>(
        PICK_ADDRESS(0x00495F50, 0x008DFF92), this, obj, transfer_dest);
#else
    return false;
#endif
}

bool ActionManager::Can_Get_Healed_At(const Object *obj, const Object *heal_dest, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x004961A0, 0x008E028B), this, obj, heal_dest, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Repair_Object(const Object *obj, const Object *object_to_repair, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x004962B0, 0x008E03C7), this, obj, object_to_repair, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Resume_Construction_Of(
    const Object *obj, const Object *object_being_constructed, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x004963F0, 0x008E053B), this, obj, object_being_constructed, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Convert_Object_To_Car_Bomb(
    const Object *obj, const Object *object_to_convert, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496990, 0x008E0D5B), this, obj, object_to_convert, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Disable_Vehicle_Via_Hacking(
    const Object *obj, const Object *object_to_hack, CommandSourceType source, bool check_for_power)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType, bool>(
        PICK_ADDRESS(0x00496E10, 0x008E1335), this, obj, object_to_hack, source, check_for_power);
#else
    return false;
#endif
}

bool ActionManager::Can_Steal_Cash_Via_Hacking(const Object *obj, const Object *object_to_hack, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496F60, 0x008E14AF), this, obj, object_to_hack, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Disable_Building_Via_Hacking(
    const Object *obj, const Object *object_to_hack, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x004970D0, 0x008E1660), this, obj, object_to_hack, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Fire_Weapon(const Object *obj, WeaponSlotType wslot, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, WeaponSlotType, CommandSourceType>(
        PICK_ADDRESS(0x00497C50, 0x008E2257), this, obj, wslot, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Player_Garrison(const Player *player, const Object *object_to_garrison, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Player *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00497C80, 0x008E23CA), this, player, object_to_garrison, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Override_Special_Power_Destination(
    const Object *obj, const Coord3D *destination, SpecialPowerType power, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Coord3D *, SpecialPowerType, CommandSourceType>(
        PICK_ADDRESS(0x00497D10, 0x008E2486), this, obj, destination, power, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Make_Object_Defector(const Object *obj, const Object *object_to_defect, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496BE0, 0x008E1042), this, obj, object_to_defect, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Sabotage_Building(const Object *obj, const Object *object_to_sabotage, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496B20, 0x008E0F5B), this, obj, object_to_sabotage, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Hijack_Vehicle(const Object *obj, const Object *object_to_hijack, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496A40, 0x008E0E26), this, obj, object_to_hijack, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Fire_Weapon_At_Location(const Object *obj,
    const Coord3D *location,
    CommandSourceType source,
    WeaponSlotType wslot,
    const Object *object_to_interact_with)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const Coord3D *,
        CommandSourceType,
        WeaponSlotType,
        const Object *>(PICK_ADDRESS(0x00497B60, 0x008E2127), this, obj, location, source, wslot, object_to_interact_with);
#else
    return false;
#endif
}

bool ActionManager::Can_Fire_Weapon_At_Object(
    const Object *obj, const Object *object_to_interact_with, CommandSourceType source, WeaponSlotType wslot)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType, WeaponSlotType>(
        PICK_ADDRESS(0x00497B90, 0x008E2161), this, obj, object_to_interact_with, source, wslot);
#else
    return false;
#endif
}

bool ActionManager::Can_Capture_Building(const Object *obj, const Object *object_to_capture, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00496C50, 0x008E10B2), this, obj, object_to_capture, source);
#else
    return false;
#endif
}

CanAttackResult ActionManager::Get_Can_Attack_Object(
    const Object *obj, const Object *object_to_attack, CommandSourceType source, AbleToAttackType attack)
{
#ifdef GAME_DLL
    return Call_Method<CanAttackResult, ActionManager, const Object *, const Object *, CommandSourceType, AbleToAttackType>(
        PICK_ADDRESS(0x00496850, 0x008E0B72), this, obj, object_to_attack, source, attack);
#else
    return ATTACK_RESULT_CANNOT_ATTACK;
#endif
}

bool ActionManager::Can_Dock_At(const Object *obj, const Object *dock_dest, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x004960F0, 0x008E0199), this, obj, dock_dest, source);
#else
    return false;
#endif
}

bool ActionManager::Can_Snipe_Vehicle(const Object *obj, const Object *vehicle_to_snipe, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType>(
        PICK_ADDRESS(0x00497240, 0x008E1837), this, obj, vehicle_to_snipe, source);
#else
    return false;
#endif
}
