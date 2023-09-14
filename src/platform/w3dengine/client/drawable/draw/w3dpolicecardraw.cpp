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
#include "w3dpolicecardraw.h"
#include "drawable.h"
#include "hanim.h"
#include "randomvalue.h"
#include "rendobj.h"
#include "w3ddisplay.h"
#include "w3ddynamiclight.h"
#include "w3dscene.h"

W3DPoliceCarDraw::W3DPoliceCarDraw(Thing *thing, ModuleData const *module_data) :
    W3DTruckDraw(thing, module_data), m_light(nullptr)
{
    m_curFrame = Get_Client_Random_Value_Real(0.0f, 10.0f);
}

W3DPoliceCarDraw::~W3DPoliceCarDraw()
{
    if (m_light != nullptr) {
        m_light->Set_Frame_Fade(0, 5);
        m_light->Set_Decay_Range();
        m_light->Set_Decay_Color();
        m_light = nullptr;
    }
}

NameKeyType W3DPoliceCarDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DPoliceCarDraw");
    return _key;
}

void W3DPoliceCarDraw::Do_Draw_Module(const Matrix3D *transform)
{
    RenderObjClass *robj = Get_Render_Object();

    if (robj != nullptr) {
        HAnimClass *anim = robj->Peek_Animation();

        if (anim != nullptr) {
            float frames = anim->Get_Num_Frames();
            m_curFrame += 0.25f;

            if (frames - 1.0f < m_curFrame) {
                m_curFrame = 0.0f;
            }

            robj->Set_Animation(anim, m_curFrame);
        }

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        if (m_curFrame < 3.0f) {
            r = 1.0f;
            g = 0.5f;
        } else if (m_curFrame < 6.0f) {
            r = 1.0f;
        } else if (m_curFrame < 7.0f) {
            r = 1.0f;
            g = 0.5f;
        } else if (m_curFrame < 9.0f) {
            r = (9.0f - m_curFrame) / 4.0f + 0.5f;
            b = (m_curFrame - 5.0f) / 6.0f;
        } else if (m_curFrame < 12.0f) {
            b = 1.0f;
        } else if (m_curFrame <= 14.0f) {
            g = (m_curFrame - 11.0f) / 3.0f;
            b = (14.0f - m_curFrame) / 2.0f;
            r = (m_curFrame - 11.0f) / 3.0f;
        }

        if (m_light == nullptr) {
            m_light = Create_Dynamic_Light();
        }

        if (m_light != nullptr) {
            const Coord3D *pos = Get_Drawable()->Get_Position();
            float x = pos->x;
            float y = pos->y;
            float z = pos->z;
            m_light->Set_Diffuse(Vector3(r, g, b));
            m_light->Set_Ambient(Vector3(r / 2.0f, g / 2.0f, b / 2.0f));
            m_light->Set_Far_Attenuation_Range(3.0f, 20.0f);
            m_light->Set_Position(Vector3(x, y, z + 8.0f));
        }

        W3DTruckDraw::Do_Draw_Module(transform);
    }
}

void W3DPoliceCarDraw::CRC_Snapshot(Xfer *xfer)
{
    W3DTruckDraw::CRC_Snapshot(xfer);
}

void W3DPoliceCarDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    W3DTruckDraw::Xfer_Snapshot(xfer);
}

void W3DPoliceCarDraw::Load_Post_Process()
{
    W3DTruckDraw::Load_Post_Process();
}

W3DDynamicLight *W3DPoliceCarDraw::Create_Dynamic_Light()
{
    W3DDynamicLight *light = W3DDisplay::s_3DScene->Get_A_Dynamic_Light();

    if (light != nullptr) {
        light->Set_Enabled(true);
        light->Set_Ambient(Vector3(0.0f, 0.0f, 0.0f));
        light->Set_Diffuse(Vector3(0.0f, 0.0f, 0.0f));
        light->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
        light->Set_Far_Attenuation_Range(5.0f, 15.0f);
    }

    return light;
}

Module *W3DPoliceCarDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DPoliceCarDraw, thing, module_data);
}
