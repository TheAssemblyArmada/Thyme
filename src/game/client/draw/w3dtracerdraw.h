/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tracer Draw Module
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
#include "w3dmodeldraw.h"

class Line3DClass;

class TracerDrawInterface
{
public:
    virtual void Set_Tracer_Params(float speed, float length, float width, const RGBColor &color, float opacity) = 0;
};

class W3DTracerDraw : public DrawModule, public TracerDrawInterface
{
    IMPLEMENT_POOL(W3DTracerDraw)

public:
    W3DTracerDraw(Thing *thing, ModuleData const *module_data);

    ~W3DTracerDraw() override;
    NameKeyType Get_Module_Name_Key() const override;
    void Do_Draw_Module(const Matrix3D *transform) override;
    void Set_Shadows_Enabled(bool enable) override {}
    void Release_Shadows() override {}
    void Allocate_Shadows() override {}
    void Set_Fully_Obscured_By_Shroud(bool obscured) override {}
    void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    void React_To_Geometry_Change() override {}
    TracerDrawInterface *Get_Tracer_Draw_Interface() override { return this; }
    const TracerDrawInterface *Get_Tracer_Draw_Interface() const override { return this; }

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    void Set_Tracer_Params(float speed, float length, float width, const RGBColor &color, float opacity) override;

    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    Line3DClass *m_theTracer;
    float m_length;
    float m_width;
    RGBColor m_color;
    float m_speedInDistPerFrame;
    float m_opacity;
};
