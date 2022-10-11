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
#pragma once
#include "always.h"
#include "object.h"
#include "typeoperators.h"
#include "updatemodule.h"

class DynamicAudioEventRTS;

enum PhysicsTurningType
{
    TURN_NEGATIVE = -1,
    TURN_NONE,
    TURN_POSITIVE,
};

class PhysicsBehaviorModuleData : public UpdateModuleData
{
public:
    PhysicsBehaviorModuleData();
    virtual ~PhysicsBehaviorModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    float m_mass;
    int m_shockResitance;
    float m_shockMaxYaw;
    float m_shockMaxPitch;
    float m_shockMaxRoll;
    float m_forwardFriction;
    float m_lateralFriction;
    float m_zFriction;
    float m_aerodynamicCoeff;
    int m_centerOfMassOffset;
    bool m_killWhenRestingOnGround;
    bool m_allowBouncing;
    bool m_allowCollideForce;
    float m_minFallHeightForDamage;
    float m_fallHeightDamageFactor;
    float m_pitchRollYawFactor;
    const WeaponTemplate *m_vehicleCrashesIntoBuildingWeapon;
    const WeaponTemplate *m_vehicleCrashesIntoNonBuildingWeapon;

    friend class PhysicsBehavior;
};

class PhysicsBehavior : public UpdateModule, public CollideModuleInterface
{
    IMPLEMENT_POOL(PhysicsBehavior)

    enum PhysicsFlagsType
    {
        STICK_TO_GROUND = 1,
        ALLOW_BOUNCE = 2,
        APPLY_FRICTION2D_WHEN_AIRBORNE = 4,
        UPDATE_EVER_RUN = 8,
        WAS_AIRBORNE_LAST_FRAME = 16,
        ALLOW_COLLIDE_FORCE = 32,
        IS_AIRBORNE = 64,
        MOVING = 128,
        FLAG_UNK3 = 256,
        FREEFALL = 512,
        UPDATING = 1024,
        STUNNED = 2048,
    };

public:
    PhysicsBehavior(Thing *thing, const ModuleData *module_data);

    virtual ~PhysicsBehavior() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual SleepyUpdatePhase Get_Update_Phase() const override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual CollideModuleInterface *Get_Collide() override;
    virtual UpdateSleepTime Update() override;
    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual void On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal) override;
    virtual bool Would_Like_To_Collide_With(Object const *other) override;
    virtual bool Is_Hijacked_Vehicle_Crate_Collide() override;
    virtual bool Is_Sabotage_Building_Crate_Collide() override;
    virtual bool Is_Car_Bomb_Crate_Collide() override;
    virtual bool Is_Railroad() override;
    virtual bool Is_Salvage_Crate_Collide() override;

    void Add_Overlap(Object *obj);
    void Add_Velocity_To(const Coord3D *velocity);
    void Scrub_Velocity_2D(float desired_velocity);
    void Scrub_Velocity_Z(float desired_velocity);
    bool Check_For_Overlap_Collision(Object *other);
    void Do_Bounce_Sound(Coord3D const &pos);
    bool Handle_Bounce(float oldz, float newz, float groundz, Coord3D *bounce_force);
    void Reset_Dynamic_Physics();
    void Test_Stunned_Unit_For_Destruction();
    void Transfer_Velocity_To(PhysicsBehavior *that) const;
    bool Was_Previously_Overlapped(Object *obj) const;
    UpdateSleepTime Calc_Sleep_Time() const;

    void Apply_Force(const Coord3D *force);
    void Apply_Frictional_Forces();
    void Apply_Gravitational_Forces();
    void Apply_Random_Rotation();
    void Apply_Shock(const Coord3D *shock);
    void Apply_YPR_Damping(float damping);
    void Apply_Motive_Force(const Coord3D *force);

    float Get_Mass() const;
    float Get_Velocity_Magnitude() const;
    float Get_Forward_Speed_2D() const;
    float Get_Forward_Speed_3D() const;
    float Get_Aerodynamic_Friction() const;
    float Get_Forward_Friction() const;
    float Get_Lateral_Friction() const;
    float Get_Z_Friction() const;
    const AudioEventRTS *Get_Bounce_Sound();

    bool Is_Ignoring_Collisions_With(ObjectID id) const;
    bool Is_Currently_Overlapped(Object *obj) const;
    bool Is_Motive() const;

    void Set_Ignore_Collisions_With(const Object *obj);
    void Set_Angles(float yaw, float pitch, float roll);
    void Set_Pitch_Rate(float rate);
    void Set_Roll_Rate(float rate);
    void Set_Yaw_Rate(float rate);
    void Set_Bounce_Sound(const AudioEventRTS *sound);

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

    static int Get_Interface_Mask() { return UpdateModule::Get_Interface_Mask() | MODULEINTERFACE_COLLIDE; }

    void Clear_Acceleration() { m_accel.Zero(); }

    PhysicsTurningType Get_Turning() const { return m_turning; }
    const Coord3D &Get_Prev_Accel() const { return m_prevAccel; }
    const Coord3D &Get_Velocity() const { return m_vel; }
    ObjectID Get_Current_Overlap() const { return m_currentOverlap; }
    ObjectID Get_Previous_Overlap() const { return m_previousOverlap; }
    float Get_Center_Of_Mass_Offset() const { return Get_Physics_Behavior_Module_Data()->m_centerOfMassOffset; }
    ObjectID Get_Collided() const { return m_collided; }

    bool Get_Stunned() const { return Get_Flag(STUNNED); }
    bool Get_Airborne() const { return Get_Flag(IS_AIRBORNE); }
    bool Get_Allow_Collide_Force() const { return Get_Flag(ALLOW_COLLIDE_FORCE); }

    void Set_Turning(PhysicsTurningType turning) { m_turning = turning; }
    void Set_Extra_Friction(float friction) { m_extraFriction = friction; }
    void Set_Extra_Bounciness(float bounciness) { m_extraBounciness = bounciness; }
    void Set_Mass(float mass) { m_mass = mass; }

    void Set_Allow_Airborne_Friction(bool set) { Set_Flag(APPLY_FRICTION2D_WHEN_AIRBORNE, set); }
    void Set_Stick_To_Ground(bool set) { Set_Flag(STICK_TO_GROUND, set); }
    void Set_Airborne(bool airborne) { Set_Flag(IS_AIRBORNE, airborne); }
    void Set_Allow_Bouncing(bool allow) { Set_Flag(ALLOW_BOUNCE, allow); }
    void Set_Allow_Collide_Force(bool allow) { Set_Flag(ALLOW_COLLIDE_FORCE, allow); }
    void Set_Freefall(bool freefall) { Set_Flag(FREEFALL, freefall); }
    void Set_Stunned(bool stunned) { Set_Flag(STUNNED, stunned); }

    const PhysicsBehaviorModuleData *Get_Physics_Behavior_Module_Data() const
    {
        return static_cast<const PhysicsBehaviorModuleData *>(Get_Module_Data());
    }

private:
    bool Get_Flag(PhysicsFlagsType flag) const { return (flag & m_flags) != 0; }

    void Set_Flag(PhysicsFlagsType flag, bool set)
    {
        if (set) {
            m_flags |= flag;
        } else {
            m_flags &= ~flag;
        }
    }

    float m_yawRate;
    float m_rollRate;
    float m_pitchRate;
    DynamicAudioEventRTS *m_bounceSound;
    Coord3D m_accel;
    Coord3D m_prevAccel; // not 100% identified yet
    Coord3D m_vel;
    PhysicsTurningType m_turning;
    ObjectID m_ignoreCollisionsWith;
    int m_flags;
    float m_mass;
    ObjectID m_currentOverlap;
    ObjectID m_previousOverlap;
    ObjectID m_collided; // not 100% identified yet
    unsigned int m_motiveForceApplied;
    float m_extraBounciness; // not 100% identified yet
    float m_extraFriction;
    ProjectileUpdateInterface *m_projectileUpdateInterface; // not 100% identified yet
    mutable float m_velMag;
    bool m_canBounce; // not 100% identified yet
};
