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
#include "w3dpropdraw.h"
#include "baseheightmap.h"
#include "drawable.h"

void W3DPropDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    static const FieldParse dataFieldParse[] = {
        { "ModelName", &INI::Parse_AsciiString, nullptr, offsetof(W3DPropDrawModuleData, m_modelName) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DPropDraw::W3DPropDraw(Thing *thing, ModuleData const *module_data) : DrawModule(thing, module_data), m_propCreated(false)
{
}

void W3DPropDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DPropDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
}

void W3DPropDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

ModuleData *W3DPropDraw::Friend_New_Module_Data(INI *ini)
{
    W3DPropDrawModuleData *data = new W3DPropDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DPropDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DPropDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DPropDraw, thing, module_data);
}

NameKeyType W3DPropDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DPropDraw");
    return _key;
}

void W3DPropDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    Drawable *drawable = Get_Drawable();

    if (!m_propCreated && (drawable->Get_Position()->x != 0.0f || drawable->Get_Position()->y != 0.0f)) {
        m_propCreated = true;
        const W3DPropDrawModuleData *data = Get_W3D_Prop_Draw_Module_Data();

        if (data != nullptr) {
            g_theTerrainRenderObject->Add_Prop(drawable->Get_ID(),
                *drawable->Get_Position(),
                drawable->Get_Orientation(),
                drawable->Get_Scale(),
                data->m_modelName);
        }
    }
}
