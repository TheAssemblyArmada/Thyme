/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for "things".
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
#include "bitflags.h"
#include "coord.h"
#include "kindof.h"
#include "matrix3d.h"
#include "mempoolobj.h"
#include "thingtemplate.h"

class Object;
class Drawable;

class Thing : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(Thing)

    enum
    {
        VALID_DIRVECTOR = 1 << 0,
        VALID_ALTITUDE_TERRAIN = 1 << 1,
        VALID_ALTITUDE_SEALEVEL = 1 << 2,
    };

protected:
    virtual ~Thing() override {}

public:
    Thing(const ThingTemplate *thing_template);

    // Thing interface virtual functions.
    virtual float Calculate_Height_Above_Terrain() const;
    virtual Object *As_Object_Meth() { return nullptr; }
    virtual Drawable *As_Drawable_Meth() { return nullptr; }
    virtual const Object *As_Object_Meth() const { return nullptr; }
    virtual const Drawable *As_Drawable_Meth() const { return nullptr; }
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) = 0;

    const ThingTemplate *Get_Template() const;
    const Coord3D *Get_Unit_Dir_Vector2D() const;
    void Get_Unit_Dir_Vector2D(Coord3D &dst) const;
    void Get_Unit_Dir_Vector3D(Coord3D &dst) const;

    void Set_Position_Z(float z);
    void Set_Position(const Coord3D *pos);
    void Set_Orientation(float angle);
    void Set_Transform_Matrix(const Matrix3D *mx);

    bool Is_KindOf(KindOfType t) const;
    bool Is_KindOf_Multi(const BitFlags<KINDOF_COUNT> &must_be_set, const BitFlags<KINDOF_COUNT> &must_be_clear) const;
    bool Is_Any_KindOf(const BitFlags<KINDOF_COUNT> &any_kind_of) const;
    bool Is_Significantly_Above_Terrain() const;

    float Get_Height_Above_Terrain() const;
    float Get_Height_Above_Terrain_Or_Water() const;

    void Convert_Bone_Pos_To_World_Pos(
        const Coord3D *bone_pos, const Matrix3D *bone_transform, Coord3D *world_pos, Matrix3D *world_transform) const;
    void Transform_Point(const Coord3D *in, Coord3D *out);

    const Matrix3D *Get_Transform_Matrix() const { return &m_transform; }
    const Coord3D *Get_Position() const { return &m_cachedPos; }
    float Get_Orientation() const { return m_cachedAngle; }

    bool Is_Above_Terrain() const { return Get_Height_Above_Terrain() > 0.0f; }
    bool Is_Above_Terrain_Or_Water() const { return Get_Height_Above_Terrain_Or_Water() > 0.0f; }

private:
    Override<ThingTemplate> m_template;
#ifdef GAME_DEBUG_STRUCTS
    Utf8String m_templateName;
#endif
    Matrix3D m_transform;
    Coord3D m_cachedPos;
    float m_cachedAngle;
    mutable Coord3D m_cachedDirVector;
    mutable float m_cachedAltitudeAboveTerrain;
    mutable float m_cachedAltitudeAboveTerrainOrWater;
    mutable int m_cacheFlags;
};
