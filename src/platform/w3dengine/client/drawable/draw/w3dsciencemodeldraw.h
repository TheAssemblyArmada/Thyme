/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Science Model Draw Module
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

class W3DScienceModelDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DScienceModelDrawModuleData() : m_requiredScience(SCIENCE_INVALID) {}
    virtual ~W3DScienceModelDrawModuleData() override {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    ScienceType m_requiredScience;
    friend class W3DScienceModelDraw;
};

class W3DScienceModelDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DScienceModelDraw);

public:
    W3DScienceModelDraw(Thing *thing, ModuleData const *module_data) : W3DModelDraw(thing, module_data) {}

    virtual ~W3DScienceModelDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

    const W3DScienceModelDrawModuleData *Get_W3D_Science_Model_Draw_Module_Data() const
    {
        return static_cast<const W3DScienceModelDrawModuleData *>(Module::Get_Module_Data());
    }
};
