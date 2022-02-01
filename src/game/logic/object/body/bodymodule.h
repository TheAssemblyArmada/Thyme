/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Body Module
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
#include "armortemplateset.h"
#include "gametype.h"
#include "object.h"

class DamageInfo;
class DamageInfoInput;

enum MaxHealthChangeType
{
    MAX_HEALTH_SAME_CURRENTHEALTH,
    MAX_HEALTH_PRESERVE_RATIO,
    MAX_HEALTH_ADD_CURRENT_HEALTH_TOO
};

class BodyModuleInterface
{
public:
    virtual void Attempt_Damage(DamageInfo *damage_info) = 0;
    virtual void Attempt_Healing(DamageInfo *damage_info) = 0;
    virtual float Estimate_Damage(DamageInfoInput &damage_info) const = 0;

    virtual float Get_Health() const = 0;
    virtual float Get_Max_Health() const = 0;
    virtual float Get_Initial_Health() const = 0;
    virtual float Get_Previous_Health() const = 0;
    virtual unsigned int Get_Subdual_Damage_Heal_Rate() const = 0;
    virtual float Get_Subdual_Damage_Heal_Amount() const = 0;

    virtual bool Has_Any_Subdual_Damage() const = 0;
    virtual float Get_Current_Subdual_Damage_Amount() const = 0;
    virtual BodyDamageType Get_Damage_State() const = 0;
    virtual void Set_Damage_State(BodyDamageType type) = 0;
    virtual void Set_Aflame(bool flame) = 0;
    virtual void On_Veterancy_Level_Changed(VeterancyLevel old_level, VeterancyLevel new_level, bool play_sound) = 0;

    virtual void Set_Armor_Set_Flag(ArmorSetType ast) = 0;
    virtual void Clear_Armor_Set_Flag(ArmorSetType ast) = 0;
    virtual bool Test_Armor_Set_Flag(ArmorSetType ast) = 0;
    virtual DamageInfo const *Get_Last_Damage_Info() const = 0;
    virtual unsigned int Get_Last_Damage_Timestamp() const = 0;
    virtual unsigned int Get_Last_Healing_Timestamp() const = 0;
    virtual ObjectID Get_Clearable_Last_Attacker() const = 0;
    virtual void Clear_Last_Attacker() = 0;

    virtual bool Get_Front_Crushed() const = 0;
    virtual bool Get_Back_Crushed() const = 0;
    virtual void Set_Initial_Health(int initial_percent) = 0;
    virtual void Set_Max_Health(float max_health, MaxHealthChangeType) = 0;
    virtual void Set_Front_Crushed(bool v) = 0;
    virtual void Set_Back_Crushed(bool v) = 0;

    virtual void Apply_Damage_Scalar(float damage) = 0;
    virtual float Get_Damage_Scalar() const = 0;
    virtual void Internal_Change_Health(float delta) = 0;
    virtual void Set_Indestructible(bool indestructable) = 0;
    virtual bool Is_Indestructible() const = 0;
    virtual void Evaluate_Visual_Condition() = 0;
    virtual void Update_Body_Particle_Systems() = 0;
};
