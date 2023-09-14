/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Overlord Tank Draw Module
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
#include "w3dtankdraw.h"

class W3DOverlordTankDrawModuleData : public W3DTankDrawModuleData
{
public:
    W3DOverlordTankDrawModuleData() {}
    virtual ~W3DOverlordTankDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    // these members seem to be leftovers (possibly a mistaken copy+paste from W3DTankDraw) and are unused.
    Utf8String m_unk1;
    Utf8String m_unk2;
    int m_unk3;
    int m_unk4;
    int m_unk5;
};

class W3DOverlordTankDraw : public W3DTankDraw
{
    IMPLEMENT_POOL(W3DOverlordTankDraw);

public:
    W3DOverlordTankDraw(Thing *thing, ModuleData const *module_data) : W3DTankDraw(thing, module_data) {}

    virtual ~W3DOverlordTankDraw() override {}
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Set_Hidden(bool hidden) override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);
};
