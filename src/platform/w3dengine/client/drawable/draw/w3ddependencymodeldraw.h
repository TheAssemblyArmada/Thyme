/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Dependency Model Draw Module
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

class W3DDependencyModelDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DDependencyModelDrawModuleData() {}
    virtual ~W3DDependencyModelDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_attachToDrawableBoneInContainer;
    friend class W3DDependencyModelDraw;
};

class W3DDependencyModelDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DDependencyModelDraw);

public:
    W3DDependencyModelDraw(Thing *thing, ModuleData const *module_data) :
        W3DModelDraw(thing, module_data), m_doDrawModule(false)
    {
    }

    virtual ~W3DDependencyModelDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Adjust_Transform_Mtx(Matrix3D &transform) const;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Notify_Draw_Module_Dependency_Cleared() override { m_doDrawModule = true; }

    const W3DDependencyModelDrawModuleData *Get_W3D_Dependency_Model_Draw_Module_Data() const
    {
        return static_cast<const W3DDependencyModelDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    bool m_doDrawModule;
};
