/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Laser Draw
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

class SegmentedLineClass;
class TextureClass;

class LaserDrawInterface
{
public:
    virtual float Get_Laser_Template_Width() const = 0;
};

class W3DLaserDrawModuleData : public ModuleData
{
    friend class W3DLaserDraw;

public:
    W3DLaserDrawModuleData();
    virtual ~W3DLaserDrawModuleData() override {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    int m_innerColor;
    int m_outerColor;
    float m_innerBeamWidth;
    float m_outerBeamWidth;
    float m_scrollRate;
    bool m_tile;
    unsigned int m_numBeams;
    int m_maxIntensityFrames;
    int m_fadeFrames;
    Utf8String m_textureName;
    unsigned int m_segments;
    float m_arcHeight;
    float m_segmentOverlapRatio;
    float m_tilingScalar;
};

class W3DLaserDraw : public DrawModule, public LaserDrawInterface
{
    IMPLEMENT_POOL(W3DLaserDraw);

public:
    W3DLaserDraw(Thing *thing, ModuleData const *module_data);
    virtual ~W3DLaserDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void CRC_Snapshot(Xfer *xfer) override { DrawModule::CRC_Snapshot(xfer); }
    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
        DrawModule::Xfer_Snapshot(xfer);
    }
    virtual void Load_Post_Process() override
    {
        DrawModule::Load_Post_Process();
        m_setLaserPosition = true;
    }
    virtual float Get_Laser_Template_Width() const override;
    virtual void Allocate_Shadows() override {}
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual LaserDrawInterface *Get_Laser_Draw_Interface() override { return this; }
    virtual const LaserDrawInterface *Get_Laser_Draw_Interface() const override { return this; }
    virtual bool Is_Laser() const override { return true; }
    virtual void React_To_Geometry_Change() override {}
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override {}
    virtual void Release_Shadows() override {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override {}
    virtual void Set_Shadows_Enabled(bool enable) override {}

    const W3DLaserDrawModuleData *Get_W3D_Laser_Draw_Module_Data() const
    {
        return static_cast<const W3DLaserDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    SegmentedLineClass **m_line3D;
    TextureClass *m_texture;
    float m_textureAspectRatio;
    bool m_setLaserPosition;
};
