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
    GeometryInfo(GeometryType type, bool little, float height, float major_radius, float minor_radius);

    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    void Set(GeometryType type, bool little, float height, float major_radius, float minor_radius);
    void Calc_Bounding_Stuff();
    void Set_Max_Height_Above_Position(float max_height);
    float Get_Max_Height_Above_Position() const;
    float Get_Max_Height_Below_Position() const;
    float Get_ZDelta_To_Center_Position() const;
    void Get_Center_Position(const Coord3D &pos, Coord3D &result) const;
    float Get_Footprint_Area() const;
    void Expand_Footprint(float expand_by);
    float Is_Intersected_By_Line_Segment(Coord3D &loc, Coord3D &from, Coord3D &to);
    void Calc_Pitches(const Coord3D &coord1, const GeometryInfo &info, const Coord3D &coord2, float &f1, float &f2) const;
    void Get_2D_Bounds(const Coord3D &pos, float angle, Region2D &region) const;
    void Clip_Point_To_Footprint(const Coord3D &pos, Coord3D &point) const;
    bool Is_Point_In_Footprint(const Coord3D &pos, const Coord3D &point) const;
    void Make_Random_Offset_In_Footprint(Coord3D &offset) const;
    void Make_Random_Offset_In_Perimeter(Coord3D &offset) const;
    // TODO none virtual functions.

    static void Parse_Geometry_MinorRadius(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_MajorRadius(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_Height(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_IsSmall(INI *ini, void *formal, void *store, void *user_data);
    static void Parse_Geometry_Type(INI *ini, void *formal, void *store, void *user_data);
    
#ifndef THYME_STANDALONE
    static void Hook_Me();
    void Hook_Xfer(Xfer *xfer);
#endif

private:
    GeometryType m_type;
    bool m_isSmall;
    float m_height;
    float m_majorRadius;
    float m_minorRadius;
    float m_boundingCircleRadius;
    float m_boundingSphereRadius;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void GeometryInfo::Hook_Me()
{
    Hook_Method(0x005CFAF0, &Hook_Xfer);
    Hook_Method(0x005CF220, &Set);
    Hook_Method(0x005CFA60, &Calc_Bounding_Stuff);
    Hook_Method(0x005CF350, &Set_Max_Height_Above_Position);
    Hook_Method(0x005CF380, &Get_Max_Height_Above_Position);
    Hook_Method(0x005CF3A0, &Get_Max_Height_Below_Position);
    Hook_Method(0x005CF3C0, &Get_ZDelta_To_Center_Position);
    Hook_Method(0x005CF3E0, &Get_Center_Position);
    Hook_Method(0x005CFA20, &Get_Footprint_Area);
    Hook_Method(0x005CF430, &Expand_Footprint);
    Hook_Method(0x005CF290, &Calc_Pitches);
    Hook_Method(0x005CF450, &Get_2D_Bounds);
    Hook_Method(0x005CF620, &Clip_Point_To_Footprint);
    Hook_Method(0x005CF720, &Is_Point_In_Footprint);
    Hook_Method(0x005CF7E0, &Make_Random_Offset_In_Footprint);
    Hook_Method(0x005CF8E0, &Make_Random_Offset_In_Perimeter);
    Hook_Function(0x005CF200, Parse_Geometry_MinorRadius);
    Hook_Function(0x005CF1E0, Parse_Geometry_MajorRadius);
    Hook_Function(0x005CF1C0, Parse_Geometry_Height);
    Hook_Function(0x005CF1A0, Parse_Geometry_IsSmall);
    Hook_Function(0x005CF170, Parse_Geometry_Type);
}

#endif

#endif // GEOMETRY_H
