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
#include "randomvalue.h"
#include "xfer.h"

#define GEOMETRY_XFER_VERSION 1

using GameMath::Sqrt;
using GameMath::Square;
using GameMath::Atan2;
using GameMath::Cos;
using GameMath::Sin;

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

float GeometryInfo::Get_Max_Height_Above_Position() const
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

float GeometryInfo::Get_Max_Height_Below_Position() const
{
    if (m_type == GEOMETRY_SPHERE) {
        return m_majorRadius;
    }

    return 0.0f;
}

float GeometryInfo::Get_ZDelta_To_Center_Position() const
{
    if (m_type != GEOMETRY_SPHERE) {
        return float(GAMEMATH_TIGHT_CORNER_RADIUS * m_height);
    }

    return 0.0f;
}

void GeometryInfo::Get_Center_Position(const Coord3D &pos, Coord3D &result) const
{
    result = pos;
    result.z = Get_ZDelta_To_Center_Position() + result.z;
}

float GeometryInfo::Get_Footprint_Area() const
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

void GeometryInfo::Calc_Pitches(const Coord3D coord1, const GeometryInfo &info, const Coord3D coord2, float &f1, float &f2) const
{
    float z = Get_ZDelta_To_Center_Position() + coord1.z;
    float xy = Sqrt(Square(coord2.y - coord1.y) + Square(coord2.x - coord1.x));
    f2 = Atan2(xy, float(Get_Max_Height_Above_Position() + coord2.z) - z);
    f1 = Atan2(xy, float(coord2.z - info.Get_Max_Height_Below_Position()) - z);
}

void GeometryInfo::Get_2D_Bounds(const Coord3D &pos, float angle, Region2D &region) const
{
    switch (m_type) {
        case GEOMETRY_SPHERE: // Fallthrough
        case GEOMETRY_CYLINDER:
            region.lo.x = pos.x - m_majorRadius;
            region.lo.y = pos.y - m_majorRadius;
            region.hi.x = pos.x + m_majorRadius;
            region.hi.y = pos.y + m_majorRadius;
            break;
        case GEOMETRY_BOX:
        {
            float sin_theta = Sin(angle);
            float cos_theta = Cos(angle);
            float adj_major = cos_theta * m_majorRadius;
            float adj_minor = cos_theta * m_minorRadius;
            float opp_major = sin_theta * m_majorRadius;
            float opp_minor = sin_theta * m_minorRadius;
            region.lo.x = region.hi.x = float(pos.x - adj_major) - opp_minor;
            region.lo.y = region.hi.y = float(pos.y + adj_minor) - opp_major;

            float tmp_x = float(adj_major + pos.x) - opp_minor;
            float tmp_y = float(opp_major + pos.y) + adj_minor;
            region.lo.x = Min(region.lo.x, tmp_x);
            region.lo.y = Min(region.lo.y, tmp_y);
            region.hi.x = Max(region.hi.x, tmp_x);
            region.hi.y = Max(region.hi.y, tmp_y);

            tmp_x = float(opp_minor + pos.x) + adj_major;
            tmp_y = float(pos.y - adj_minor) + opp_major;
            region.lo.x = Min(region.lo.x, tmp_x);
            region.lo.y = Min(region.lo.y, tmp_y);
            region.hi.x = Max(region.hi.x, tmp_x);
            region.hi.y = Max(region.hi.y, tmp_y);

            tmp_x = float(opp_minor + pos.x) - adj_major;
            tmp_y = float(pos.y - adj_minor) - opp_major;
            region.lo.x = Min(region.lo.x, tmp_x);
            region.lo.y = Min(region.lo.y, tmp_y);
            region.hi.x = Max(region.hi.x, tmp_x);
            region.hi.y = Max(region.hi.y, tmp_y);
        }
            break;
        default:
            break;
    }
}

void GeometryInfo::Clip_Point_To_Footprint(const Coord3D &pos, Coord3D &point) const
{
    switch (m_type) {
        case GEOMETRY_SPHERE: // Fallthrough
        case GEOMETRY_CYLINDER:
        {
            float x_diff = point.x - pos.x;
            float y_diff = point.y - pos.y;
            float hyp = Sqrt(Square(y_diff) + Square(x_diff));

            if (hyp > m_majorRadius) {
                point.x = float(float(m_majorRadius / hyp) * x_diff) + pos.x;
                point.x = float(float(m_majorRadius / hyp) * y_diff) + pos.y;
            }
        }
            break;
        case GEOMETRY_BOX:
            point.x = Clamp(point.x, pos.x - m_majorRadius, pos.x + m_majorRadius);
            point.y = Clamp(point.y, pos.y - m_minorRadius, pos.y + m_minorRadius);

            break;
        default:
            break;
    }
}

bool GeometryInfo::Is_Point_In_Footprint(const Coord3D &pos, const Coord3D &point) const
{
    switch (m_type) {
        case GEOMETRY_SPHERE: // Fallthrough
        case GEOMETRY_CYLINDER:
            return Sqrt(Square(point.y - pos.y) + Square(point.x - pos.x)) <= m_majorRadius;
        case GEOMETRY_BOX:
            return pos.x - m_majorRadius <= point.x && point.x <= pos.x + m_majorRadius && pos.y - m_minorRadius <= point.y
                && point.y <= m_minorRadius + pos.y;
        default:
            break;
    }

    return false;
}

void GeometryInfo::Make_Random_Offset_In_Footprint(Coord3D &offset) const
{
    offset.z = 0.0f;

    switch (m_type) {
        case GEOMETRY_SPHERE: // Fallthrough
        case GEOMETRY_CYLINDER:
            do {
                offset.x = Get_Logic_Random_Value_Real(-m_majorRadius, m_majorRadius);
                offset.y = Get_Logic_Random_Value_Real(-m_majorRadius, m_majorRadius);
            } while (Square(offset.y) + Square(offset.x) > Square(m_majorRadius));

            break;
        case GEOMETRY_BOX:
            offset.x = Get_Logic_Random_Value_Real(-m_majorRadius, m_majorRadius);
            offset.y = Get_Logic_Random_Value_Real(-m_minorRadius, m_minorRadius);

            break;
        default:
            break;
    }
}

void GeometryInfo::Make_Random_Offset_In_Perimeter(Coord3D &offset) const
{
    switch (m_type) {
        case GEOMETRY_SPHERE: // Fallthrough
        case GEOMETRY_CYLINDER:
            offset.x = 0.0f;
            offset.y = 0.0f;

            break;
        case GEOMETRY_BOX:
            if (Get_Logic_Random_Value_Real(0.0f, 1.0f) >= 0.5f) {
                offset.y = Get_Logic_Random_Value_Real(-m_minorRadius, m_minorRadius);

                if (Get_Logic_Random_Value_Real(0.0f, 1.0f) >= 0.5f) {
                    offset.x = m_majorRadius;
                } else {
                    offset.x = -m_majorRadius;
                }
            } else {
                offset.x = Get_Logic_Random_Value_Real(-m_majorRadius, m_majorRadius);

                if (Get_Logic_Random_Value_Real(0.0f, 1.0f) >= 0.5f) {
                    offset.y = m_minorRadius;
                } else {
                    offset.y = -m_minorRadius;
                }
            }

            offset.z = 0.0f;

            break;
        default:
            break;
    }
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
