/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
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
#include "gamedebug.h"
#include "gamemath.h"
#include "minmax.h"
#include "xfer.h"

#define GEOMETRY_XFER_VERSION 1

using GameMath::Sqrt;
using GameMath::Square;

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

    Calc_Bounding_Stuff();
}

void GeometryInfo::Calc_Bounding_Stuff()
{
    switch (m_type) {
        case GEOMETRY_SPHERE:
            m_boundingCircleRadius = m_boundingSphereRadius = m_majorRadius;
            break;
        case GEOMETRY_CYLINDER:
            m_boundingCircleRadius = m_majorRadius;
            m_boundingSphereRadius = Max(m_height * GAMEMATH_TIGHT_CORNER_RADIUS, m_majorRadius);
            break;
        case GEOMETRY_BOX:
            m_boundingCircleRadius = Sqrt(float(Square(m_minorRadius)) + float(Square(m_majorRadius)));
            m_boundingSphereRadius = Sqrt(float(float(Square(m_minorRadius)) + float(Square(m_minorRadius)))
                + float(float(m_height * GAMEMATH_TIGHT_CORNER_RADIUS) * float(m_height * GAMEMATH_TIGHT_CORNER_RADIUS)));
            break;
        default:
            break;
    }
}

void GeometryInfo::Set_Max_Height_Above_Position(float max_height)
{
    switch (m_type) {
        case GEOMETRY_SPHERE:
            m_majorRadius = max_height;
            break;
        case GEOMETRY_CYLINDER: // Fallthrough
        case GEOMETRY_BOX:
            m_height = max_height;
            break;
        default:
            DEBUG_LOG("GeometryInfo::Set_Max_Height_Above_Position - should never get here\n");
            break;
    }

    Calc_Bounding_Stuff();
}

float GeometryInfo::Get_Max_Height_Above_Position()
{
    switch (m_type) {
        case GEOMETRY_SPHERE:
            return m_majorRadius;
        case GEOMETRY_CYLINDER: // Fallthrough
        case GEOMETRY_BOX:
            return m_height;
        default:
            DEBUG_LOG("GeometryInfo::Get_Max_Height_Above_Position - should never get here\n");
            break;
    }

    return 0.0f;
}

float GeometryInfo::Get_Max_Height_Below_Position()
{
    if (m_type == GEOMETRY_SPHERE) {
        return m_majorRadius;
    }

    return 0.0f;
}

float GeometryInfo::Get_ZDelta_To_Center_Position()
{
    if (m_type != GEOMETRY_SPHERE) {
        return float(GAMEMATH_TIGHT_CORNER_RADIUS * m_height);
    }

    return 0.0f;
}

void GeometryInfo::Get_Center_Position(const Coord3D &pos, Coord3D &result)
{
    result = pos;
    result.z = Get_ZDelta_To_Center_Position() + result.z;
}

float GeometryInfo::Get_Footprint_Area()
{
    switch (m_type) {
        case GEOMETRY_CYLINDER:
            return float(Square(m_boundingCircleRadius) * GAMEMATH_PI);
        case GEOMETRY_BOX:
            return float(4.0f * float(m_majorRadius * m_minorRadius));
        case GEOMETRY_SPHERE: // Fallthrough
        default:
            DEBUG_LOG("GeometryInfo::Get_Footprint_Area - should never get here\n");
            break;
    }

    return 0.0f;
}

void GeometryInfo::Expand_Footprint(float expand_by)
{
    m_majorRadius += expand_by;
    m_minorRadius += expand_by;
    Calc_Bounding_Stuff();
}

float GeometryInfo::Is_Intersected_By_Line_Segment(Coord3D &loc, Coord3D &from, Coord3D &to)
{
    // TODO Unused in game or inlined somewhere? From symbols.
    return 0.0f;
}

void GeometryInfo::Parse_Geometry_MinorRadius(INI *ini, void *formal, void *store, void *user_data)
{
    static_cast<GeometryInfo *>(store)->m_minorRadius = INI::Scan_Real(ini->Get_Next_Token());
    static_cast<GeometryInfo *>(store)->Calc_Bounding_Stuff();
}

void GeometryInfo::Parse_Geometry_MajorRadius(INI *ini, void *formal, void *store, void *user_data)
{
    static_cast<GeometryInfo *>(store)->m_majorRadius = INI::Scan_Real(ini->Get_Next_Token());
    static_cast<GeometryInfo *>(store)->Calc_Bounding_Stuff();
}

void GeometryInfo::Parse_Geometry_Height(INI *ini, void *formal, void *store, void *user_data)
{
    static_cast<GeometryInfo *>(store)->m_height = INI::Scan_Real(ini->Get_Next_Token());
    static_cast<GeometryInfo *>(store)->Calc_Bounding_Stuff();
}

void GeometryInfo::Parse_Geometry_IsSmall(INI *ini, void *formal, void *store, void *user_data)
{
    static_cast<GeometryInfo *>(store)->m_isSmall = INI::Scan_Bool(ini->Get_Next_Token());
    static_cast<GeometryInfo *>(store)->Calc_Bounding_Stuff();
}

void GeometryInfo::Parse_Geometry_Type(INI *ini, void *formal, void *store, void *user_data)
{
    static const char *_geometry_names[] = { "SPHERE", "CYLINDER", "BOX", nullptr };

    static_cast<GeometryInfo *>(store)->m_type = GeometryType(INI::Scan_IndexList(ini->Get_Next_Token(), _geometry_names));
    static_cast<GeometryInfo *>(store)->Calc_Bounding_Stuff();
}
