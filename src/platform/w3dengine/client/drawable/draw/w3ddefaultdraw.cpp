/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Default Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddefaultdraw.h"
#include "drawable.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dscene.h"
#include "w3dshadow.h"

W3DDefaultDraw::W3DDefaultDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data), m_renderObject(nullptr), m_shadow(nullptr)
{
    Drawable *drawable = Get_Drawable();
    const ThingTemplate *tmplate = drawable->Get_Template();

    if (!tmplate->Get_LTA_Name().Is_Empty()) {
        m_renderObject = W3DDisplay::s_assetManager->Create_Render_Obj(
            tmplate->Get_LTA_Name().Str(), drawable->Get_Scale(), 0, nullptr, nullptr);
        Shadow::ShadowTypeInfo info;
        info.m_type = SHADOW_VOLUME;
        info.m_sizeX = 0;
        info.m_sizeY = 0;
        info.m_offsetX = 0;
        info.m_offsetY = 0;
        m_shadow = g_theW3DShadowManager->Add_Shadow(m_renderObject, &info, nullptr);

        captainslog_dbgassert(m_renderObject != nullptr, "Test asset %s not found", tmplate->Get_LTA_Name().Str());

        if (m_renderObject != nullptr) {
            W3DDisplay::s_3DScene->Add_Render_Object(m_renderObject);
            m_renderObject->Set_User_Data(drawable->Get_Drawable_Info(), false);
            Matrix3D m;
            m.Set(Vector3(0.0f, 0.0f, 0.0f));
            m_renderObject->Set_Transform(m);
        }
    }
}

W3DDefaultDraw::~W3DDefaultDraw()
{
    if (g_theW3DShadowManager != nullptr && m_shadow != nullptr) {
        g_theW3DShadowManager->Remove_Shadow(m_shadow);
    }

    if (m_renderObject != nullptr) {
        W3DDisplay::s_3DScene->Remove_Render_Object(m_renderObject);
        Ref_Ptr_Release(m_renderObject);
        m_renderObject = nullptr;
    }
}

NameKeyType W3DDefaultDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DDefaultDraw");
    return _key;
}

void W3DDefaultDraw::Do_Draw_Module(const Matrix3D *transform)
{
    if (m_renderObject != nullptr) {
        Matrix3D m;

        if (Get_Drawable()->Get_Instance_Scale() == 1.0f) {
            m_renderObject->Set_Transform(*transform);
        } else {
            m = *transform;
            m.Scale(Get_Drawable()->Get_Instance_Scale());
            m_renderObject->Set_ObjectScale(Get_Drawable()->Get_Instance_Scale());
        }
    }
}

void W3DDefaultDraw::Set_Shadows_Enabled(bool enable)
{
    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Render(enable);
    }
}

void W3DDefaultDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Invisible(obscured);
    }
}

void W3DDefaultDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    if (m_renderObject != nullptr) {
        m_renderObject->Set_Transform(*Get_Drawable()->Get_Transform_Matrix());
    }
}

void W3DDefaultDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DDefaultDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
}

void W3DDefaultDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

Module *W3DDefaultDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DDefaultDraw, thing, module_data);
}
