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
    csave.Begin_Chunk(0x42300);

    csave.Begin_Micro_Chunk(0);
    csave.Write(&m_ambientLight, sizeof(m_ambientLight));
    csave.End_Micro_Chunk();

    csave.Begin_Micro_Chunk(1);
    csave.Write(&m_polyRenderMode, sizeof(m_polyRenderMode));
    csave.End_Micro_Chunk();

    csave.Begin_Micro_Chunk(2);
    csave.Write(&m_fogColor, sizeof(m_fogColor));
    csave.End_Micro_Chunk();

    csave.Begin_Micro_Chunk(3);
    csave.Write(&m_fogEnabled, sizeof(m_fogEnabled));
    csave.End_Micro_Chunk();

    csave.Begin_Micro_Chunk(4);
    csave.Write(&m_fogStart, sizeof(m_fogStart));
    csave.End_Micro_Chunk();

    csave.Begin_Micro_Chunk(5);
    csave.Write(&m_fogEnd, sizeof(m_fogEnd));
    csave.End_Micro_Chunk();

    csave.End_Chunk();
}

// 0x008327E0
void SceneClass::Load(ChunkLoadClass &cload)
{
    cload.Open_Chunk();
    if (cload.Cur_Chunk_ID() == 0x42300) {
        while (cload.Open_Micro_Chunk()) {
            switch (cload.Cur_Micro_Chunk_ID()) {
                case 0:
                    cload.Read(&m_ambientLight, sizeof(m_ambientLight));
                    break;
                case 1:
                    cload.Read(&m_polyRenderMode, sizeof(m_polyRenderMode));
                    break;
                case 2:
                    cload.Read(&m_fogColor, sizeof(m_fogColor));
                    break;
                case 3:
                    cload.Read(&m_fogEnabled, sizeof(m_fogEnabled));
                    break;
                case 4:
                    cload.Read(&m_fogStart, sizeof(m_fogStart));
                    break;
                case 5:
                    cload.Read(&m_fogEnd, sizeof(m_fogEnd));
                    break;
                default:
                    break;
            }
            cload.Close_Micro_Chunk();
        }
    }
    cload.Close_Chunk();
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
        } else if (m_extraPassPolyRenderMode == EXTRA_PASS_LINE) {
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
    // auto camera_position = rinfo.m_camera.Get_Position();
    // LineSegClass line{ camera_position, point };
    // Requires RayCollisionTestClass
#ifdef GAME_DLL
    return Call_Method<float, SimpleSceneClass, RenderInfoClass &, const Vector3 &>(0x00832DF0, this, rinfo, point);
#else
    return 0.0f;
#endif
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
    if (light_environment != nullptr) {
        static LightEnvironmentClass s_lightEnv;
        s_lightEnv.Reset({ 0, 0, 0 }, m_ambientLight);
        for (auto iter = m_lightList.Iterator(); iter; ++iter) {
            s_lightEnv.Add_Light(*(static_cast<LightClass *>(iter.Get_Obj())));
        }
        rinfo.m_camera.Restart();
        s_lightEnv.Pre_Render_Update(rinfo.m_camera.Get_Transform_No_Validity_Check());
        rinfo.m_lightEnvironment = &s_lightEnv;
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
        robj->Notify_Removed(this);
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
