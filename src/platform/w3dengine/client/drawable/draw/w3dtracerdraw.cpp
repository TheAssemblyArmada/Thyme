/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tracer Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtracerdraw.h"
#include "drawable.h"
#include "gamelogic.h"
#include "line3d.h"
#include "w3ddisplay.h"
#include "w3dscene.h"

W3DTracerDraw::W3DTracerDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data),
    m_theTracer(nullptr),
    m_length(20.0f),
    m_width(0.5f),
    m_speedInDistPerFrame(1.0f),
    m_opacity(1.0f)
{
    m_color.red = 0.9f;
    m_color.green = 0.8f;
    m_color.blue = 0.7f;
}

W3DTracerDraw::~W3DTracerDraw()
{
    if (m_theTracer != nullptr) {
        W3DDisplay::s_3DScene->Remove_Render_Object(m_theTracer);
        Ref_Ptr_Release(m_theTracer);
    }
}

NameKeyType W3DTracerDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DTracerDraw");
    return _key;
}

void W3DTracerDraw::Do_Draw_Module(const Matrix3D *transform)
{
    if (m_theTracer == nullptr) {
        m_theTracer = new Line3DClass(Vector3(0.0f, 0.0f, 0.0f),
            Vector3(m_length, 0.0f, 0.0f),
            m_width,
            m_color.red,
            m_color.green,
            m_color.blue,
            m_opacity);
        W3DDisplay::s_3DScene->Add_Render_Object(m_theTracer);
        m_theTracer->Set_Transform(*transform);
    }

    if (Get_Drawable()->Get_Expiration_Date() != 0) {
        m_opacity -= m_opacity / (float)(Get_Drawable()->Get_Expiration_Date() / g_theGameLogic->Get_Frame());
        m_theTracer->Set_Opacity(m_opacity);
    }

    if (m_speedInDistPerFrame != 0.0f) {
        Matrix3D tm = m_theTracer->Get_Transform();
        tm.Translate(Vector3(m_speedInDistPerFrame, 0.0f, 0.0f));
        m_theTracer->Set_Transform(tm);
    }
}

void W3DTracerDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    if (m_theTracer != nullptr) {
        m_theTracer->Set_Transform(*Get_Drawable()->Get_Transform_Matrix());
    }
}

void W3DTracerDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DTracerDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
}

void W3DTracerDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

void W3DTracerDraw::Set_Tracer_Params(float speed, float length, float width, const RGBColor &color, float opacity)
{
    m_speedInDistPerFrame = speed;
    m_length = length;
    m_width = width;
    m_color = color;
    m_opacity = opacity;

    if (m_theTracer != nullptr) {
        m_theTracer->Reset(Vector3(0.0f, 0.0f, 0.0f), Vector3(m_length, 0.0f, 0.0f), m_width);
        m_theTracer->Re_Color(m_color.red, m_color.green, m_color.blue);
        m_theTracer->Set_Opacity(m_opacity);

        if (m_theTracer != nullptr) {
            m_theTracer->Set_Transform(*Get_Drawable()->Get_Transform_Matrix());
        }
    }
}

Module *W3DTracerDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DTracerDraw, thing, module_data);
}
