/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Overlord Aircraft Draw Module
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

class W3DOverlordAircraftDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DOverlordAircraftDrawModuleData() {}
    ~W3DOverlordAircraftDrawModuleData() override {}
    static void Build_Field_Parse(MultiIniFieldParse &p);
};

class W3DOverlordAircraftDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DOverlordAircraftDraw);

public:
    W3DOverlordAircraftDraw(Thing *thing, ModuleData const *module_data) : W3DModelDraw(thing, module_data) {}

    ~W3DOverlordAircraftDraw() override {}
    NameKeyType Get_Module_Name_Key() const override;
    void Do_Draw_Module(const Matrix3D *transform) override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    void Set_Hidden(bool hidden) override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);
};
