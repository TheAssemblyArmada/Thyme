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
#include "coord.h"
#include "ini.h"
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
    void Calc_Bounding_Stuff();
    void Set_Max_Height_Above_Position(float max_height);
    float Get_Max_Height_Above_Position();
    float Get_Max_Height_Below_Position();
    float Get_ZDelta_To_Center_Position();
    void Get_Center_Position(const Coord3D &pos, Coord3D &result);
    float Get_Footprint_Area();
    void Expand_Footprint(float expand_by);
    float Is_Intersected_By_Line_Segment(Coord3D &loc, Coord3D &from, Coord3D &to);

    // TODO none virtual functions.

    static void Parse_Geometry_MinorRadius(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_MajorRadius(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_Height(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_IsSmall(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_Type(INI *ini, void *formal, void *store, void *user_data);
    
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
