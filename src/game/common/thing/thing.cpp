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
#include "thing.h"
#include "gamemath.h"
#include "thingtemplate.h"

using GameMath::Cos;
using GameMath::Inv_Sqrt;
using GameMath::Sin;

Thing::Thing(const ThingTemplate *templ) : m_template()
{
    if (templ != nullptr) {
        m_template = const_cast<ThingTemplate *>(templ);
#ifdef GAME_DEBUG_STRUCTS
        m_templateName = templ->Get_Name();
#endif
        m_transform.Make_Identity();
        m_cachedPos = { 0.0f, 0.0f, 0.0f };
        m_cachedAngle = 0.0f;
        m_cachedDirVector = { 0.0f, 0.0f, 0.0f };
        m_cachedAltitudeAboveTerrain = 0.0f;
        m_cachedAltitudeAboveTerrainOrWater = 0.0f;
        m_cacheFlags = 0;
    } else {
        captainslog_debug("no template");
    }
}

const ThingTemplate *Thing::Get_Template() const
{
    return *m_template;
}

const Coord3D *Thing::Get_Unit_Dir_Vector2D() const
{
    if (!(m_cacheFlags & VALID_DIRVECTOR)) {
        m_cachedDirVector.x = Cos(m_cachedAngle);
        m_cachedDirVector.y = Sin(m_cachedAngle);
        m_cachedDirVector.z = 0.0f;
        m_cacheFlags |= VALID_DIRVECTOR;
    }

    return &m_cachedDirVector;
}

void Thing::Get_Unit_Dir_Vector2D(Coord3D &dst) const
{
    dst = *Get_Unit_Dir_Vector2D();
}

void Thing::Get_Unit_Dir_Vector3D(Coord3D &dst) const
{
    Vector3 d = m_transform.Get_X_Vector();
    d.Normalize();

    dst.x = d.X;
    dst.y = d.Y;
    dst.z = d.Z;
}

void Thing::Set_Position_Z(float pos)
{
    // TODO Requires TerrainLogic
#ifdef GAME_DLL
    Call_Method<void, Thing, float>(0x005435C0, this, pos);
#elif 0
    if (Get_Template()->Is_KindOf(KINDOF_STICK_TO_TERRAIN_SLOPE)) {

    } else {
    }
#endif
}

float Thing::Calculate_Height_Above_Terrain() const
{
    // TODO Requires TerrainLogic
#ifdef GAME_DLL
    return Call_Method<float, const Thing>(0x00543EB0, this);
#else
    return 0;
#endif
}
