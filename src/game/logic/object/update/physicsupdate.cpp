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
#include "aiupdate.h"
#include "audiomanager.h"
#include "bodymodule.h"
#include "damage.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "gamelogic.h"
#include "globaldata.h"
#include "opencontain.h"
#include "scriptengine.h"
#include "terrainlogic.h"
#include <algorithm>

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
    m_killWhenRestingOnGround(false),
    m_allowBouncing(false),
    m_allowCollideForce(true),
    m_minFallHeightForDamage(Height_To_Speed(40.0f)),
    m_fallHeightDamageFactor(1.0f),
    m_pitchRollYawFactor(2.0f),
    m_vehicleCrashesIntoBuildingWeapon(g_theWeaponStore->Find_Weapon_Template("VehicleCrashesIntoBuildingWeapon")),
    m_vehicleCrashesIntoNonBuildingWeapon(g_theWeaponStore->Find_Weapon_Template("VehicleCrashesIntoNonBuildingWeapon"))
{
}

void PhysicsBehaviorModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "Mass", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_mass) },
        { "ShockResistance", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockResitance) },
        { "ShockMaxYaw", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockMaxYaw) },
        { "ShockMaxPitch", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockMaxPitch) },
        { "ShockMaxRoll", &INI::Parse_Positive_Non_Zero_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_shockMaxRoll) },
        { "ForwardFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_forwardFriction) },
        { "LateralFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_lateralFriction) },
        { "ZFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_zFriction) },
        { "AerodynamicFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(PhysicsBehaviorModuleData, m_aerodynamicCoeff) },
        { "CenterOfMassOffset", &INI::Parse_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_centerOfMassOffset) },
        { "AllowBouncing", &INI::Parse_Bool, nullptr, offsetof(PhysicsBehaviorModuleData, m_allowBouncing) },
        { "AllowCollideForce", &INI::Parse_Bool, nullptr, offsetof(PhysicsBehaviorModuleData, m_allowCollideForce) },
        { "KillWhenRestingOnGround", &INI::Parse_Bool, nullptr, offsetof(PhysicsBehaviorModuleData, m_killWhenRestingOnGround) },
        { "MinFallHeightForDamage", &Parse_Height_To_Speed, nullptr, offsetof(PhysicsBehaviorModuleData, m_minFallHeightForDamage) },
        { "FallHeightDamageFactor", &INI::Parse_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_fallHeightDamageFactor) },
        { "PitchRollYawFactor", &INI::Parse_Real, nullptr, offsetof(PhysicsBehaviorModuleData, m_pitchRollYawFactor) },
        { "VehicleCrashesIntoBuildingWeaponTemplate", &WeaponStore::Parse_Weapon_Template, nullptr, offsetof(PhysicsBehaviorModuleData, m_vehicleCrashesIntoBuildingWeapon) },
        { "VehicleCrashesIntoNonBuildingWeaponTemplate", &WeaponStore::Parse_Weapon_Template, nullptr, offsetof(PhysicsBehaviorModuleData, m_vehicleCrashesIntoNonBuildingWeapon) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    UpdateModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

PhysicsBehavior::PhysicsBehavior(Thing *thing, const ModuleData *module_data) :
    UpdateModule(thing, module_data),
    m_yawRate(0.0f),
    m_rollRate(0.0f),
    m_pitchRate(0.0f),
    m_bounceSound(nullptr),
    m_ignoreCollisionsWith(INVALID_OBJECT_ID),
    m_flags(0),
    m_mass(Get_Physics_Behavior_Module_Data()->m_mass),
    m_currentOverlap(INVALID_OBJECT_ID),
    m_previousOverlap(INVALID_OBJECT_ID),
    m_collided(INVALID_OBJECT_ID),
    m_motiveForceApplied(0),
    m_extraBounciness(0.0f),
    m_extraFriction(0.0f),
    m_projectileUpdateInterface(nullptr),
    m_velMag(0.0f)
{
    m_accel.Zero();
    m_prevAccel = m_accel;
    m_vel.Zero();
    Set_Allow_Bouncing(Get_Physics_Behavior_Module_Data()->m_allowBouncing);
    Set_Allow_Collide_Force(Get_Physics_Behavior_Module_Data()->m_allowCollideForce);
    Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MIN);
}

PhysicsBehavior::~PhysicsBehavior()
{
    if (m_bounceSound != nullptr) {
        m_bounceSound->Delete_Instance();
    }
}

NameKeyType PhysicsBehavior::Get_Module_Name_Key() const
{
    static const NameKeyType nk = g_theNameKeyGenerator->Name_To_Key("PhysicsBehavior");
    return nk;
}

ProjectileUpdateInterface *Get_PUI(Object *obj)
{
    if (!obj->Is_KindOf(KINDOF_PROJECTILE)) {
        return nullptr;
    }

    for (BehaviorModule **i = obj->Get_All_Modules(); *i != nullptr; i++) {
        ProjectileUpdateInterface *update = (*i)->Get_Projectile_Update_Interface();

        if (update != nullptr) {
            return update;
        }
    }

    return nullptr;
}

void PhysicsBehavior::On_Object_Created()
{
    m_projectileUpdateInterface = Get_PUI(Get_Object());
}

SleepyUpdatePhase PhysicsBehavior::Get_Update_Phase() const
{
    return SLEEPY_UPDATE_PHASE_1;
}

void PhysicsBehavior::CRC_Snapshot(Xfer *xfer)
{
    UpdateModule::CRC_Snapshot(xfer);
}

void PhysicsBehavior::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 2;
    xfer->xferVersion(&version, 2);
    UpdateModule::Xfer_Snapshot(xfer);
    xfer->xferReal(&m_yawRate);
    xfer->xferReal(&m_rollRate);
    xfer->xferReal(&m_pitchRate);
    xfer->xferCoord3D(&m_accel);
    xfer->xferCoord3D(&m_prevAccel);
    xfer->xferCoord3D(&m_vel);

    if (version < 2) {
        Coord3D c;
        c.Zero();
        xfer->xferCoord3D(&c);
    }

    xfer->xferUser(&m_turning, sizeof(m_turning));
    xfer->xferObjectID(&m_ignoreCollisionsWith);
    xfer->xferInt(&m_flags);
    xfer->xferReal(&m_mass);
    xfer->xferObjectID(&m_currentOverlap);
    xfer->xferObjectID(&m_previousOverlap);
    xfer->xferUnsignedInt(&m_motiveForceApplied);
    xfer->xferReal(&m_extraBounciness);
    xfer->xferReal(&m_extraFriction);
    xfer->xferReal(&m_velMag);
}

void PhysicsBehavior::Load_Post_Process()
{
    UpdateModule::Load_Post_Process();
}

CollideModuleInterface *PhysicsBehavior::Get_Collide()
{
    return this;
}

bool Is_Low_Velocity(const Coord3D &vel)
{
    return GameMath::Fabs(vel.x) < 0.01f && GameMath::Fabs(vel.y) < 0.01f && GameMath::Fabs(vel.z) < 0.01f;
}

UpdateSleepTime PhysicsBehavior::Update()
{
    Object *obj = Get_Object();
    const PhysicsBehaviorModuleData *data = Get_Physics_Behavior_Module_Data();
    float z = 0.0f;
    bool b = false;
    captainslog_dbgassert(!Get_Flag(UPDATING), "impossible");
    Set_Flag(UPDATING, true);

    if (!Get_Flag(UPDATE_EVER_RUN)) {
        Set_Flag(WAS_AIRBORNE_LAST_FRAME, obj->Is_Above_Terrain());
    }

    Coord3D pos = *obj->Get_Position();
    m_prevAccel = m_accel;
    Coord3D bounce;

    if (!obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
        Matrix3D tm = *obj->Get_Transform_Matrix();
        Apply_Gravitational_Forces();
        Apply_Frictional_Forces();
        m_vel += m_accel;

        if (GameMath::Fabs(m_vel.x) < 0.001f) {
            m_vel.x = 0.0f;
        }

        if (GameMath::Fabs(m_vel.y) < 0.001f) {
            m_vel.y = 0.0f;
        }

        if (GameMath::Fabs(m_vel.z) < 0.001f) {
            m_vel.z = 0.0f;
        }

        m_velMag = -1.0f;
        float oldz = tm.Get_Z_Translation();

        if (obj->Get_Status(OBJECT_STATUS_IS_BRAKING)) {
            if (!obj->Is_KindOf(KINDOF_PROJECTILE)) {
                tm.Adjust_Z_Translation(m_vel.z);
            }
        } else {
            tm.Adjust_X_Translation(m_vel.x);
            tm.Adjust_Y_Translation(m_vel.y);
            tm.Adjust_Z_Translation(m_vel.z);
        }

        if (GameMath::Is_Nan(tm.Get_X_Translation()) || GameMath::Is_Nan(tm.Get_Y_Translation())
            || GameMath::Is_Nan(tm.Get_Z_Translation())) {
            captainslog_dbgassert(false, "Object position is NAN, deleting.");
            g_theGameLogic->Destroy_Object(obj);
        }

        if (Get_Stunned()
                && (GameMath::Fabs(m_vel.x) < 0.5f && GameMath::Fabs(m_vel.y) < 0.5f && GameMath::Fabs(m_vel.z) < 0.5f)
            || !obj->Is_Significantly_Above_Terrain()) {
            Set_Stunned(false);
            Get_Object()->Clear_Model_Condition_State(MODELCONDITION_STUNNED);
        }

        if (Get_Flag(MOVING)) {
            float yaw = m_yawRate * data->m_pitchRollYawFactor;
            float pitch = m_pitchRate * data->m_pitchRollYawFactor;
            float roll = m_rollRate * data->m_pitchRollYawFactor;
            float offset = Get_Center_Of_Mass_Offset();

            if (offset != 0.0f) {
                Vector3 v = tm.Get_X_Vector();
                float angle = GameMath::Atan2(v.Z, GameMath::Sqrt(GameMath::Square(v.X) + GameMath::Square(v.Y)));

                if (offset > 0.0f) {
                    angle = DEG_TO_RADF(90) - angle;
                } else {
                    angle = angle + DEG_TO_RADF(-90);
                }

                pitch = pitch * GameMath::Sin(angle);
            }

            tm.Rotate_X(roll);
            tm.Rotate_Y(pitch);
            tm.Rotate_Z(yaw);
        }

        float height = g_theTerrainLogic->Get_Layer_Height(
            tm.Get_X_Translation(), tm.Get_Y_Translation(), obj->Get_Layer(), nullptr, true);

        if (obj->Get_Status_Bits().Test(OBJECT_STATUS_DECK_HEIGHT_OFFSET)) {
            height += obj->Get_Carrier_Deck_Height();
        }

        b = Handle_Bounce(oldz, tm.Get_Z_Translation(), height, &bounce);
        z = m_vel.z;

        if (tm.Get_Z_Translation() <= height) {
            m_vel.z += height - tm.Get_Z_Translation();

            if (m_vel.z > 0.0f) {
                m_vel.z = 0.0f;
            }

            m_velMag = -1.0f;
            tm.Set_Z_Translation(height);
            Set_Flag(IS_AIRBORNE, false);

            if (Get_Flag(STUNNED)) {
                obj->Clear_Model_Condition_State(MODELCONDITION_STUNNED_FLAILING);
                obj->Set_Model_Condition_State(MODELCONDITION_STUNNED);
            }
        } else {
            if (tm.Get_Z_Translation() > height) {
                if (Get_Flag(FREEFALL)) {
                    obj->Set_Disabled(DISABLED_TYPE_DISABLED_FREEFALL);
                    obj->Set_Model_Condition_State(MODELCONDITION_FREEFALL);
                } else if (Get_Flag(STICK_TO_GROUND) && !Get_Flag(IS_AIRBORNE)) {
                    tm.Set_Z_Translation(height);
                }
            }
        }

        if (b) {
            Set_Angles(obj->Get_Transform_Matrix()->Get_Z_Rotation(), 0.0f, 0.0f);
            Matrix3D tm2 = *obj->Get_Transform_Matrix();
            tm2.Set_Translation(tm.Get_Translation());
            obj->Set_Transform_Matrix(&tm2);
        } else {
            obj->Set_Transform_Matrix(&tm);
        }
    }

    m_accel.Zero();
    m_previousOverlap = m_currentOverlap;
    m_currentOverlap = INVALID_OBJECT_ID;

    if (b && Get_Flag(ALLOW_BOUNCE)) {
        Apply_Force(&bounce);
    }

    bool above = obj->Is_Above_Terrain();

    if (Get_Flag(WAS_AIRBORNE_LAST_FRAME)) {
        if (!above && !Get_Flag(FLAG_UNK3)) {
            Do_Bounce_Sound(pos);
            Coord3D normal;
            normal.x = 0.0f;
            normal.y = 0.0f;
            normal.z = -1.0f;
            obj->On_Collide(nullptr, obj->Get_Position(), &normal);
            float fall_height = -z - data->m_minFallHeightForDamage;

            if (fall_height > 0.0f && m_projectileUpdateInterface == nullptr) {
                if (0.01f >= GameMath::Fabs(m_vel.x) || 3.0f <= GameMath::Fabs(z / m_vel.x)) {
                    if (0.01f >= GameMath::Fabs(m_vel.y) || 3.0f <= GameMath::Fabs(z / m_vel.y)) {
                        DamageInfo info;
                        info.m_in.m_damageType = DAMAGE_FALLING;
                        info.m_in.m_deathType = DEATH_SPLATTED;
                        info.m_in.m_sourceID = obj->Get_ID();
                        info.m_in.m_amount = Get_Mass() * fall_height * data->m_fallHeightDamageFactor;
                        info.m_in.m_shockWaveAmount = 0.0f;

                        obj->Attempt_Damage(&info);

                        if (obj->Is_Effectively_Dead()) {
                            obj->Clear_Model_Condition_State(MODELCONDITION_SPLATTED);
                        }
                    }
                }
            }
        }
    }

    if (!above) {
        Set_Flag(FREEFALL, false);

        if (obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_FREEFALL)) {
            obj->Clear_Disabled(DISABLED_TYPE_DISABLED_FREEFALL);
        }

        obj->Clear_Model_Condition_State(MODELCONDITION_FREEFALL);
    }

    if (data->m_killWhenRestingOnGround && !above && Is_Low_Velocity(m_vel)) {
        if ((!obj->Is_KindOf(KINDOF_DRONE) || obj->Is_Effectively_Dead()
                || obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNMANNED))) {
            obj->Kill(DAMAGE_UNRESISTABLE, DEATH_NORMAL);
        }
    }

    Set_Flag(UPDATE_EVER_RUN, true);
    Set_Flag(WAS_AIRBORNE_LAST_FRAME, above);
    Set_Flag(UPDATING, false);
    return Calc_Sleep_Time();
}

BitFlags<DISABLED_TYPE_COUNT> PhysicsBehavior::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

void PhysicsBehavior::On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal)
{
    if (m_projectileUpdateInterface == nullptr || !m_projectileUpdateInterface->Projectile_Handle_Collision(other)) {
        Object *obj = Get_Object();
        Object *contained = obj->Get_Contained_By();

        if (other == nullptr) {
            if (contained != nullptr) {
                contained->On_Collide(nullptr, loc, normal);
            }

            return;
        }

        if (other->Get_Contained_By() != obj && contained != other && !obj->Get_Status(OBJECT_STATUS_PARACHUTING)
            && !other->Get_Status(OBJECT_STATUS_PARACHUTING)) {
            AIUpdateInterface *update = obj->Get_AI_Update_Interface();

            if (update != nullptr) {
                if (update->Get_Ignored_Obstacle_ID() == other->Get_ID()) {
                    if (obj->Is_KindOf(KINDOF_INFANTRY) && other->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNMANNED)) {
                        other->Clear_Disabled(DISABLED_TYPE_DISABLED_UNMANNED);
                        other->Set_Captured(true);
                        other->Defect(obj->Get_Team(), 0);
                        g_theScriptEngine->Transfer_Object_Name(obj->Get_Name(), other);
                        g_theGameLogic->Destroy_Object(obj);
                    }

                    return;
                }
            }

            AIUpdateInterface *update2 = other->Get_AI_Update_Interface();

            if (update2 == nullptr || update2->Get_Ignored_Obstacle_ID() != obj->Get_ID()) {
                if (!Is_Ignoring_Collisions_With(other->Get_ID())) {
                    bool immobile1 = obj->Is_KindOf(KINDOF_IMMOBILE);
                    bool immobile2 = other->Is_KindOf(KINDOF_IMMOBILE);
                    PhysicsBehavior *physics = other->Get_Physics();

                    if (physics != nullptr) {
                        if (physics->Is_Ignoring_Collisions_With(obj->Get_ID())) {
                            return;
                        }
                    } else if (!immobile2) {
                        return;
                    }

                    if (!Check_For_Overlap_Collision(other)) {
                        if (update == nullptr
                            || (obj->Is_Effectively_Dead() || obj->Get_Status(OBJECT_STATUS_PARACHUTING)) && immobile2
                            || update->Process_Collision(this, other)) {
                            Coord3D center_pos;
                            obj->Get_Geometry_Info().Get_Center_Position(*obj->Get_Position(), center_pos);
                            Coord3D other_center_pos;
                            other->Get_Geometry_Info().Get_Center_Position(*other->Get_Position(), other_center_pos);
                            Coord3D center_diff = other_center_pos - center_pos;
                            float radius;
                            float other_radius;
                            float diff_radius;

                            if (obj->Is_Above_Terrain()) {
                                radius = obj->Get_Geometry_Info().Get_Bounding_Sphere_Radius();
                                other_radius = other->Get_Geometry_Info().Get_Bounding_Sphere_Radius();
                                diff_radius = GameMath::Square(center_diff.x) + GameMath::Square(center_diff.y)
                                    + GameMath::Square(center_diff.z);
                            } else {
                                radius = obj->Get_Geometry_Info().Get_Bounding_Circle_Radius();
                                other_radius = other->Get_Geometry_Info().Get_Bounding_Circle_Radius();
                                diff_radius = GameMath::Square(center_diff.x) + GameMath::Square(center_diff.y);
                                center_diff.z = 0.0f;
                            }

                            if (GameMath::Square(radius + other_radius) >= diff_radius) {
                                m_collided = other->Get_ID();
                                float f1 = GameMath::Sqrt(diff_radius);
                                float f2 = radius + other_radius - f1;

                                if (f1 < 1.0f) {
                                    f1 = 1.0f;
                                }

                                if (Get_Allow_Collide_Force()) {
                                    float f3;

                                    if (immobile2 && !obj->Is_Destroyed()) {
                                        if (obj->Get_Status(OBJECT_STATUS_PARACHUTING)) {
                                            Object *contained2 = obj;

                                            for (Object *i = obj->Get_Contained_By(); i != nullptr;
                                                 i = contained2->Get_Contained_By()) {
                                                contained2 = contained2->Get_Contained_By();
                                            }

                                            Coord3D pos = *contained2->Get_Position();
                                            pos.x = pos.x - (radius * 0.1f) * center_diff.x / f1;
                                            pos.y = pos.y - (radius * 0.1f) * center_diff.y / f1;
                                            contained2->Set_Position(&pos);
                                            contained2->Get_Physics()->Scrub_Velocity_2D(0.0f);
                                            return;
                                        }

                                        float stiffness = g_theWriteableGlobalData->m_structureStiffness;

                                        if (stiffness < 0.01f) {
                                            stiffness = 0.01f;
                                        }

                                        if (stiffness > 0.99f) {
                                            stiffness = 0.99f;
                                        }

                                        float magnitude = Get_Velocity_Magnitude();

                                        if (magnitude < 0.0066666668f) {
                                            magnitude = 0.0066666668f;
                                        }

                                        f3 = Get_Mass() * -magnitude * stiffness;

                                        if (center_diff.z < 0.0f
                                            && obj->Get_Position()->z
                                                >= g_theWriteableGlobalData->m_defaultStructureRubbleHeight) {
                                            if (other->Is_KindOf(KINDOF_STRUCTURE)) {
                                                if (obj->Is_KindOf(KINDOF_VEHICLE)) {
                                                    g_theWeaponStore->Create_And_Fire_Temp_Weapon(
                                                        Get_Physics_Behavior_Module_Data()
                                                            ->m_vehicleCrashesIntoBuildingWeapon,
                                                        obj,
                                                        obj->Get_Position());
                                                }

                                                g_theGameLogic->Destroy_Object(obj);
                                                return;
                                            }

                                            if (obj->Is_KindOf(KINDOF_VEHICLE)) {
                                                g_theWeaponStore->Create_And_Fire_Temp_Weapon(
                                                    Get_Physics_Behavior_Module_Data()
                                                        ->m_vehicleCrashesIntoNonBuildingWeapon,
                                                    obj,
                                                    obj->Get_Position());
                                            }
                                        }

                                        m_vel.x = 0.0f;
                                        m_vel.y = 0.0f;
                                        m_vel.z = 0.0f;
                                        m_velMag = -1.0f;
                                    } else {
                                        if (f2 > 5.0f) {
                                            f2 = 5.0f;
                                        }

                                        f3 = -f2;
                                    }

                                    Coord3D force;
                                    force.x = f3 * center_diff.x / f1;
                                    force.y = f3 * center_diff.y / f1;
                                    force.z = f3 * center_diff.z / f1;
                                    captainslog_dbgassert(!GameMath::Is_Nan(force.x) && !GameMath::Is_Nan(force.y)
                                            && !GameMath::Is_Nan(force.z),
                                        "PhysicsBehavior::onCollide force NAN!");
                                    Apply_Force(&force);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool PhysicsBehavior::Would_Like_To_Collide_With(Object const *other)
{
    return false;
}

bool PhysicsBehavior::Is_Hijacked_Vehicle_Crate_Collide()
{
    return false;
}

bool PhysicsBehavior::Is_Sabotage_Building_Crate_Collide()
{
    return false;
}

bool PhysicsBehavior::Is_Car_Bomb_Crate_Collide()
{
    return false;
}

bool PhysicsBehavior::Is_Railroad()
{
    return false;
}

bool PhysicsBehavior::Is_Salvage_Crate_Collide()
{
    return false;
}

void PhysicsBehavior::Apply_Motive_Force(const Coord3D *force)
{
    m_motiveForceApplied = false;
    Apply_Force(force);
    m_motiveForceApplied = g_theGameLogic->Get_Frame() + 10;
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

void PhysicsBehavior::Set_Angles(float yaw, float pitch, float roll)
{
    const Coord3D *pos = Get_Object()->Get_Position();
    Matrix3D tm;
    tm.Make_Identity();
    tm.Translate(pos->x, pos->y, pos->z);
    tm.In_Place_Pre_Rotate_X(-roll);
    tm.In_Place_Pre_Rotate_Y(pitch);
    tm.In_Place_Pre_Rotate_Z(yaw);
    Get_Object()->Set_Transform_Matrix(&tm);
}

void PhysicsBehavior::Do_Bounce_Sound(Coord3D const &pos)
{
    if (m_bounceSound != nullptr) {
        AudioEventRTS sound(m_bounceSound->m_event);
        sound.Set_Object_ID(Get_Object()->Get_ID());
        g_theAudio->Add_Audio_Event(&sound);
    }
}

void PhysicsBehavior::Apply_Gravitational_Forces()
{
    m_accel.z += g_theWriteableGlobalData->m_gravity;
}

void PhysicsBehavior::Apply_Force(const Coord3D *force)
{
    captainslog_dbgassert(!GameMath::Is_Nan(force->x) && !GameMath::Is_Nan(force->y) && !GameMath::Is_Nan(force->z),
        "PhysicsBehavior::applyForce force NAN!");

    if (!GameMath::Is_Nan(force->x) && !GameMath::Is_Nan(force->y) && !GameMath::Is_Nan(force->z)) {
        float mass = Get_Mass();
        Coord3D new_force = *force;

        if (Is_Motive()) {
            const Coord3D *dir = Get_Object()->Get_Unit_Dir_Vector2D();
            float f = -dir->y * force->x + force->y * dir->x;
            new_force.x = -dir->y * f;
            new_force.y = dir->x * f;
        }

        m_accel.x = new_force.x * (1.0f / mass) + m_accel.x;
        m_accel.y = new_force.y * (1.0f / mass) + m_accel.y;
        m_accel.z = new_force.z * (1.0f / mass) + m_accel.z;
    }

    if (!Get_Flag(UPDATING)) {
        Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MIN);
    }
}

bool PhysicsBehavior::Handle_Bounce(float oldz, float newz, float groundz, Coord3D *bounce_force)
{
    if (Get_Flag(ALLOW_BOUNCE) && newz <= groundz) {
        float stiffness = g_theWriteableGlobalData->m_groundStiffness;

        if (stiffness < 0.01f) {
            stiffness = 0.01f;
        }

        if (stiffness > 0.99f) {
            stiffness = 0.99f;
        }

        float accel = 0.0f;

        float z = Get_Velocity().z;

        if (oldz > groundz && z < 0.0f) {
            accel = GameMath::Fabs(z) * stiffness;
        }

        bounce_force->x = 0.0f;
        bounce_force->y = 0.0f;
        bounce_force->z = Get_Mass() * accel;

        Apply_YPR_Damping(0.7f);

        if (z < 0.0f) {
            Vector3 z_vector;
            Get_Object()->Get_Transform_Matrix()->Get_Z_Vector(&z_vector);
            float roll;

            if (z_vector.Z > 0.0f) {
                roll = 0.0f;
            } else {
                roll = DEG_TO_RADF(180.0f);
            }

            Set_Angles(Get_Object()->Get_Transform_Matrix()->Get_Z_Rotation(), 0.0f, roll);
        }

        if (bounce_force->z > 0.0f) {
            Test_Stunned_Unit_For_Destruction();
            return true;
        } else {
            Set_Allow_Bouncing(m_canBounce);
            return false;
        }
    } else {
        bounce_force->Zero();
        return false;
    }
}

bool Is_Zero(const Coord3D &coord)
{
    return coord.x == 0.0f && coord.y == 0.0f && coord.z == 0.0f;
}

UpdateSleepTime PhysicsBehavior::Calc_Sleep_Time() const
{
    if (!Is_Zero(m_vel)) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    if (!Is_Zero(m_accel)) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    if (Get_Flag(MOVING)) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    if (Is_Motive()) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    const Object *obj = Get_Object();

    if (obj->Get_Layer() != LAYER_GROUND) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    if (obj->Is_Above_Terrain() || Get_Current_Overlap() || Get_Previous_Overlap() || !Get_Flag(UPDATE_EVER_RUN)) {
        return UPDATE_SLEEP_TIME_MIN;
    }

    return UPDATE_SLEEP_TIME_MAX;
}

void PhysicsBehavior::Apply_Frictional_Forces()
{
    Object *obj = Get_Object();
    bool taxiing = false;

    if (obj->Get_Status(OBJECT_STATUS_DECK_HEIGHT_OFFSET)) {
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (update->Get_Cur_Locomotor_Set() == LOCOMOTORSET_TAXIING) {
                taxiing = true;
            }
        }
    }

    if (Get_Flag(APPLY_FRICTION2D_WHEN_AIRBORNE) || !obj->Is_Significantly_Above_Terrain() || taxiing) {
        Apply_YPR_Damping(1.0f - 0.15f);

        if (m_vel.x != 0.0f || m_vel.y != 0.0f) {
            const Coord3D *dir = obj->Get_Unit_Dir_Vector2D();
            float mass = Get_Mass();
            float f1 = -dir->y * m_vel.x + m_vel.y * dir->x;
            float f2 = -dir->y * f1;
            float f3 = dir->x * f1;
            float f4 = Get_Lateral_Friction() * mass;

            Coord3D force;
            force.x = -(f4 * f2);
            force.y = -(f4 * f3);
            force.z = 0.0f;

            if (!Is_Motive()) {
                float f5 = m_vel.x * dir->x + m_vel.y * dir->y;
                float f6 = f5 * dir->x;
                float f7 = f5 * dir->y;
                float f8 = Get_Forward_Friction() * mass;
                force.x = force.x - f8 * f6;
                force.y = force.y - f8 * f7;
            }

            Apply_Force(&force);
        }
    } else {
        float friction = -Get_Aerodynamic_Friction();
        m_accel.x = friction * m_vel.x + m_accel.x;
        m_accel.y = friction * m_vel.y + m_accel.y;
        m_accel.z = friction * m_vel.z + m_accel.z;
        Apply_YPR_Damping(friction + 1.0f);
    }
}

ModuleData *PhysicsBehavior::Friend_New_Module_Data(INI *ini)
{
    PhysicsBehaviorModuleData *data = new PhysicsBehaviorModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, PhysicsBehaviorModuleData::Build_Field_Parse);
    }

    return data;
}

Module *PhysicsBehavior::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(PhysicsBehavior, thing, module_data);
}

bool Perps_Logically_Equal(float f1, float f2)
{
    return GameMath::Fabs(f1 - f2) <= 0.15f;
}

bool PhysicsBehavior::Check_For_Overlap_Collision(Object *other)
{
    if (Is_Low_Velocity(Get_Velocity())) {
        return false;
    }

    Object *obj = Get_Object();
    bool squish1 = obj->Can_Crush_Or_Squish(other, TEST_TYPE_0);
    bool squish2 = other->Can_Crush_Or_Squish(obj, TEST_TYPE_0);

    if (squish1 && squish2) {
        captainslog_dbgassert(false,
            "%s (Crusher:%d, Crushable:%d) is attempting to crush %s (Crusher:%d, Crushable:%d) but it is reciprocating -- "
            "shouldn't be possible!",
            obj->Get_Template()->Get_Name().Str(),
            obj->Get_Crusher_Level(),
            obj->Get_Crushable_Level(),
            other->Get_Template()->Get_Name().Str(),
            other->Get_Crusher_Level(),
            other->Get_Crushable_Level());
        return false;
    }

    if (squish2) {
        return true;
    } else if (!this) {
        return false;
    } else if (!squish1) {
        return false;
    } else {
        Add_Overlap(other);

        if (!Was_Previously_Overlapped(other)) {
            DamageInfo info;
            info.m_in.m_damageType = DAMAGE_CRUSH;
            info.m_in.m_deathType = DEATH_CRUSHED;
            info.m_in.m_sourceID = obj->Get_ID();
            info.m_in.m_amount = 0.0f;
            other->Attempt_Damage(&info);
        }

        const Coord3D *other_pos = other->Get_Position();
        const Coord3D *pos = obj->Get_Position();
        bool front_crushed = other->Get_Body_Module()->Get_Front_Crushed();
        bool back_crushed = other->Get_Body_Module()->Get_Back_Crushed();

        if (!front_crushed || !back_crushed) {
            bool crushed = false;
            const Coord3D *dir = obj->Get_Unit_Dir_Vector2D();
            const Coord3D *other_dir = other->Get_Unit_Dir_Vector2D();
            float radius = other->Get_Geometry_Info().Get_Major_Radius() / 2.0f;
            float x = radius * other_dir->x;
            float y = radius * other_dir->y;

            int i1 = 3;

            if (front_crushed || back_crushed) {
                if (front_crushed) {
                    i1 = 1;
                } else {
                    i1 = 2;
                }
            } else {
                Coord3D pos1 = *other_pos;
                pos1.x = pos1.x + x;
                pos1.y = pos1.y + y;

                Coord3D pos2 = pos1;
                pos2.x = pos1.x - pos->x;
                pos2.y = pos1.y - pos->y;
                pos2.z = 0.0f;

                float f4 = pos2.x * dir->x + pos2.y * dir->y;

                Coord3D pos3;
                pos3.x = f4 * dir->x;
                pos3.y = f4 * dir->y;
                pos3.z = 0.0f;

                Coord3D pos4;
                pos4.x = pos3.x - pos2.x;
                pos4.y = pos3.y - pos2.y;
                pos4.z = 0.0f;

                float f5 = pos4.Length();
                pos1 = *other_pos;
                pos1.x = pos1.x - x;
                pos1.y = pos1.y - y;

                Coord3D pos5;
                pos5.x = pos1.x - pos->x;
                pos5.y = pos1.y - pos->y;
                pos5.z = 0.0f;

                float f6 = pos5.x * dir->x + pos5.y * dir->y;

                Coord3D pos6;
                pos6.x = f6 * dir->x;
                pos6.y = f6 * dir->y;
                pos6.z = 0.0f;

                Coord3D pos7;
                pos7.x = pos6.x - pos5.x;
                pos7.y = pos6.y - pos5.y;
                pos7.z = 0.0f;

                float f7 = pos7.Length();

                pos1 = *other_pos;

                Coord3D pos8;
                pos8 = pos1;
                pos8.x = pos1.x - pos->x;
                pos8.y = pos1.y - pos->y;
                pos8.z = 0.0f;

                float f8 = pos8.x * dir->x + pos8.y * dir->y;

                Coord3D pos9;
                pos9.x = f8 * dir->x;
                pos9.y = f8 * dir->y;
                pos9.z = 0.0f;

                Coord3D pos10;
                pos10.x = pos9.x - pos8.x;
                pos10.y = pos9.y - pos8.y;
                pos10.z = 0.0;

                float f9 = pos10.Length();

                if (f5 <= f9 || f5 <= f7) {
                    if (Perps_Logically_Equal(f5, f9) || Perps_Logically_Equal(f5, f7)) {
                        float f12 = pos2.Length();

                        if (Perps_Logically_Equal(f5, f9)) {
                            if (f12 < pos8.Length()) {
                                i1 = 2;
                            } else {
                                i1 = 0;
                            }
                        } else if (Perps_Logically_Equal(f5, f7)) {
                            if (f12 < pos5.Length()) {
                                i1 = 2;
                            } else {
                                i1 = 1;
                            }
                        }
                    } else {
                        i1 = 2;
                    }
                } else if (f7 <= f9 || f7 <= f5) {
                    if (Perps_Logically_Equal(f7, f9) || Perps_Logically_Equal(f7, f5)) {
                        float f11 = pos5.Length();

                        if (Perps_Logically_Equal(f7, f9)) {
                            i1 = f11 < pos8.Length();
                        } else if (Perps_Logically_Equal(f7, f5)) {
                            if (f11 < pos2.Length()) {
                                i1 = 1;
                            } else {
                                i1 = 2;
                            }
                        }
                    } else {
                        i1 = 1;
                    }
                } else if (Perps_Logically_Equal(f9, f7) || Perps_Logically_Equal(f9, f5)) {
                    float f10 = pos8.Length();

                    if (Perps_Logically_Equal(f9, f5)) {
                        if (f10 < pos2.Length()) {
                            i1 = 0;
                        } else {
                            i1 = 2;
                        }
                    } else if (Perps_Logically_Equal(f9, f7)) {
                        i1 = f10 >= pos5.Length();
                    }
                } else {
                    i1 = 0;
                }
            }

            float f13 = radius * 2.25f * radius;

            switch (i1) {
                case 0: {
                    Coord3D pos1;
                    pos1 = *other_pos;
                    float f14 = pos1.x - pos->x;
                    float f15 = pos1.y - pos->y;
                    float f16 = f14 * dir->x + f15 * dir->y;
                    float f17 = f14 * f14 + f15 * f15;

                    if (f16 < 0.0f && f17 < f13) {
                        crushed = true;
                    }

                    break;
                }
                case 1: {
                    Coord3D pos1;
                    pos1 = *other_pos;
                    pos1.x = pos1.x - x;
                    pos1.y = pos1.y - y;
                    float f14 = pos1.x - pos->x;
                    float f15 = pos1.y - pos->y;
                    float f16 = f14 * dir->x + f15 * dir->y;
                    float f17 = f14 * f14 + f15 * f15;

                    if (f16 < 0.0f && f17 < f13) {
                        crushed = true;
                    }

                    break;
                }
                case 2: {
                    Coord3D pos1;
                    pos1 = *other_pos;
                    pos1.x = pos1.x + x;
                    pos1.y = pos1.y + y;
                    float f14 = pos1.x - pos->x;
                    float f15 = pos1.y - pos->y;
                    float f16 = f14 * dir->x + f15 * dir->y;
                    float f17 = f14 * f14 + f15 * f15;

                    if (f16 < 0.0f && f17 < f13) {
                        crushed = true;
                    }

                    break;
                }
                default:
                    break;
            }

            if (crushed) {
                DamageInfo info;
                info.m_in.m_damageType = DAMAGE_CRUSH;
                info.m_in.m_deathType = DEATH_CRUSHED;
                info.m_in.m_sourceID = obj->Get_ID();
                info.m_in.m_amount = 999999.0f;
                other->Attempt_Damage(&info);
            }
        }

        return true;
    }
}

void PhysicsBehavior::Add_Overlap(Object *obj)
{
    if (obj != nullptr) {
        if (!Is_Currently_Overlapped(obj)) {
            m_currentOverlap = obj->Get_ID();
        }
    }
}

bool PhysicsBehavior::Was_Previously_Overlapped(Object *obj) const
{
    return obj != nullptr && obj->Get_ID() == m_previousOverlap;
}

bool PhysicsBehavior::Is_Ignoring_Collisions_With(ObjectID id) const
{
    return id != INVALID_OBJECT_ID && id == m_ignoreCollisionsWith;
}

bool PhysicsBehavior::Is_Currently_Overlapped(Object *obj) const
{
    return obj != nullptr && obj->Get_ID() == m_currentOverlap;
}

void PhysicsBehavior::Test_Stunned_Unit_For_Destruction()
{
    if (Get_Flag(STUNNED)) {
        Object *obj = Get_Object();
        const Coord3D *pos = obj->Get_Position();
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (obj->Get_Transform_Matrix()->Get_Z_Vector().Z < 0.0f || obj->Is_Outside_Map()
            || (update != nullptr)
                && (g_theTerrainLogic->Is_Cliff_Cell(pos->x, pos->y)
                        && !update->Has_Locomotor_For_Surface(LOCOMOTOR_SURFACE_CLIFF)
                    || g_theTerrainLogic->Is_Underwater(pos->x, pos->y, nullptr, nullptr)
                        && !update->Has_Locomotor_For_Surface(LOCOMOTOR_SURFACE_WATER))) {
            obj->Kill(DAMAGE_UNRESISTABLE, DEATH_NORMAL);
        }
    }
}

void PhysicsBehavior::Apply_YPR_Damping(float damping)
{
    m_pitchRate = damping * m_pitchRate;
    m_rollRate = damping * m_rollRate;
    m_yawRate = damping * m_yawRate;
    if (m_pitchRate == 0.0f && m_rollRate == 0.0f && m_yawRate == 0.0f) {
        Set_Flag(MOVING, false);
    } else {
        Set_Flag(MOVING, true);
    }
}

float PhysicsBehavior::Get_Aerodynamic_Friction() const
{
    float friction = Get_Physics_Behavior_Module_Data()->m_aerodynamicCoeff + m_extraFriction;

    if (friction < 0.0f) {
        friction = 0.0f;
    }

    if (friction > 0.99f) {
        friction = 0.99f;
    }

    return friction;
}

float PhysicsBehavior::Get_Forward_Friction() const
{
    float friction = Get_Physics_Behavior_Module_Data()->m_forwardFriction + m_extraFriction;

    if (friction < 0.0f) {
        friction = 0.0f;
    }

    if (friction > 0.99f) {
        friction = 0.99f;
    }

    return friction;
}

float PhysicsBehavior::Get_Lateral_Friction() const
{
    float friction = Get_Physics_Behavior_Module_Data()->m_lateralFriction + m_extraFriction;

    if (friction < 0.0f) {
        friction = 0.0f;
    }

    if (friction > 0.99f) {
        friction = 0.99f;
    }

    return friction;
}

float PhysicsBehavior::Get_Z_Friction() const
{
    float friction = Get_Physics_Behavior_Module_Data()->m_zFriction + m_extraFriction;

    if (friction < 0.0f) {
        friction = 0.0f;
    }

    if (friction > 0.99f) {
        friction = 0.99f;
    }

    return friction;
}

void PhysicsBehavior::Add_Velocity_To(const Coord3D *velocity)
{
    if (velocity != nullptr) {
        m_vel.Add(velocity);
    }
}

void PhysicsBehavior::Scrub_Velocity_Z(float desired_velocity)
{
    if (GameMath::Fabs(desired_velocity) < 0.001f) {
        m_vel.z = 0.0f;
    } else {
        if (desired_velocity < 0.0f && m_vel.z < desired_velocity || desired_velocity > 0.0f && m_vel.z > desired_velocity) {
            m_vel.z = desired_velocity;
        }
    }

    m_velMag = -1.0f;
}

void PhysicsBehavior::Reset_Dynamic_Physics()
{
    m_accel.Zero();
    m_prevAccel.Zero();
    m_vel.Zero();
    m_velMag = 0.0f;
    m_turning = TURN_NONE;
    m_yawRate = 0.0f;
    m_rollRate = 0.0f;
    m_pitchRate = 0.0f;
    Set_Flag(MOVING, false);

    captainslog_dbgassert(!Get_Flag(UPDATING), "hmm, should not happen, may not work");
    Set_Wake_Frame(Get_Object(), Calc_Sleep_Time());
}

void PhysicsBehavior::Transfer_Velocity_To(PhysicsBehavior *that) const
{
    if (that != nullptr) {
        that->m_vel.Add(&m_vel);
    }

    that->m_velMag = -1.0f;
}

void PhysicsBehavior::Apply_Random_Rotation()
{
    if (!Get_Flag(STICK_TO_GROUND)) {
        Set_Allow_Bouncing(true);
        m_yawRate = Get_Logic_Random_Value(-1, 1) * Get_Physics_Behavior_Module_Data()->m_shockMaxYaw;
        m_pitchRate = Get_Logic_Random_Value(-1, 1) * Get_Physics_Behavior_Module_Data()->m_shockMaxPitch;
        m_rollRate = Get_Logic_Random_Value(-1, 1) * Get_Physics_Behavior_Module_Data()->m_shockMaxRoll;

        if (!Get_Flag(UPDATING)) {
            Set_Wake_Frame(Get_Object(), UPDATE_SLEEP_TIME_MIN);
        }
    }
}

void PhysicsBehavior::Apply_Shock(const Coord3D *shock)
{
    Coord3D c = *shock;
    c.Scale(1.0f - std::clamp((float)Get_Physics_Behavior_Module_Data()->m_shockResitance, 0.0f, 1.0f));
    Apply_Force(&c);
}

const AudioEventRTS *PhysicsBehavior::Get_Bounce_Sound()
{
    if (m_bounceSound != nullptr) {
        return &m_bounceSound->m_event;
    } else {
        return g_theAudio->Get_Empty_Audio_Event();
    }
}

void PhysicsBehavior::Set_Ignore_Collisions_With(const Object *obj)
{
    if (obj != nullptr) {
        m_ignoreCollisionsWith = obj->Get_ID();
    } else {
        m_ignoreCollisionsWith = INVALID_OBJECT_ID;
    }
}

void PhysicsBehavior::Set_Pitch_Rate(float rate)
{
    m_pitchRate = rate;

    if (m_pitchRate == 0.0f && m_rollRate == 0.0f && m_yawRate == 0.0f) {
        Set_Flag(MOVING, false);
    } else {
        Set_Flag(MOVING, true);
    }
}

void PhysicsBehavior::Set_Roll_Rate(float rate)
{
    m_rollRate = rate;

    if (m_pitchRate == 0.0f && m_rollRate == 0.0f && m_yawRate == 0.0f) {
        Set_Flag(MOVING, false);
    } else {
        Set_Flag(MOVING, true);
    }
}

void PhysicsBehavior::Set_Yaw_Rate(float rate)
{
    m_yawRate = rate;

    if (m_pitchRate == 0.0f && m_rollRate == 0.0f && m_yawRate == 0.0f) {
        Set_Flag(MOVING, false);
    } else {
        Set_Flag(MOVING, true);
    }
}

void PhysicsBehavior::Set_Bounce_Sound(const AudioEventRTS *sound)
{
    if (sound != nullptr) {
        if (m_bounceSound == nullptr) {
            m_bounceSound = new DynamicAudioEventRTS();
        }

        m_bounceSound->m_event = *sound;
    } else if (m_bounceSound != nullptr) {
        m_bounceSound->Delete_Instance();
        m_bounceSound = nullptr;
    }
}
