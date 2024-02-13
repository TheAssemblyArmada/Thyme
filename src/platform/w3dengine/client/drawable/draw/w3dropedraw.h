/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Rope Draw Module
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
#include "color.h"
#include "w3dmodeldraw.h"

class Line3DClass;

class RopeDrawInterface
{
public:
    virtual void Init_Rope_Params(
        float height, float width, const RGBColor &color, float wobble_len, float wobble_amplitude, float wobble_rate) = 0;
    virtual void Set_Rope_Cur_Len(float length) = 0;
    virtual void Set_Rope_Speed(float initial_speed, float max_speed, float gravity) = 0;
};

class W3DRopeDraw : public DrawModule, public RopeDrawInterface
{
    IMPLEMENT_POOL(W3DRopeDraw)

public:
    W3DRopeDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DRopeDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Shadows_Enabled(bool enable) override {}
    virtual void Release_Shadows() override {}
    virtual void Allocate_Shadows() override {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override {}
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override {}
    virtual void React_To_Geometry_Change() override {}
    virtual RopeDrawInterface *Get_Rope_Draw_Interface() { return this; }
    virtual const RopeDrawInterface *Get_Rope_Draw_Interface() const { return this; }

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Init_Rope_Params(float height,
        float width,
        const RGBColor &color,
        float wobble_len,
        float wobble_amplitude,
        float wobble_rate) override;
    virtual void Set_Rope_Cur_Len(float length) override;
    virtual void Set_Rope_Speed(float initial_speed, float max_speed, float gravity) override;

    void Build_Segments();
    void Toss_Segments();

    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    struct SegInfo
    {
        Line3DClass *m_line1;
        Line3DClass *m_line2;
        float m_cosAngle;
        float m_sinAngle;
    };

    std::vector<SegInfo> m_segments;
    float m_ropeLength;
    float m_height;
    float m_width;
    RGBColor m_color;
    float m_curDropSpeed;
    float m_maxDropSpeed;
    float m_gravity;
    float m_wobbleLen;
    float m_wobbleAmplitude;
    float m_wobbleRate;
    float m_wobbleAngle;
    float m_curDropHeight;
};
