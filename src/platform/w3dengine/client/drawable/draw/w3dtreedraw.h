/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tree Draw
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

class W3DTreeDrawModuleData : public ModuleData
{
public:
    W3DTreeDrawModuleData();
    virtual ~W3DTreeDrawModuleData() override {}
    virtual const W3DTreeDrawModuleData *Get_As_W3D_Tree_Draw_Module_Data() const override { return this; }
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_modelName;
    Utf8String m_textureName;
    unsigned int m_moveOutwardTime;
    unsigned int m_moveInwardTime;
    float m_moveOutwardDistanceFactor;
    float m_darkeningFactor;
    FXList *m_toppleFX;
    FXList *m_bounceFX;
    Utf8String m_stumpName;
    float m_initialVelocityPercent;
    float m_initialAccelPercent;
    float m_bounceVelocityPercent;
    float m_minimumToppleSpeed;
    bool m_killWhenFinishedToppling;
    bool m_doTopple;
    unsigned int m_sinkTime;
    float m_sinkDistance;
    bool m_doShadow;
    friend class W3DTreeBuffer;
};

class W3DTreeDraw : public DrawModule
{
    IMPLEMENT_POOL(W3DTreeDraw);

public:
    W3DTreeDraw(Thing *thing, ModuleData const *module_data);
    virtual ~W3DTreeDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override {}
    virtual void Set_Shadows_Enabled(bool enable) override {}
    virtual void Release_Shadows() override {}
    virtual void Allocate_Shadows() override {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override {}
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    virtual void React_To_Geometry_Change() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override { DrawModule::CRC_Snapshot(xfer); }
    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
        DrawModule::Xfer_Snapshot(xfer);
    }
    virtual void Load_Post_Process() override { DrawModule::Load_Post_Process(); }

    const W3DTreeDrawModuleData *Get_W3D_Tree_Draw_Module_Data() const
    {
        return static_cast<const W3DTreeDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    bool m_transformSet;
};
