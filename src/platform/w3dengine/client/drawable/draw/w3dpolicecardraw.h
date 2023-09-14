/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Police Car Draw Module
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

class W3DDynamicLight;

class W3DPoliceCarDraw : public W3DTruckDraw
{
    IMPLEMENT_POOL(W3DPoliceCarDraw);

public:
    W3DPoliceCarDraw(Thing *thing, ModuleData const *module_data);

protected:
    virtual ~W3DPoliceCarDraw() override;

public:
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    W3DDynamicLight *Create_Dynamic_Light();

    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    W3DDynamicLight *m_light;
    float m_curFrame;
};
