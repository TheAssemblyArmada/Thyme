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
#include "audiomanager.h"
#include "damage.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "gamelogic.h"
#include "globaldata.h"
#include "opencontain.h"
#include "terrainlogic.h"

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
    m_velMag(0.0f),
    m_yawRate(0.0f),
    m_rollRate(0.0f),
    m_pitchRate(0.0f),
    m_mass(Get_Physics_Behavior_Module_Data()->m_mass),
    m_motiveForceApplied(0),
    m_flags(0),
    m_extraBounciness(0.0f),
    m_extraFriction(0.0f),
    m_currentOverlap(OBJECT_UNK),
    m_previousOverlap(OBJECT_UNK),
    m_collided(OBJECT_UNK),
    m_ignoreCollisionsWith(OBJECT_UNK),
    m_projectileUpdateInterface(nullptr),
    m_bounceSound(nullptr)
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

bool Is_Low_Velocity(Coord3D &vel)
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

        if (gm_isnanf(tm.Get_X_Translation()) || gm_isnanf(tm.Get_Y_Translation()) || gm_isnanf(tm.Get_Z_Translation())) {
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
            tm.Rotate_Z(roll);
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
    m_currentOverlap = OBJECT_UNK;

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
#ifdef GAME_DLL
    Call_Method<void, CollideModuleInterface, Object *, Coord3D const *, Coord3D const *>(
        PICK_ADDRESS(0x005CE0A0, 0x007E6C94), this, other, loc, normal);
#endif
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

void PhysicsBehavior::Set_Angles(float yaw, float pitch, float roll)
{
    const Coord3D *pos = Get_Object()->Get_Position();
    Matrix3D tm;
    tm.Make_Identity();
    tm.Translate(pos->x, pos->y, pos->z);
    tm.Pre_Rotate_X(-roll);
    tm.Pre_Rotate_Y(pitch);
    tm.Pre_Rotate_Z(yaw);
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
    captainslog_dbgassert(
        !gm_isnanf(force->x) && !gm_isnanf(force->y) && !gm_isnanf(force->z), "PhysicsBehavior::applyForce force NAN!");

    if (!gm_isnanf(force->x) && !gm_isnanf(force->y) && !gm_isnanf(force->z)) {
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
#ifdef GAME_DLL
    return Call_Method<bool, PhysicsBehavior, float, float, float, Coord3D *>(
        PICK_ADDRESS(0x005CC730, 0x007E55AF), this, oldz, newz, groundz, bounce_force);
#else
    return false;
#endif
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
#ifdef GAME_DLL
    Call_Method<void, PhysicsBehavior>(PICK_ADDRESS(0x005CC440, 0x007E5370), this);
#endif
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
