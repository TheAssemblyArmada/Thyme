/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Topple Update
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

class ToppleUpdate : public UpdateModule, public CollideModuleInterface
{
    IMPLEMENT_POOL(ToppleUpdate)

public:
    virtual ~ToppleUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual CollideModuleInterface *Get_Collide() override;

    virtual UpdateSleepTime Update() override;

    virtual void On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal) override;
    virtual bool Would_Like_To_Collide_With(Object const *other) override;
    virtual bool Is_Hijacked_Vehicle_Crate_Collide() override;
    virtual bool Is_Sabotage_Building_Crate_Collide() override;
    virtual bool Is_Car_Bomb_Crate_Collide() override;
    virtual bool Is_Railroad() override;
    virtual bool Is_Salvage_Crate_Collide() override;

    bool Is_Able_To_Be_Toppled() const;
    void Apply_Toppling_Force(const Coord3D *topple_direction, float topple_speed, unsigned int options);

private:
    enum ToppleState
    {
        TOPPLE_UPRIGHT,
        TOPPLE_FALLING,
        TOPPLE_DOWN,
    };

    float m_angularVelocity;
    float m_angularAcceleration;
    Coord3D m_toppleDirection;
    ToppleState m_toppleState;
    float m_angularAccumulation;
    float m_angleDeltaX;
    int m_numAngleDeltaX;
    bool m_doBounceFX;
    unsigned int m_options;
    ObjectID m_stumpID;
};
