/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Projectile Stream Draw Module
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
#include "drawmodule.h"
#include "projectilestreamupdate.h"

class TextureClass;
class SegmentedLineClass;

class W3DProjectileStreamDrawModuleData : ModuleData
{
public:
    W3DProjectileStreamDrawModuleData();
    virtual ~W3DProjectileStreamDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_texture;
    float m_width;
    float m_tileFactor;
    float m_scrollRate;
    int m_maxSegments;
    friend class W3DProjectileStreamDraw;
};

class W3DProjectileStreamDraw : DrawModule
{
    IMPLEMENT_POOL(W3DProjectileStreamDraw)

public:
    W3DProjectileStreamDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DProjectileStreamDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Shadows_Enabled(bool enable) override {}
    virtual void Release_Shadows() override {}
    virtual void Allocate_Shadows() override {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override {}
    virtual void React_To_Geometry_Change() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Make_Or_Update_Line(Vector3 *points, unsigned int point_count, int line_index);

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

    const W3DProjectileStreamDrawModuleData *Get_W3D_Projectile_Stream_Draw_Module_Data() const
    {
        return static_cast<const W3DProjectileStreamDrawModuleData *>(Module::Get_Module_Data());
    }

private:
    TextureClass *m_texture;
    SegmentedLineClass *m_allLines[MAX_SEGMENT_COUNT];
    int m_linesValid;
};
