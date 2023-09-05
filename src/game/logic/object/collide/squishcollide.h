/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Squish Module
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
#include "collidemodule.h"

class SquishCollide : public CollideModule
{
    IMPLEMENT_POOL(SquishCollide);

    ~SquishCollide() override;
    NameKeyType Get_Module_Name_Key() const override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    void On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal) override;
};
