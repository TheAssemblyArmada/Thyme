/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Prop Draw Module
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

class W3DPropDrawModuleData : public ModuleData
{
public:
    W3DPropDrawModuleData() {}
    virtual ~W3DPropDrawModuleData() override {}
    virtual const W3DPropDrawModuleData *Get_As_W3D_Prop_Draw_Module_Data() const { return this; }

    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_modelName;
    friend class W3DPropDraw;
};

class W3DPropDraw : public DrawModule
{
    IMPLEMENT_POOL(W3DPropDraw)

public:
    W3DPropDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DPropDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override {}
    virtual void Set_Shadows_Enabled(bool enable) override {}
    virtual void Release_Shadows() override {}
    virtual void Allocate_Shadows() override {}
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override {}
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    virtual void React_To_Geometry_Change() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    const W3DPropDrawModuleData *Get_W3D_Prop_Draw_Module_Data() const
    {
        return static_cast<const W3DPropDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    bool m_propCreated;
};