/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Physics Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "physicsupdate.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "gamelogic.h"
#include "globaldata.h"
#include "opencontain.h"

float Height_To_Speed(float height)
{
    return GameMath::Sqrt(GameMath::Fabs(2.0f * g_theWriteableGlobalData->m_gravity * height));
}

void Parse_Height_To_Speed(INI *ini, void *, void *store, const void *)
{
    *static_cast<float *>(store) = Height_To_Speed(ini->Scan_Real(ini->Get_Next_Token()));
}

void Parse_Friction_Per_Sec(INI *ini, void *, void *store, const void *)
{
    *static_cast<float *>(store) = ini->Scan_Real(ini->Get_Next_Token()) * 1.0f / 30.0f;
}

PhysicsBehaviorModuleData::PhysicsBehaviorModuleData() :
    m_mass(1.0f),
    m_shockResitance(0),
    m_shockMaxYaw(0.05f),
    m_shockMaxPitch(0.025f),
    m_shockMaxRoll(0.025f),
    m_forwardFriction(0.15f),
    m_lateralFriction(0.15f),
    m_zFriction(0.8f),
    m_aerodynamicCoeff(0.0f),
    m_centerOfMassOffset(0),
    m_allowBouncing(false),
    m_allowCollideForce(true),
    m_killWhenRestingOnGround(false),
    m_minFallHeightForDamage(Height_To_Speed(40.0f)),
    m_fallHeightDamageFactor(1.0f),
    m_pitchRollYawFactor(2.0f),
    m_vehicleCrashesIntoBuildingWeapon(g_theWeaponStore->Find_Weapon_Template("VehicleCrashesIntoBuildingWeapon")),
    m_vehicleCrashesIntoNonBuildingWeapon(g_theWeaponStore->Find_Weapon_Template("VehicleCrashesIntoNonBuildingWeapon"))
{
}

void PhysicsBehaviorModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    static const FieldParse dataFieldParse[] = {
        { "Mass", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_mass) },
        { "ShockResistance",
            &INI::Parse_Positive_Non_Zero_Real,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_shockResitance) },
        { "ShockMaxYaw", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockMaxYaw) },
        { "ShockMaxPitch",
            &INI::Parse_Positive_Non_Zero_Real,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_shockMaxPitch) },
        { "ShockMaxRoll", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockMaxRoll) },
        { "ForwardFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_forwardFriction) },
        { "LateralFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_lateralFriction) },
        { "ZFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_zFriction) },
        { "AerodynamicFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_aerodynamicCoeff) },
        { "CenterOfMassOffset", &INI::Parse_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_centerOfMassOffset) },
        { "AllowBouncing", &INI::Parse_Bool, nullptr, offsetof(PhysicsBehaviorModuleData, m_allowBouncing) },
        { "AllowCollideForce", &INI::Parse_Bool, nullptr, offsetof(PhysicsBehaviorModuleData, m_allowCollideForce) },
        { "KillWhenRestingOnGround",
            &INI::Parse_Bool,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_killWhenRestingOnGround) },
        { "MinFallHeightForDamage",
            &Parse_Height_To_Speed,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_minFallHeightForDamage) },
        { "FallHeightDamageFactor",
            &INI::Parse_Real,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_fallHeightDamageFactor) },
        { "PitchRollYawFactor", &INI::Parse_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_pitchRollYawFactor) },
        { "VehicleCrashesIntoBuildingWeaponTemplate",
            &WeaponStore::Parse_Weapon_Template,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_vehicleCrashesIntoBuildingWeapon) },
        { "VehicleCrashesIntoNonBuildingWeaponTemplate",
            &WeaponStore::Parse_Weapon_Template,
            nullptr,
            offsetof(PhysicsBehaviorModuleData, m_vehicleCrashesIntoNonBuildingWeapon) },
    };

    UpdateModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

void PhysicsBehavior::Apply_Motive_Force(const Coord3D *force)
{
#ifdef GAME_DLL
    Call_Method<void, PhysicsBehavior, const Coord3D *>(PICK_ADDRESS(0x005CC300, 0x007E523E), this, force);
#endif
}

void PhysicsBehavior::Scrub_Velocity_2D(float desired_velocity)
{
    if (desired_velocity < 0.001f) {
        m_vel.x = 0.0f;
        m_vel.y = 0.0f;
    } else {
        float velocity = GameMath::Sqrt(m_vel.x * m_vel.x + m_vel.y * m_vel.y);

        if (desired_velocity > velocity) {
            return;
        }

        float velocity_change = desired_velocity / velocity;
        m_vel.x = velocity_change * m_vel.x;
        m_vel.y = velocity_change * m_vel.y;
    }

    m_velMag = -1.0;
}

float PhysicsBehavior::Get_Mass() const
{
    ContainModuleInterface *contain = Get_Object()->Get_Contain();

    if (contain != nullptr) {
        return contain->Get_Contained_Items_Mass() + m_mass;
    }

    return m_mass;
}

float PhysicsBehavior::Get_Velocity_Magnitude() const
{
    if (m_velMag == -1.0f) {
        m_velMag = GameMath::Sqrt(GameMath::Square(m_vel.x) + GameMath::Square(m_vel.y) + GameMath::Square(m_vel.z));
    }

    return m_velMag;
}

// TODO investigate doesn't account for diagonal movement
float PhysicsBehavior::Get_Forward_Speed_2D() const
{
    const Coord3D *dir = Get_Object()->Get_Unit_Dir_Vector2D();
    float x = m_vel.x * dir->x;
    float y = m_vel.y * dir->y;
    float xy = x + y;
    float len = GameMath::Sqrt(x * x + y * y);

    if (xy < 0.0f) {
        return -len;
    } else {
        return len;
    }
}

// TODO investigate doesn't account for diagonal movement
float PhysicsBehavior::Get_Forward_Speed_3D() const
{
    Vector3 xv = Get_Object()->Get_Transform_Matrix()->Get_X_Vector();
    float x = xv.X * m_vel.x;
    float y = xv.Y * m_vel.y;
    float z = xv.Z * m_vel.z;
    float xyz = x + y + z;
    float len = GameMath::Sqrt(x * x + y * y + z * z);

    if (xyz < 0.0f) {
        return -len;
    } else {
        return len;
    }
}

bool PhysicsBehavior::Is_Motive() const
{
    return g_theGameLogic->Get_Frame() < (unsigned int)m_motiveForceApplied;
}
