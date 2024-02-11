/**
 * @file
 *
 * @author tomsons26
 *
 * @brief scene classes
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scene.h"
#include "camera.h"
#include "chunkio.h"
#include "coltest.h"
#include "dx8wrapper.h"
#include "light.h"
#include "lightenv.h"
#include "rinfo.h"
#include "w3d.h"
#include "w3d_file.h"

SceneClass::SceneClass() :
    m_ambientLight(0.5f, 0.5f, 0.5f),
    m_polyRenderMode(FILL),
    m_extraPassPolyRenderMode(EXTRA_PASS_DISABLE),
    m_fogEnabled(false),
    m_fogColor(0.0f, 0.0f, 0.0f),
    m_fogStart(0.0f),
    m_fogEnd(1000.0f)
{
}

// 0x00832710
void SceneClass::Save(ChunkSaveClass &csave)
{
    captainslog_dbgassert(false, "SceneClass::Save is not used");
}

// 0x008327E0
void SceneClass::Load(ChunkLoadClass &cload)
{
    captainslog_dbgassert(false, "SceneClass::Load is not used");
}

// 0x00832550
void SceneClass::Render(RenderInfoClass &rinfo)
{
    Pre_Render_Processing(rinfo);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Fog(m_fogEnabled, m_fogColor, m_fogStart, m_fogEnd);
#endif
    if (m_extraPassPolyRenderMode == EXTRA_PASS_DISABLE) {
        Customized_Render(rinfo);
    } else {

        bool state = W3D::Is_Texturing_Enabled();
#ifdef BUILD_WITH_D3D8
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0); // BFME2 calls Set_DX8_ZBias(0.0f) here
#endif
        Customized_Render(rinfo);

        if (m_extraPassPolyRenderMode == EXTRA_PASS_LINE) {
            W3D::Enable_Texturing(false);
#ifdef BUILD_WITH_D3D8
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7); // BFME2 calls Set_DX8_ZBias(7.0f) here
#endif
            Customized_Render(rinfo);
        } else if (m_extraPassPolyRenderMode == EXTRA_PASS_CLEAR_LINE) {
            DX8Wrapper::Clear(true, false, { 0, 0, 0 }, 0.0f);
            W3D::Enable_Texturing(false);
#ifdef BUILD_WITH_D3D8
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7); // BFME2 calls Set_DX8_ZBias(7.0f) here
#endif
            Customized_Render(rinfo);
        }

        W3D::Enable_Texturing(state);
    }

    Post_Render_Processing(rinfo);
}

// 0x00832890
SimpleSceneClass::SimpleSceneClass() : m_visibilityChecked(false) {}

// 0x00832B70
void SimpleSceneClass::Add_Render_Object(RenderObjClass *obj)
{
    SceneClass::Add_Render_Object(obj);
    m_renderList.Add(obj);
}

// 0x00832BB0
void SimpleSceneClass::Remove_Render_Object(RenderObjClass *obj)
{
    SceneClass::Remove_Render_Object(obj);
    m_renderList.Remove(obj);
}

// 0x00833160
SceneIterator *SimpleSceneClass::Create_Iterator(bool onlyvisible)
{
    // onlyvisible is not used!
    return new SimpleSceneIterator(&m_renderList, onlyvisible);
}

void SimpleSceneClass::Destroy_Iterator(SceneIterator *it)
{
    delete it;
}

// 0x00832BF0
void SimpleSceneClass::Register(RenderObjClass *obj, RegType for_what)
{
    switch (for_what) {
        case RegType::ON_FRAME_UPDATE:
            m_updateList.Add(obj);
            break;
        case RegType::LIGHT:
            m_lightList.Add(obj);
            break;
        case RegType::RELEASE:
            m_releaseList.Add(obj);
            break;
        default:
            break;
    }
}

// 0x00832C80
void SimpleSceneClass::Unregister(RenderObjClass *obj, RegType for_what)
{
    switch (for_what) {
        case RegType::ON_FRAME_UPDATE:
            m_updateList.Remove(obj);
            break;
        case RegType::LIGHT:
            m_lightList.Remove(obj);
            break;
        case RegType::RELEASE:
            m_releaseList.Remove(obj);
            break;
        default:
            break;
    }
}

// 0x00832DF0
float SimpleSceneClass::Compute_Point_Visibility(RenderInfoClass &rinfo, const Vector3 &point)
{
    CastResultStruct res;
    auto camera_position = rinfo.m_camera.Get_Position();
    LineSegClass ray(camera_position, point);
    RayCollisionTestClass raytest(ray, &res);

    for (auto iter = m_renderList.Iterator(); iter; ++iter) {
        auto *robj = iter.Peek_Obj();
        robj->Cast_Ray(raytest);
    }
    return res.fraction == 1.0f ? 1.0f : 0.0f;
}

// 0x00832F40
void SimpleSceneClass::Customized_Render(RenderInfoClass &rinfo)
{
    if (m_visibilityChecked == false) {
        Visibility_Check(&rinfo.m_camera);
    }

    m_visibilityChecked = false;

    for (auto iter = m_updateList.Iterator(); iter; ++iter) {
        auto *robj = iter.Peek_Obj();
        robj->On_Frame_Update();
    }

#ifdef BUILD_WITH_D3D8
    for (auto i = 0; i < LightEnvironmentClass::MAX_LIGHTS; ++i) {
        DX8Wrapper::Set_Light(i, nullptr);
    }
#endif

    auto *light_environment = rinfo.m_lightEnvironment;
    captainslog_dbgassert(light_environment == nullptr, "Light Environment already exists");
    if (light_environment == nullptr) {
        static LightEnvironmentClass _light_env;
        _light_env.Reset({ 0, 0, 0 }, m_ambientLight);
        for (auto iter = m_lightList.Iterator(); iter; ++iter) {
            _light_env.Add_Light(*(static_cast<LightClass *>(iter.Get_Obj())));
        }
        _light_env.Pre_Render_Update(rinfo.m_camera.Get_Transform());
        rinfo.m_lightEnvironment = &_light_env;
    }

    for (auto iter = m_renderList.Iterator(); iter; ++iter) {
        auto *robj = iter.Peek_Obj();
        if (robj->Is_Really_Visible()) {
            auto *unk = robj->Get_Unknown();
            if (unk != nullptr) {
                if (unk->Unk1(robj, rinfo)) {
                    robj->Render(rinfo);
                }
                unk->Unk2(robj, rinfo);
            } else {
                robj->Render(rinfo);
            }
        }
    }
}

// 0x008330E0
void SimpleSceneClass::Post_Render_Processing(RenderInfoClass &rinfo)
{
    for (auto iter = m_releaseList.Iterator(); iter; ++iter) {
        auto *robj = iter.Get_Obj();
        auto *container = robj->Get_Container();
        if (container != nullptr) {
            container->Remove_Sub_Object(robj);
        } else {
            robj->Remove();
        }
    }
    m_releaseList.Reset_List();
}

// 0x00832B30
void SimpleSceneClass::Remove_All_Render_Objects()
{
    for (auto *robj = m_renderList.Remove_Head(); robj != nullptr; robj = m_renderList.Remove_Head()) {
        SceneClass::Remove_Render_Object(robj);
        robj->Release_Ref();
    }
}

// 0x00832D20
void SimpleSceneClass::Visibility_Check(CameraClass *camera)
{
    if (m_renderList.Is_Empty()) {
        m_visibilityChecked = true;
        return;
    }

    for (auto iter = m_renderList.Iterator(); iter; ++iter) {
        auto *robj = iter.Get_Obj();
        if (robj->Is_Force_Visible()) {
            robj->Set_Visible(1);
        } else {
            auto &sphere = robj->Get_Bounding_Sphere();
            robj->Set_Visible(!camera->Cull_Sphere(sphere));
        }
        if (robj->Is_Really_Visible()) {
            if (!robj->Is_Unk_15()) {
                robj->Prepare_LOD(*camera);
            }
        }
    }
    m_visibilityChecked = true;
}
