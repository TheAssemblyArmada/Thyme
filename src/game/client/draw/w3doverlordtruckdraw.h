/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Overlord Truck Draw Module
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
#include "w3dtruckdraw.h"

class W3DOverlordTruckDrawModuleData : public W3DTruckDrawModuleData
{
public:
    W3DOverlordTruckDrawModuleData() {}
    ~W3DOverlordTruckDrawModuleData() override {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    // these members seem to be leftovers (possibly a mistaken copy+paste) and are unused.
    Utf8String m_unk1;
    Utf8String m_unk2;
    int m_unk3;
    int m_unk4;
    int m_unk5;
};

class W3DOverlordTruckDraw : public W3DTruckDraw
{
    IMPLEMENT_POOL(W3DOverlordTruckDraw);

public:
    W3DOverlordTruckDraw(Thing *thing, ModuleData const *module_data) : W3DTruckDraw(thing, module_data) {}

protected:
    ~W3DOverlordTruckDraw() override {}

public:
    NameKeyType Get_Module_Name_Key() const override;
    void Do_Draw_Module(const Matrix3D *transform) override;

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    void Set_Hidden(bool hidden) override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);
};
