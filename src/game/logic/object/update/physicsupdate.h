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
#include "updatemodule.h"

class DynamicAudioEventRTS;

enum PhysicsTurningType
{
    TURN_NEGATIVE = 0xFF,
    TURN_NONE,
    TURN_POSITIVE,
};

class PhysicsBehavior : public UpdateModule, public CollideModuleInterface
{
    IMPLEMENT_POOL(PhysicsBehavior)

    enum PhysicsFlagsType
    {
        STICK_TO_GROUND,
        ALLOW_BOUNCE,
        APPLY_FRICTION2D_WHEN_AIRBORNE,
        UPDATE_EVER_RUN,
        WAS_AIRBORNE_LAST_FRAME,
        ALLOW_COLLIDE_FORCE,
    };

public:
    virtual ~PhysicsBehavior() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual SleepyUpdatePhase Get_Update_Phase() const override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual CollideModuleInterface *Get_Collide() override;
    virtual UpdateModuleInterface *Get_Update() override;
    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual void On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal) override;
    virtual bool Would_Like_To_Collide_With(Object const *other) override;
    virtual bool Is_Hijacked_Vehicle_Crate_Collide() override;
    virtual bool Is_Sabotage_Building_Crate_Collide() override;
    virtual bool Is_Car_Bomb_Crate_Collide() override;
    virtual bool Is_Railroad() override;
    virtual bool Is_Salvage_Crate_Collide() override;
    PhysicsTurningType Get_Turning() const { return m_turning; }
    static int Get_Interface_Mask() { return UpdateModule::Get_Interface_Mask() | MODULEINTERFACE_COLLIDE; }

private:
    float m_yawRate;
    float m_rollRate;
    float m_pitchRate;
    DynamicAudioEventRTS *m_bounceSound;
    Coord3D m_accel;
    Coord3D m_prevAccel; // not 100% identified yet
    Coord3D m_vel;
    PhysicsTurningType m_turning;
    int m_ignoreCollisionsWith;
    PhysicsFlagsType m_flags;
    float m_mass;
    ObjectID m_currentOverlap;
    ObjectID m_previousOverlap;
    ObjectID m_collided; // not 100% identified yet
    int m_motiveForceApplied;
    float m_extraBounciness; // not 100% identified yet
    float m_extraFriction;
    ProjectileUpdateInterface *m_projectileUpdateInterface; // not 100% identified yet
    float m_velMag;
    bool m_canBounce; // not 100% identified yet
};
