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
#include "opencontain.h"

void PhysicsBehavior::Apply_Motive_Force(const Coord3D *force)
{
#ifdef GAME_DLL
    Call_Method<void, PhysicsBehavior, const Coord3D *>(PICK_ADDRESS(0x005CC300, 0x007E523E), this, force);
#endif
}

void PhysicsBehavior::Scrub_Velocity_2D(float desired_velocity)
{
    if (desired_velocity >= 0.001f) {
        float velocity = GameMath::Sqrt(m_vel.x * m_vel.x + m_vel.y * m_vel.y);

        if (desired_velocity > velocity) {
            return;
        }

        float velocity_change = desired_velocity / velocity;
        m_vel.x = velocity_change * m_vel.x;
        m_vel.y = velocity_change * m_vel.y;
    } else {
        m_vel.x = 0.0f;
        m_vel.y = 0.0f;
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
