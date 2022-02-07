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
#include "gamelogic.h"
#include "updatemodule.h"

class DynamicAudioEventRTS;

enum PhysicsTurningType
{
    TURN_NEGATIVE = -1,
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
    const Coord3D &Get_Prev_Accel() const { return m_prevAccel; }
    const Coord3D &Get_Velocity() const { return m_vel; }
    bool Is_Motive() const { return g_theGameLogic->Get_Frame() < (unsigned int)m_motiveForceApplied; }
    ObjectID Get_Current_Overlap() const { return m_currentOverlap; }
    ObjectID Get_Previous_Overlap() const { return m_previousOverlap; }

    float Get_Velocity_Magnitude() const
    {
        if (m_velMag == -1.0f) {
            m_velMag = GameMath::Sqrt(GameMath::Square(m_vel.x) + GameMath::Square(m_vel.y) + GameMath::Square(m_vel.z));
        }

        return m_velMag;
    }

    // TODO investigate doesn't account for diagonal movement
    float Get_Forward_Speed_2D() const
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
    float Get_Forward_Speed_3D() const
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
    mutable float m_velMag;
    bool m_canBounce; // not 100% identified yet
};
