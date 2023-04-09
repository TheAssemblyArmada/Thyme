/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Spawn Behavior
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
#include "damage.h"
#include "object.h"

class SpawnBehaviorInterface
{
public:
    virtual bool May_Spawn_Self_Task_AI(float f) = 0;
    virtual void On_Spawn_Death(ObjectID id, DamageInfo *info) = 0;
    virtual Object *Get_Closest_Slave(const Coord3D *pos) = 0;
    virtual void Order_Slaves_To_Attack_Target(Object *target, int shots, CommandSourceType source) = 0;
    virtual void Order_Slaves_To_Attack_Position(const Coord3D *pos, int shots, CommandSourceType source) = 0;
    virtual CanAttackResult Get_Can_Any_Slaves_Attack_Specific_Target(
        AbleToAttackType type, const Object *target, CommandSourceType source) = 0;
    virtual CanAttackResult Get_Can_Any_Slaves_Use_Weapon_Against_Target(
        AbleToAttackType type, const Object *target, const Coord3D *location, CommandSourceType source) = 0;
    virtual bool Can_Any_Slaves_Attack() = 0;
    virtual void Order_Slaves_To_Go_Idle(CommandSourceType source) = 0;
    virtual void Order_Slaves_Disabled_Until(DisabledType type, unsigned int frame) = 0;
    virtual void Order_Slaves_To_Clear_Disabled(DisabledType type) = 0;
    virtual void Give_Slaves_Stealth_Upgrade(bool stealth) = 0;
    virtual bool Are_All_Slaves_Stealthed() const = 0;
    virtual void Reveal_Slaves() = 0;
    virtual bool Do_Slaves_Have_Freedom() const = 0;
};
