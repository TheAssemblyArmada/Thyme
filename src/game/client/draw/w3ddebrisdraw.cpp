/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Debris Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddebrisdraw.h"
#include "drawable.h"
#include "hanim.h"
#include "hlod.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dscene.h"

void Do_FX_Pos(FXList const *list,
    const Coord3D *primary,
    const Matrix3D *primary_mtx,
    float primary_speed,
    const Coord3D *secondary,
    float radius);

bool Is_Animation_Complete(RenderObjClass *robj);

W3DDebrisDraw::W3DDebrisDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data),
    m_renderObject(nullptr),
    m_fxFinal(nullptr),
    m_state(INITIAL),
    m_frames(0),
    m_finalStop(false),
    m_shadow(nullptr)
{
    for (int i = 0; i < STATECOUNT; i++) {
        m_anims[i] = nullptr;
    }
}

W3DDebrisDraw::~W3DDebrisDraw()
{
    if (g_theW3DShadowManager != nullptr && m_shadow != nullptr) {
        g_theW3DShadowManager->Remove_Shadow(m_shadow);
        m_shadow = nullptr;
    }

    if (m_renderObject != nullptr) {
        W3DDisplay::s_3DScene->Remove_Render_Object(m_renderObject);
        Ref_Ptr_Release(m_renderObject);
        m_renderObject = nullptr;
    }

    for (int i = 0; i < STATECOUNT; i++) {
        Ref_Ptr_Release(m_anims[i]);
        m_anims[i] = nullptr;
    }
}

NameKeyType W3DDebrisDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DDebrisDraw");
    return _key;
}

void W3DDebrisDraw::Do_Draw_Module(const Matrix3D *transform)
{
    if (m_renderObject != nullptr) {
        Matrix3D tm;
        Drawable *drawable = Get_Drawable();

        if (drawable->Get_Instance_Scale() != 1.0f) {
            tm = *transform;
            tm.Scale(drawable->Get_Instance_Scale());
            transform = &tm;
            m_renderObject->Set_ObjectScale(drawable->Get_Instance_Scale());
        }

        m_renderObject->Set_Transform(*transform);
        int old_state = m_state;
        Object *object = drawable->Get_Object();

        if (m_state != FINAL && object != nullptr && !object->Is_Above_Terrain() && m_frames > 3) {
            m_state = FINAL;
        } else {
            if (m_state < FINAL && Is_Animation_Complete(m_renderObject)) {
                m_state++;
            }
        }

        HAnimClass *anim = m_anims[m_state];

        if (anim != nullptr) {
            if (anim != m_renderObject->Peek_Animation() || old_state != m_state) {
                static int s_theAnimModes[STATECOUNT] = {
                    RenderObjClass::ANIM_MODE_ONCE, RenderObjClass::ANIM_MODE_LOOP, RenderObjClass::ANIM_MODE_ONCE
                };

                int mode = s_theAnimModes[m_state];

                if (m_state == FINAL) {
                    Do_FX_Pos(m_fxFinal, drawable->Get_Position(), drawable->Get_Transform_Matrix(), 0.0f, nullptr, 0.0f);

                    if (m_finalStop) {
                        mode = RenderObjClass::ANIM_MODE_MANUAL;
                    }
                }

                m_renderObject->Set_Animation(anim, 0.0f, mode);
            }
        }

        m_frames++;
    }
}

void W3DDebrisDraw::Set_Shadows_Enabled(bool enable)
{
    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Render(enable);
    }
}

void W3DDebrisDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Invisible(obscured);
    }
}

void W3DDebrisDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    if (m_renderObject != nullptr) {
        m_renderObject->Set_Transform(*Get_Drawable()->Get_Transform_Matrix());
    }
}

void W3DDebrisDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DDebrisDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
    xfer->xferAsciiString(&m_modelName);
    xfer->xferColor(&m_color);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Set_Model_Name(m_modelName, m_color, SHADOW_NONE);
    }

    xfer->xferAsciiString(&m_initialAnimName);
    xfer->xferAsciiString(&m_flyingAnimName);
    xfer->xferAsciiString(&m_finalAnimName);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Set_Anim_Names(m_initialAnimName, m_flyingAnimName, m_finalAnimName, nullptr);
    }

    xfer->xferInt(&m_state);
    xfer->xferInt(&m_frames);
    xfer->xferBool(&m_finalStop);
}

void W3DDebrisDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

void W3DDebrisDraw::Set_Model_Name(Utf8String name, unsigned int color, ShadowType type)
{
    if (m_renderObject == nullptr && !name.Is_Empty()) {
        int col = 0;

        if (color) {
            col = color | 0xFF000000;
        }

        m_renderObject =
            W3DDisplay::s_assetManager->Create_Render_Obj(name.Str(), Get_Drawable()->Get_Scale(), col, nullptr, nullptr);
        captainslog_dbgassert(m_renderObject, "Debris model %s not found!", name.Str());

        if (m_renderObject != nullptr) {
            W3DDisplay::s_3DScene->Add_Render_Object(m_renderObject);
            m_renderObject->Set_User_Data(Get_Drawable()->Get_Drawable_Info(), 0);
            Matrix3D tm;
            tm.Set(Vector3(0.0f, 0.0f, 0.0f));
            m_renderObject->Set_Transform(tm);
        }

        if (type != SHADOW_NONE) {
            Shadow::ShadowTypeInfo info;
            info.m_type = type;
            info.m_sizeX = 0.0f;
            info.m_sizeY = 0.0f;
            m_shadow = g_theW3DShadowManager->Add_Shadow(m_renderObject, &info, nullptr);
        } else if (g_theW3DShadowManager != nullptr && m_shadow != nullptr) {
            g_theW3DShadowManager->Remove_Shadow(m_shadow);
            m_shadow = nullptr;
        }

        m_modelName = name;
        m_color = color;
    }
}

void W3DDebrisDraw::Set_Anim_Names(Utf8String initial, Utf8String flying, Utf8String final, const FXList *final_fx)
{
    for (int i = 0; i < STATECOUNT; i++) {
        Ref_Ptr_Release(m_anims[i]);
        m_anims[i] = nullptr;
    }

    if (initial.Is_Empty()) {
        m_anims[INITIAL] = nullptr;
    } else {
        m_anims[INITIAL] = W3DDisplay::s_assetManager->Get_HAnim(initial.Str());
    }

    if (flying.Is_Empty()) {
        m_anims[FLYING] = nullptr;
    } else {
        m_anims[FLYING] = W3DDisplay::s_assetManager->Get_HAnim(flying.Str());
    }

    if (strcasecmp(final.Str(), "STOP") == 0) {
        m_finalStop = true;
        final = flying;
    } else {
        m_finalStop = false;
    }

    if (final.Is_Empty()) {
        m_anims[FINAL] = nullptr;
    } else {
        m_anims[FINAL] = W3DDisplay::s_assetManager->Get_HAnim(final.Str());
    }

    m_state = INITIAL;
    m_frames = 0;
    m_fxFinal = final_fx;
    m_initialAnimName = initial;
    m_flyingAnimName = flying;
    m_finalAnimName = final;
}

Module *W3DDebrisDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DDebrisDraw, thing, module_data);
}
