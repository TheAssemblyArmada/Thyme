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
#include "geometry.h"
#include "gamemath.h"
#include "minmax.h"
#include "xfer.h"

#define GEOMETRY_XFER_VERSION 1

using GameMath::Sqrt;

GeometryInfo::GeometryInfo(GeometryType type, bool small, float height, float major_radius, float minor_radius) :
    m_boundingCircleRadius(0.0f),
    m_boundingSphereRadius(0.0f)
{
    Set(type, small, height, major_radius, minor_radius);
}

void GeometryInfo::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = GEOMETRY_XFER_VERSION;

    xfer->xferVersion(&version, GEOMETRY_XFER_VERSION);
    xfer->xferUser(&m_type, sizeof(m_type));
    xfer->xferBool(&m_isSmall);
    xfer->xferReal(&m_height);
    xfer->xferReal(&m_majorRadius);
    xfer->xferReal(&m_minorRadius);
    xfer->xferReal(&m_boundingCircleRadius);
    xfer->xferReal(&m_boundingSphereRadius);
}

void GeometryInfo::Set(GeometryType type, bool small, float height, float major_radius, float minor_radius)
{
    m_type = type;
    m_isSmall = small;

    switch (type) {
        case GEOMETRY_SPHERE:
            m_height = m_majorRadius = m_minorRadius = major_radius;
            break;
        case GEOMETRY_CYLINDER:
            m_height = height;
            m_majorRadius = m_minorRadius = major_radius;
            break;
        case GEOMETRY_BOX:
            m_height = height;
            m_majorRadius = major_radius;
            m_minorRadius = minor_radius;
            break;
        default:
            break;
    }

    Calc_Bounding_Box_Stuff();
}

void GeometryInfo::Calc_Bounding_Box_Stuff()
{
    switch (m_type) {
        case GEOMETRY_SPHERE:
            m_boundingCircleRadius = m_boundingSphereRadius = m_majorRadius;
            break;
        case GEOMETRY_CYLINDER:
            m_boundingCircleRadius = m_majorRadius;
            m_boundingSphereRadius = Max(m_height * 0.5f, m_majorRadius);
            break;
        case GEOMETRY_BOX:
            m_boundingCircleRadius =
                Sqrt(float(float(m_minorRadius * m_minorRadius) + float(m_majorRadius * m_majorRadius)));
            m_boundingSphereRadius = Sqrt(float(float(m_minorRadius * m_minorRadius) + float(m_majorRadius * m_majorRadius)
                + float(float(m_height * 0.5f) * float(m_height * 0.5f))));
            break;
        default:
            break;
    }
}
