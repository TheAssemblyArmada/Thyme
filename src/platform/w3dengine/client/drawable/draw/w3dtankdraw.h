/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tank Draw Module
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
#include "rendobj.h"
#include "w3dmodeldraw.h"

class W3DTankDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DTankDrawModuleData();
    virtual ~W3DTankDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_treadDebrisNameLeft;
    Utf8String m_treadDebrisNameRight;
    float m_treadAnimationRate;
    float m_treadPivotSpeedFraction;
    float m_treadDriveSpeedFraction;
    friend class W3DTankDraw;
};

class W3DTankDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DTankDraw);

public:
    enum
    {
        MAX_TREADS_PER_TANK = 4,
    };

    enum TreadType
    {
        TREAD_LEFT,
        TREAD_RIGHT,
        TREAD_MIDDLE,
    };

    struct TreadObjectInfo
    {
        RenderObjClass *m_robj;
        TreadType m_type;
        RenderObjClass::Material_Override m_materialSettings;
    };

    W3DTankDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DTankDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual void On_Render_Obj_Recreated() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Set_Hidden(bool hidden) override;

    void Toss_Emitters();
    void Create_Emitters();
    void Start_Move_Debris();
    void Stop_Move_Debris();
    void Update_Tread_Positions(float uv_delta);
    void Update_Tread_Objects();

    const W3DTankDrawModuleData *Get_W3D_Tank_Draw_Module_Data() const
    {
        return static_cast<const W3DTankDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    ParticleSystem *m_treadDebrisLeft;
    ParticleSystem *m_treadDebrisRight;
    RenderObjClass *m_prevRenderObj;
    TreadObjectInfo m_treads[MAX_TREADS_PER_TANK];
    int m_treadCount;
    Coord3D m_treadDirection;
};
