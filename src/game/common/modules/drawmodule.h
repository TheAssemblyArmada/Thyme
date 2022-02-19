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
#include "bitflags.h"
#include "gametype.h"
#include "module.h"
#include "object.h"
class DebrisDrawInterface;
class TracerDrawInterface;
class RopeDrawInterface;
class LaserDrawInterface;
class Matrix3D;
class OBBoxClass;
#ifdef GAME_DEBUG_STRUCTS
class DebugDrawStats
{
public:
    DebugDrawStats() : m_extraDrawCalls(0), m_skins(0), m_bones(0), m_sortMeshes(0), m_drawCalls(0) {}
    int32_t Get_Draw_Calls() const { return m_drawCalls; }
    int32_t Get_Bones() const { return m_bones; }
    int32_t Get_Skins() const { return m_skins; }
    int32_t Get_Sort_Meshes() const { return m_sortMeshes; }
    int32_t Get_Extra_Draw_Calls() const { return m_extraDrawCalls; }
    void Add_Draw_Calls(int32_t i) { m_drawCalls += i; }
    void Add_Bones(int32_t i) { m_bones += i; }
    void Add_Skins(int32_t i) { m_skins += i; }
    void Add_Sort_Meshes(int32_t i) { m_sortMeshes += i; }
    void Add_Extra_Draw_Calls(int32_t i) { m_extraDrawCalls += i; }

private:
    int32_t m_drawCalls;
    int32_t m_sortMeshes;
    int32_t m_skins;
    int32_t m_bones;
    int32_t m_extraDrawCalls;
};
#endif

class ObjectDrawInterface
{
public:
    virtual bool Client_Only_Get_Render_Obj_Info(Coord3D *pos, float *radius, Matrix3D *transform) const = 0;
    virtual bool Client_Only_Get_Render_Obj_Bound_Box(OBBoxClass *box) const = 0;
    virtual bool Client_Only_Get_Render_Obj_Bone_Transform(Utf8String const &bone, Matrix3D *transform) const = 0;
    virtual int32_t Get_Pristine_Bone_Positions_For_Condition_State(BitFlags<MODELCONDITION_COUNT> const &c,
        char const *bone_name,
        int32_t start_index,
        Coord3D *positions,
        Matrix3D *transforms,
        int32_t max_bones) const = 0;
    virtual int32_t Get_Current_Bone_Positions(
        char const *bone_name_prefix, int32_t start_index, Coord3D *positions, Matrix3D *transforms, int32_t max_bones) const = 0;
    virtual bool Get_Current_Worldspace_Client_Bone_Positions(char const *bone_name_prefix, Matrix3D &transform) const = 0;
    virtual bool Get_Projectile_Launch_Offset(BitFlags<MODELCONDITION_COUNT> const &c,
        WeaponSlotType wslot,
        int32_t ammo_index,
        Matrix3D *launch_pos,
        WhichTurretType tur,
        Coord3D *turret_rot_pos,
        Coord3D *turret_pitch_pos) const = 0;
    virtual void Update_Projectile_Clip_Status(uint32_t show, uint32_t count, WeaponSlotType wslot) = 0;
    virtual void Update_Draw_Module_Supply_Status(int32_t status1, int32_t status2) = 0;
    virtual void Notify_Draw_Module_Dependency_Cleared() = 0;
    virtual void Set_Hidden(bool hidden) = 0;
    virtual void Replace_Model_Condition_State(BitFlags<MODELCONDITION_COUNT> const &c) = 0;
    virtual void Replace_Indicator_Color(int32_t color) = 0;
    virtual bool Handle_Weapon_Fire_FX(WeaponSlotType wslot,
        int32_t specific_barrel_to_use,
        FXList const *fxl,
        float weapon_speed,
        Coord3D const *victim_pos,
        float radius) = 0;
    virtual int32_t Get_Barrel_Count(WeaponSlotType wslot) const = 0;
    virtual void Set_Selectable(bool selectable) = 0;
    virtual void Set_Animation_Loop_Duration(uint32_t num_frames) = 0;
    virtual void Set_Animation_Completion_Time(uint32_t num_frames) = 0;
    virtual bool Update_Bones_For_Client_Particle_Systems() = 0;
    virtual void Set_Animation_Frame(int32_t frame) = 0;
    virtual void Set_Pause_Animation(bool pause) = 0;
    virtual void Update_Sub_Objects() = 0;
    virtual void Show_Sub_Object(Utf8String const &sub_obj_name, bool visible) = 0;
};

class DrawModule : public DrawableModule
{
    IMPLEMENT_ABSTRACT_POOL(DrawModule);

public:
    DrawModule(Thing *thing, const ModuleData *module_data) : DrawableModule(thing, module_data) {}
    virtual ~DrawModule() override {}
    virtual void Do_Draw_Module(const Matrix3D *transform) = 0;
    virtual void Set_Shadows_Enabled(bool enable) = 0;
    virtual void Release_Shadows() = 0;
    virtual void Allocate_Shadows() = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats) {}
#endif
    virtual void Set_Terrain_Decal(TerrainDecalType decal) {}
    virtual void Set_Terrain_Decal_Size(float width, float height) {}
    virtual void Set_Terrain_Decal_Opacity(float opacity) {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) = 0;
    virtual bool Is_Visible() const { return true; }
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) = 0;
    virtual void React_To_Geometry_Change() = 0;
    virtual bool Is_Laser() const { return false; }
    virtual ObjectDrawInterface *Get_Object_Draw_Interface() { return nullptr; }
    virtual const ObjectDrawInterface *Get_Object_Draw_Interface() const { return nullptr; }
    virtual DebrisDrawInterface *Get_Debris_Draw_Interface() { return nullptr; }
    virtual const DebrisDrawInterface *Get_Debris_Draw_Interface() const { return nullptr; }
    virtual TracerDrawInterface *Get_Tracer_Draw_Interface() { return nullptr; }
    virtual const TracerDrawInterface *Get_Tracer_Draw_Interface() const { return nullptr; }
    virtual RopeDrawInterface *Get_Rope_Draw_Interface() { return nullptr; }
    virtual const RopeDrawInterface *Get_Rope_Draw_Interface() const { return nullptr; }
    virtual LaserDrawInterface *Get_Laser_Draw_Interface() { return nullptr; }
    virtual const LaserDrawInterface *Get_Laser_Draw_Interface() const { return nullptr; }

    virtual void CRC_Snapshot(Xfer *xfer) override { DrawableModule::CRC_Snapshot(xfer); }

    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
        DrawableModule::Xfer_Snapshot(xfer);
    }

    virtual void Load_Post_Process() override { DrawableModule::Load_Post_Process(); }

    static ModuleType Get_Module_Type() { return MODULE_DRAW; }
    static int32_t Get_Interface_Mask() { return MODULEINTERFACE_DRAW; }
};
