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
#include "thingtemplate.h"
#include "gamemath.h"

using GameMath::Cos;
using GameMath::Sin;
using GameMath::Inv_Sqrt;

Thing::Thing(const ThingTemplate *thing) : m_template(nullptr)
{
    if (thing != nullptr) {
        m_transform.Make_Identity();
        m_cachedPos = {0.0f, 0.0f, 0.0f};
        m_cachedAngle = 0.0f;
        m_cachedDirVector = {0.0f, 0.0f, 0.0f};
        m_cachedAltitudeAboveTerrain = 0.0f;
        m_cachedAltitudeAboveTerrainOrWater = 0.0f;
        m_cacheFlags = 0;
    }
}

ThingTemplate *Thing::Get_Template()
{
    // TODO Should probably do dynamic, but not sure it would interpret ThingTemplates done by original game correctly.
    return reinterpret_cast<ThingTemplate *>(m_template->Get_Final_Override());
}

const Coord3D &Thing::Get_Unit_Dir_Vector2D()
{
    if (!(m_cacheFlags & CACHED_DIRECTION2D)) {
        m_cachedDirVector.x = Cos(m_cachedAngle);
        m_cachedDirVector.y = Sin(m_cachedAngle);
        m_cachedDirVector.z = 0.0f;
        m_cacheFlags |= CACHED_DIRECTION2D;
    }

    return m_cachedDirVector;
}

void Thing::Get_Unit_Dir_Vector2D(Coord3D &dst)
{
    dst = Get_Unit_Dir_Vector2D();
}

void Thing::Get_Unit_Dir_Vector3D(Coord3D &dst)
{
    float square_sum = float(m_transform[0].X * m_transform[0].X) + float(m_transform[1].X * m_transform[1].X) + float(m_transform[2].X * m_transform[2].X);

    if (square_sum == 0.0f) {
        dst.x = m_transform[0].X;
        dst.y = m_transform[1].X;
        dst.z = m_transform[2].X;
    } else {
        float inv_sum = Inv_Sqrt(square_sum);
        dst.x = inv_sum * m_transform[0].X;
        dst.y = inv_sum * m_transform[1].X;
        dst.z = inv_sum * m_transform[2].X;
    }
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
