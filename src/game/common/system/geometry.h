/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Geometry information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "always.h"
#include "snapshot.h"

enum GeometryType : int32_t
{
    GEOMETRY_SPHERE,
    GEOMETRY_CYLINDER,
    GEOMETRY_BOX,
    GEOMETRY_COUNT,
};

class GeometryInfo : public SnapShot
{
public:
    GeometryInfo(GeometryType type, bool small, float height, float major_radius, float minor_radius);

    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    void Set(GeometryType type, bool small, float height, float major_radius, float minor_radius);
    void Calc_Bounding_Box_Stuff();
    // TODO none virtual functions.

private:
    GeometryType m_type;
    bool m_isSmall;
    float m_height;
    float m_majorRadius;
    float m_minorRadius;
    float m_boundingCircleRadius;
    float m_boundingSphereRadius;
};

#endif // GEOMETRY_H
