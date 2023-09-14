/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Supply Draw Module
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

class W3DSupplyDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DSupplyDrawModuleData() {}
    virtual ~W3DSupplyDrawModuleData() override {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_supplyBonePrefix;
    friend class W3DSupplyDraw;
};

class W3DSupplyDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DSupplyDraw)

public:
    W3DSupplyDraw(Thing *thing, ModuleData const *module_data) :
        W3DModelDraw(thing, module_data), m_totalSupplyBoneCount(-1), m_currentSupplyBoneCount(0)
    {
    }

    virtual ~W3DSupplyDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void React_To_Geometry_Change() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Update_Draw_Module_Supply_Status(int max, int current) override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

    const W3DSupplyDrawModuleData *Get_W3D_Supply_Draw_Module_Data() const
    {
        return static_cast<const W3DSupplyDrawModuleData *>(Module::Get_Module_Data());
    }

private:
    int m_totalSupplyBoneCount;
    int m_currentSupplyBoneCount;
};
