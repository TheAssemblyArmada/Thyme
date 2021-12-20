/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Draw Module
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
#include "gametype.h"
#include "module.h"
class DebrisDrawInterface;
class TracerDrawInterface;
class RopeDrawInterface;
class LaserDrawInterface;
class Marix3D;
class OBBoxClass;
#ifdef GAME_DEBUG_STRUCTS
class DebugDrawStats
{
public:
    DebugDrawStats() : m_extraDrawCalls(0), m_skins(0), m_bones(0), m_sortMeshes(0), m_drawCalls(0) {}
    void Get_Draw_Calls() { return m_drawCalls; }
    void Get_bones() { return m_bones; }
    void Get_Skins() { return m_skins; }
    void Get_Sort_Meshes() { return m_sortMeshes; }
    void Get_Extra_Draw_Calls() { return m_extraDrawCalls; }
    void Add_Extra_Draw_Calls(int i) { m_extraDrawCalls += i; }

private:
    int m_drawCalls;
    int m_sortMeshes;
    int m_skins;
    int m_bones;
    int m_extraDrawCalls;
};
#endif

class ObjectDrawInterface
{
public:
    virtual bool Client_Only_Get_Render_Obj_Info(Coord3D *, float *, Matrix3D *) = 0;
    virtual bool Client_Only_Get_Render_Obj_Bound_Box(OBBoxClass *) = 0;
    virtual bool Client_Only_Get_Render_Obj_Bone_Transform(Utf8String const &, Matrix3D *) = 0;
    virtual int Get_Pristine_Bone_Positions_For_Condition_State(
        BitFlags<MODELCONDITION_COUNT> const &, char const *, int, Coord3D *, Matrix3D *, int) = 0;
    virtual int Get_Current_Bone_Positions(char const *, int, Coord3D *, Matrix3D *, int) = 0;
    virtual bool Get_Current_Worldspace_Client_Bone_Positions(char const *, Matrix3D &) = 0;
    virtual bool Get_Projectile_Launch_Offset(
        BitFlags<MODELCONDITION_COUNT> const &, WeaponSlotType, int, Matrix3D *, WhichTurretType, Coord3D *, Coord3D *) = 0;
    virtual void Update_Projectile_Clip_Status(unsigned int, unsigned int, WeaponSlotType) = 0;
    virtual void Update_Draw_Module_Supply_Status(int, int) = 0;
    virtual void Notify_Draw_Module_Dependency_Cleared() = 0;
    virtual void Set_Hidden(bool) = 0;
    virtual void Replace_Model_Condition_State(BitFlags<MODELCONDITION_COUNT> const &) = 0;
    virtual void Replace_Indicator_Color(int) = 0;
    virtual bool Handle_Weapon_Fire_FX(WeaponSlotType, int, FXList const *, float, Coord3D const *, float) = 0;
    virtual int Get_Barrel_Count(WeaponSlotType) = 0;
    virtual void Set_Selectable(bool) = 0;
    virtual void Set_Animation_Loop_Duration(unsigned int) = 0;
    virtual void Set_Animation_Completion_Time(unsigned int) = 0;
    virtual bool Update_Bones_For_Client_Particle_Systems() = 0;
    virtual void Set_Animation_Frame(int) = 0;
    virtual void Set_Pause_Animation(bool) = 0;
    virtual void Update_Sub_Objects() = 0;
    virtual void Show_Sub_Object(Utf8String const &, bool) = 0;
};

class DrawModule : public DrawableModule
{
    IMPLEMENT_ABSTRACT_POOL(DrawModule);

public:
    virtual ~DrawModule() override;
    virtual void Do_Draw_Module(Marix3D *transform) = 0;
    virtual void Set_Shadows_Enabled(bool enable) = 0;
    virtual void Release_Shadows() = 0;
    virtual void Allocate_Shadows() = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats);
#endif
    virtual void Set_Terrain_Decal(TerrainDecalType decal);
    virtual void Set_Terrain_Decal_Size(float width, float height);
    virtual void Set_Terrain_Decal_Opacity(float opacity);
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured);
    virtual bool Is_Visible() const;
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) = 0;
    virtual void React_To_Geometry_Change() = 0;
    virtual bool Is_Laser() const;
    virtual ObjectDrawInterface *Get_Object_Draw_Interface();
    virtual const ObjectDrawInterface *Get_Object_Draw_Interface() const;
    virtual DebrisDrawInterface *Get_Debris_Draw_Interface();
    virtual const DebrisDrawInterface *Get_Debris_Draw_Interface() const;
    virtual TracerDrawInterface *Get_Tracer_Draw_Interface();
    virtual const TracerDrawInterface *Get_Tracer_Draw_Interface() const;
    virtual RopeDrawInterface *Get_Rope_Draw_Interface();
    virtual const RopeDrawInterface *Get_Rope_Draw_Interface() const;
    virtual LaserDrawInterface *Get_Laser_Draw_Interface();
    virtual const LaserDrawInterface *Get_Laser_Draw_Interface() const;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
};
