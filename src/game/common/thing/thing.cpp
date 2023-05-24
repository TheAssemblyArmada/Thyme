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
#include "globaldata.h"
#include "terrainlogic.h"
#include "thingtemplate.h"

using GameMath::Cos;
using GameMath::Inv_Sqrt;
using GameMath::Sin;

Thing::Thing(const ThingTemplate *thing_template)
{
    if (thing_template != nullptr) {
        m_template = const_cast<ThingTemplate *>(thing_template);
#ifdef GAME_DEBUG_STRUCTS
        m_templateName = thing_template->Get_Name();
#endif
    } else {
        captainslog_debug("no template");
        m_template = nullptr;
    }
    // #BUGFIX Initialize all members always
    m_transform.Make_Identity();
    m_cachedPos.Zero();
    m_cachedAngle = 0.0f;
    m_cachedDirVector.Zero();
    m_cachedAltitudeAboveTerrain = 0.0f;
    m_cachedAltitudeAboveTerrainOrWater = 0.0f;
    m_cacheFlags = 0;
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

void Thing::Set_Position_Z(float z)
{
    if (Get_Template()->Is_KindOf(KINDOF_STICK_TO_TERRAIN_SLOPE)) {
        Matrix3D tm;
        Coord3D pos;
        pos.x = m_cachedPos.x;
        pos.y = m_cachedPos.y;
        pos.z = z;
        g_theTerrainLogic->Align_On_Terrain(Get_Orientation(), pos, true, tm);
        Set_Transform_Matrix(&tm);
    } else {
        float old_angle = m_cachedAngle;
        Coord3D old_pos = m_cachedPos;
        Matrix3D old_tm(m_transform);
        m_transform.Set_Z_Translation(z);
        m_cachedPos.z = z;

        if ((m_cacheFlags & VALID_ALTITUDE_TERRAIN) != 0) {
            m_cachedAltitudeAboveTerrain = z - old_pos.z + m_cachedAltitudeAboveTerrain;
        }

        if ((m_cacheFlags & VALID_ALTITUDE_SEALEVEL) != 0) {
            m_cachedAltitudeAboveTerrainOrWater = z - old_pos.z + m_cachedAltitudeAboveTerrainOrWater;
        }

        React_To_Transform_Change(&old_tm, &old_pos, old_angle);
    }

    captainslog_dbgassert(
        !GameMath::Is_Nan(Get_Position()->x) && !GameMath::Is_Nan(Get_Position()->y) && !GameMath::Is_Nan(Get_Position()->z),
        "Drawable/Object position NAN! \'%s\'",
        m_template->Get_Name().Str());
}

float Thing::Get_Height_Above_Terrain() const
{
    if (!(m_cacheFlags & VALID_ALTITUDE_TERRAIN)) {
        m_cachedAltitudeAboveTerrain = Calculate_Height_Above_Terrain();
        m_cacheFlags |= VALID_ALTITUDE_TERRAIN;
    }

    return m_cachedAltitudeAboveTerrain;
}

float Thing::Calculate_Height_Above_Terrain() const
{
    const Coord3D *pos = Get_Position();
    return pos->z - g_theTerrainLogic->Get_Ground_Height(pos->x, pos->y, nullptr);
}

bool Thing::Is_KindOf(KindOfType t) const
{
    return Get_Template()->Is_KindOf(t);
}

bool Thing::Is_KindOf_Multi(const BitFlags<KINDOF_COUNT> &must_be_set, const BitFlags<KINDOF_COUNT> &must_be_clear) const
{
    return Get_Template()->Is_KindOf_Multi(must_be_set, must_be_clear);
}

bool Thing::Is_Any_KindOf(const BitFlags<KINDOF_COUNT> &any_kind_of) const
{
    return Get_Template()->Is_Any_KindOf(any_kind_of);
}

void Thing::Set_Orientation(float angle)
{
    float old_angle = m_cachedAngle;
    Coord3D old_pos = m_cachedPos;
    Matrix3D old_tm(m_transform);
    Coord3D pos;
    pos.x = m_transform.Get_X_Translation();
    pos.y = m_transform.Get_Y_Translation();
    pos.z = m_transform.Get_Z_Translation();

    if (m_template->Is_KindOf(KINDOF_STICK_TO_TERRAIN_SLOPE)) {
        g_theTerrainLogic->Align_On_Terrain(angle, pos, true, m_transform);
    } else {
        Coord3D pos1;
        pos1.x = 0.0f;
        pos1.y = 0.0f;
        pos1.z = 1.0f;

        Coord3D pos2;
        pos2.x = Cos(angle);
        pos2.y = Sin(angle);
        pos2.z = 0.0f;

        Coord3D pos3;
        Coord3D::Cross_Product(&pos1, &pos2, &pos3);

        Coord3D pos4;
        Coord3D::Cross_Product(&pos3, &pos1, &pos4);

        m_transform.Set(pos4.x, pos3.x, pos1.x, pos.x, pos4.y, pos3.y, pos1.y, pos.y, pos4.z, pos3.z, pos1.z, pos.z);
    }

    m_cachedAngle = Normalize_Angle(angle);
    m_cachedPos = pos;
    m_cacheFlags &= ~VALID_DIRVECTOR;
    React_To_Transform_Change(&old_tm, &old_pos, old_angle);
    captainslog_dbgassert(
        !GameMath::Is_Nan(Get_Position()->x) && !GameMath::Is_Nan(Get_Position()->y) && !GameMath::Is_Nan(Get_Position()->z),
        "Drawable/Object position NAN! \'%s\'",
        m_template->Get_Name().Str());
}

void Thing::Set_Position(const Coord3D *pos)
{
    if (m_template->Is_KindOf(KINDOF_STICK_TO_TERRAIN_SLOPE)) {
        Matrix3D tm;
        g_theTerrainLogic->Align_On_Terrain(Get_Orientation(), *pos, true, tm);
        Set_Transform_Matrix(&tm);
    } else {
        float old_angle = m_cachedAngle;
        Coord3D old_pos = m_cachedPos;
        Matrix3D old_tm(m_transform);
        m_transform.Set_X_Translation(pos->x);
        m_transform.Set_Y_Translation(pos->y);
        m_transform.Set_Z_Translation(pos->z);
        m_cachedPos = *pos;
        m_cacheFlags &= ~(VALID_ALTITUDE_TERRAIN | VALID_ALTITUDE_SEALEVEL);
        React_To_Transform_Change(&old_tm, &old_pos, old_angle);
    }

    captainslog_dbgassert(
        !GameMath::Is_Nan(Get_Position()->x) && !GameMath::Is_Nan(Get_Position()->y) && !GameMath::Is_Nan(Get_Position()->z),
        "Drawable/Object position NAN! \'%s\'",
        m_template->Get_Name().Str());
}

void Thing::Set_Transform_Matrix(const Matrix3D *mx)
{
    float old_angle = m_cachedAngle;
    Coord3D old_pos = m_cachedPos;
    Matrix3D old_tm(m_transform);
    m_transform = *mx;
    m_cachedPos.x = m_transform.Get_X_Translation();
    m_cachedPos.y = m_transform.Get_Y_Translation();
    m_cachedPos.z = m_transform.Get_Z_Translation();
    m_cachedAngle = m_transform.Get_Z_Rotation();
    m_cacheFlags = 0;
    React_To_Transform_Change(&old_tm, &old_pos, old_angle);

    captainslog_dbgassert(
        !GameMath::Is_Nan(Get_Position()->x) && !GameMath::Is_Nan(Get_Position()->y) && !GameMath::Is_Nan(Get_Position()->z),
        "Drawable/Object position NAN! \'%s\'",
        m_template->Get_Name().Str());
}

float Thing::Get_Height_Above_Terrain_Or_Water() const
{
    if ((m_cacheFlags & VALID_ALTITUDE_SEALEVEL) == 0) {
        const Coord3D *pos = Get_Position();
        float z;

        if (g_theTerrainLogic->Is_Underwater(pos->x, pos->y, &z, nullptr)) {
            m_cachedAltitudeAboveTerrainOrWater = pos->z - z;
        } else {
            m_cachedAltitudeAboveTerrainOrWater = Get_Height_Above_Terrain();
        }

        m_cacheFlags |= VALID_ALTITUDE_SEALEVEL;
    }

    return m_cachedAltitudeAboveTerrainOrWater;
}

bool Thing::Is_Significantly_Above_Terrain() const
{
    return -9.0f * g_theWriteableGlobalData->m_gravity < Get_Height_Above_Terrain();
}

void Thing::Convert_Bone_Pos_To_World_Pos(
    const Coord3D *bone_pos, const Matrix3D *bone_transform, Coord3D *world_pos, Matrix3D *world_transform) const
{
    if (world_transform != nullptr) {
        world_transform->Mul(m_transform, *bone_transform);
    }

    if (world_pos != nullptr) {
        Vector3 in;
        in.X = bone_pos->x;
        in.Y = bone_pos->y;
        in.Z = bone_pos->z;
        Matrix3D::Transform_Vector(m_transform, in, &in);
        world_pos->x = in.X;
        world_pos->y = in.Y;
        world_pos->z = in.Z;
    }
}

void Thing::Transform_Point(const Coord3D *in, Coord3D *out)
{
    if (in != nullptr && out != nullptr) {
        Vector3 vin;
        Vector3 vout;
        vin.X = in->x;
        vin.Y = in->y;
        vin.Z = in->z;
        Matrix3D::Transform_Vector(m_transform, vin, &vout);
        out->x = vout.X;
        out->y = vout.Y;
        out->z = vout.Z;
    }
}
