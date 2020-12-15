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

#ifndef THING_H
#define THING_H

#include "always.h"
#include "coord.h"
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
        CACHED_DIRECTION2D = 1 << 0,
    };

protected:
    virtual ~Thing() override {}

public:
    Thing(const ThingTemplate *thing = nullptr);

    // Thing interface virtual functions.
    virtual Object *As_Object_Meth() { return nullptr; }
    virtual Drawable *As_Drawable_Meth() { return nullptr; }
    virtual const Object *As_Object_Meth() const { return nullptr; }
    virtual const Drawable *As_Drawable_Meth() const { return nullptr; }
    virtual void React_To_Transform(const Matrix3D *matrix, const Coord3D *pos, float angle) = 0;

    const ThingTemplate *Get_Template() const;
    const Coord3D &Get_Unit_Dir_Vector2D();
    void Get_Unit_Dir_Vector2D(Coord3D &dst);
    void Get_Unit_Dir_Vector3D(Coord3D &dst);
    void Set_Position_Z(float pos);

private:
    Override<ThingTemplate> m_template;
    Matrix3D m_transform;
    Coord3D m_cachedPos;
    float m_cachedAngle;
    Coord3D m_cachedDirVector;
    float m_cachedAltitudeAboveTerrain;
    float m_cachedAltitudeAboveTerrainOrWater;
    int m_cacheFlags;
};

#ifdef GAME_DLL
#include "hooker.h"
#endif

#endif // THING_H
