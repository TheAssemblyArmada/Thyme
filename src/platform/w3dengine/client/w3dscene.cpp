/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Scene
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dscene.h"
#include "baseheightmap.h"
#include "camera.h"
#include "colorspace.h"
#include "coltest.h"
#include "drawable.h"
#include "dx8caps.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "light.h"
#include "object.h"
#include "particlesysmanager.h"
#include "player.h"
#include "playerlist.h"
#include "rinfo.h"
#include "sortingrenderer.h"
#include "view.h"
#include "w3ddynamiclight.h"
#include "w3dparticlesys.h"
#include "w3dshadow.h"
#include "w3dshroud.h"
#include "w3dstatuscircle.h"

ShaderClass g_playerColorShader(0x84417);

RTS3DScene::RTS3DScene() : m_drawTerrainOnly(false), m_numGlobalLights(0)
{
    Set_Name("RTS3DScene");

    for (int i = 0; i < LightEnvironmentClass::MAX_LIGHTS; i++) {
        m_globalLight[i] = nullptr;
        m_infantryLight[i] = new LightClass(LightClass::DIRECTIONAL);
    }

    m_sceneLight = new LightClass(LightClass::DIRECTIONAL);

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_shroudOn) {
        m_shroudMaterialPass = new W3DShroudMaterialPassClass();
    } else {
        m_shroudMaterialPass = nullptr;
    }
#else
    m_shroudMaterialPass = new W3DShroudMaterialPassClass();
#endif

    m_maskMaterialPass = new W3DMaskMaterialPassClass();
    m_customScenePassMode = MODE_DEFAULT;
    m_normalMatPass = new MaterialPassClass();
    m_stealthDetectedMatPass = new MaterialPassClass();
    VertexMaterialClass *mat = new VertexMaterialClass();
    mat->Set_Lighting(true);
    mat->Set_Ambient(0.0f, 0.0f, 0.0f);
    mat->Set_Diffuse(0.02f, 0.01f, 0.0f);
    mat->Set_Emissive(0.5f, 0.2f, 0.0f);
    m_normalMatPass->Set_Material(mat);
    ShaderClass shader(ShaderClass::s_presetAdditiveSolidShader);
    shader.Set_Depth_Compare(ShaderClass::PASS_EQUAL);
    m_normalMatPass->Set_Shader(shader);
    mat->Release_Ref();
    shader.Set_Depth_Compare(ShaderClass::PASS_LEQUAL);
    shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
    m_stealthDetectedMatPass->Set_Material(mat);
    m_stealthDetectedMatPass->Set_Shader(shader);
    m_translucentObjectsCount = 0;

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_maxTranslucencyObjects) {
        m_translucentObjectsBuffer = new RenderObjClass *[g_theWriteableGlobalData->m_maxTranslucencyObjects];
    } else {
        m_translucentObjectsBuffer = nullptr;
    }

    m_occludedBuildingsCount = 0;
    m_occludedObjectsCount = 0;
    m_occludedOthersCount = 0;
    m_flaggedOccludedCount = 0;
    m_occludedBuildingsBuffer = nullptr;
    m_occludedObjectsBuffer = nullptr;
    m_occludedOthersBuffer = nullptr;
    ShaderClass shader2(g_playerColorShader);
    shader2.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
    shader2.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);

    // #BUGFIX Test pointer
    if (g_theWriteableGlobalData) {
        m_occludedBuildingsBuffer = new RenderObjClass *[g_theWriteableGlobalData->m_maxOccludedBuildings];
        m_occludedObjectsBuffer = new RenderObjClass *[g_theWriteableGlobalData->m_maxOccludedObjects];
        m_occludedOthersBuffer = new RenderObjClass *[g_theWriteableGlobalData->m_maxOccludedOthers];
    } else {
        m_occludedBuildingsBuffer = new RenderObjClass *[512];
        m_occludedObjectsBuffer = new RenderObjClass *[512];
        m_occludedOthersBuffer = new RenderObjClass *[512];
    }

    for (int i = 0; i < 16; i++) {
        m_occludedMatPassesPerPlayer[i] = nullptr;
    }

    // #BUGFIX Initialize all members
    m_unk = 0;
    m_camera = nullptr;
}

RTS3DScene::~RTS3DScene()
{
    for (int i = 0; i < LightEnvironmentClass::MAX_LIGHTS; i++) {
        Ref_Ptr_Release(m_globalLight[i]);
        Ref_Ptr_Release(m_infantryLight[i]);
    }

    Ref_Ptr_Release(m_sceneLight);
    Ref_Ptr_Release(m_shroudMaterialPass);
    Ref_Ptr_Release(m_maskMaterialPass);
    Ref_Ptr_Release(m_normalMatPass);
    Ref_Ptr_Release(m_stealthDetectedMatPass);

    if (m_translucentObjectsBuffer) {
        delete[] m_translucentObjectsBuffer;
    }

    if (m_occludedOthersBuffer) {
        delete[] m_occludedOthersBuffer;
    }

    if (m_occludedObjectsBuffer) {
        delete[] m_occludedObjectsBuffer;
    }

    if (m_occludedBuildingsBuffer) {
        delete[] m_occludedBuildingsBuffer;
    }

    for (int i = 0; i < 16; i++) {
        Ref_Ptr_Release(m_occludedMatPassesPerPlayer[i]);
    }
}

void RTS3DScene::Set_Global_Light(LightClass *light, int light_index)
{
    if (m_numGlobalLights < light_index + 1) {
        m_numGlobalLights = light_index + 1;
    }

    Ref_Ptr_Set(m_globalLight[light_index], light);
}

void RTS3DScene::Flag_Occluded_Objects(CameraClass *camera)
{
    Vector3 pos = camera->Get_Position();
    LineSegClass line;
    CastResultStruct result;
    bool hit = false;
    Vector3 newEndPoint;
    result.compute_contact_point = false;
    RayCollisionTestClass tempRayTest(line, &result, COLLISION_TYPE_ALL);
    tempRayTest.m_collisionType = COLLISION_TYPE_ALL;
    m_flaggedOccludedCount = 0;
    RenderObjClass **objects = m_occludedObjectsBuffer;

    for (int i = 0; i < m_occludedObjectsCount; i++) {
        tempRayTest.m_ray.Set(pos, (*objects)->Get_Position());
        RenderObjClass **buildings = m_occludedBuildingsBuffer;

        for (int j = 0; j < m_occludedBuildingsCount; j++) {
            RenderObjClass *building = *buildings;
            const SphereClass &sphere = building->Get_Bounding_Sphere();
            PlaneClass p;
            p.N = sphere.Center - tempRayTest.m_ray.Get_P0();
            p.D = p.N * tempRayTest.m_ray.Get_Dir();

            if ((sphere.Radius * sphere.Radius) - ((p.N * p.N) - p.D * p.D) >= 0.0f && building->Cast_Ray(tempRayTest)) {
                tempRayTest.m_collidedRenderObj = building;
                hit = true;
                result.start_bad = false;
                result.fraction = 1.0f;
                break;
            }

            buildings++;
        }

        if (hit) {
            DrawableInfo *info = static_cast<DrawableInfo *>((*objects)->Get_User_Data());
            info->flags |= 1;
            m_occludedObjectsBuffer[m_flaggedOccludedCount++] = *objects;
        }

        objects++;
    }
}

bool RTS3DScene::Cast_Ray(RayCollisionTestClass &ray_test, bool test_all, int collision_type)
{
    CastResultStruct result;
    RayCollisionTestClass tempRayTest(ray_test.m_ray, &result);
    Vector3 newEndPoint;
    bool hit = false;
    tempRayTest.m_collisionType = COLLISION_TYPE_ALL;
    tempRayTest.m_checkAlpha = true;
    RefMultiListIterator<RenderObjClass> iter(&m_renderList);
    iter.First();

    while (!iter.Is_Done()) {
        RenderObjClass *robj = iter.Peek_Obj();
        iter.Next();

        if ((collision_type & robj->Get_Collision_Type()) != 0 && (test_all || robj->Is_Really_Visible())) {
            const SphereClass &sphere = robj->Get_Bounding_Sphere();
            PlaneClass p;
            p.N = sphere.Center - tempRayTest.m_ray.Get_P0();
            p.D = p.N * tempRayTest.m_ray.Get_Dir();

            if ((sphere.Radius * sphere.Radius) - ((p.N * p.N) - p.D * p.D) >= 0.0f && robj->Cast_Ray(tempRayTest)) {
                ray_test.m_collidedRenderObj = robj;
                hit = true;
                tempRayTest.m_ray.Compute_Point(tempRayTest.m_result->fraction, &newEndPoint);
                tempRayTest.m_ray.Set(ray_test.m_ray.Get_P0(), newEndPoint);
                tempRayTest.m_result->fraction = 1.0f;
            }
        }
    }

    ray_test.m_ray = tempRayTest.m_ray;
    return hit;
}

void RTS3DScene::Visibility_Check(CameraClass *camera)
{
    Drawable::Friend_Lock_Dirty_Stuff_For_Iteration();
    RefMultiListIterator<RenderObjClass> iter(&m_renderList);
    DrawableInfo *info = nullptr;
    Drawable *drawable = nullptr;
    m_occludedBuildingsCount = 0;
    m_occludedObjectsCount = 0;
    m_translucentObjectsCount = 0;
    m_occludedOthersCount = 0;
    unsigned int frame = 0;

    if (g_theGameLogic) {
        frame = g_theGameLogic->Get_Frame();
    }

    if (frame <= g_theWriteableGlobalData->m_defaultOcclusionDelay) {
        frame = g_theWriteableGlobalData->m_defaultOcclusionDelay + 1;
    }

    if (!ShaderClass::Is_Backface_Culling_Inverted()) {
        for (iter.First(); !iter.Is_Done(); iter.Next()) {
            RenderObjClass *robj = iter.Peek_Obj();

            if (robj->Is_Force_Visible()) {
                robj->Set_Visible(true);
                continue;
            }

            if (robj->Is_Hidden()) {
                robj->Set_Visible(false);
                continue;
            }

            bool cull = camera->Cull_Sphere(robj->Get_Bounding_Sphere());
            bool visible = !cull;

            if (cull) {
                robj->Set_Visible(visible);
                continue;
            }

            info = static_cast<DrawableInfo *>(robj->Get_User_Data());

            if (!info) {
                robj->Set_Visible(visible);
                continue;
            }

            drawable = info->drawable;

            if (!drawable) {
                robj->Set_Visible(visible);
                continue;
            }

            if (drawable->Is_Hidden() || drawable->Is_Fully_Obscured_By_Shroud()) {
                visible = false;
                robj->Set_Visible(false);
            }

            info->flags = 0;

            if (visible) {
                if (drawable->Get_Alpha_Override() != 1.0f
                    && m_translucentObjectsCount < g_theWriteableGlobalData->m_maxTranslucencyObjects) {
                    info->flags |= 8;
                    m_translucentObjectsBuffer[m_translucentObjectsCount++] = robj;
                }

                if (g_theWriteableGlobalData->m_useBehindBuildingMarker) {
                    if (g_theGameLogic->Get_Occlusion_Enabled()) {
                        if (drawable->Is_KindOf(KINDOF_STRUCTURE)
                            && m_occludedBuildingsCount < g_theWriteableGlobalData->m_maxOccludedBuildings) {

                            if (info->flags != 8) {
                                m_occludedBuildingsBuffer[m_occludedBuildingsCount++] = robj;
                            }

                            info->flags |= 2;
                        } else if (drawable->Get_Object()
                            && (drawable->Is_KindOf(KINDOF_SCORE) || drawable->Is_KindOf(KINDOF_SCORE_CREATE)
                                || drawable->Is_KindOf(KINDOF_SCORE_DESTROY)
                                || drawable->Is_KindOf(KINDOF_MP_COUNT_FOR_VICTORY))
                            && drawable->Get_Object()->Get_Occlusion_Delay_Frame() <= frame
                            && m_occludedObjectsCount < g_theWriteableGlobalData->m_maxOccludedObjects) {
                            m_occludedObjectsBuffer[m_occludedObjectsCount++] = robj;
                            info->flags |= 4;
                        } else if (!info->flags && m_occludedOthersCount < g_theWriteableGlobalData->m_maxOccludedOthers) {
                            if (info->flags != 8) {
                                m_occludedOthersBuffer[m_occludedOthersCount++] = robj;
                            }

                            info->flags |= 16;
                        }
                    }
                }

                robj->Set_Visible(visible);
            }
        }
    } else {
        for (iter.First(); !iter.Is_Done(); iter.Next()) {
            RenderObjClass *robj = iter.Peek_Obj();
            drawable = nullptr;
            info = static_cast<DrawableInfo *>(robj->Get_User_Data());

            if (info) {
                drawable = info->drawable;
            }

            if (drawable) {
                if (robj->Is_Force_Visible()) {
                    robj->Set_Visible(true);
                } else {
                    int visible = 0;

                    if (drawable->Get_Draws_In_Mirror()) {
                        if (!camera->Cull_Sphere(robj->Get_Bounding_Sphere())) {
                            visible = 1;
                        }
                    }

                    robj->Set_Visible(visible);
                }
            } else if (robj->Is_Force_Visible()) {
                robj->Set_Visible(true);
            } else {
                robj->Set_Visible(!camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
            }
        }
    }

    m_visibilityChecked = true;
    Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration();
}

void RTS3DScene::Render_Specific_Drawables(RenderInfoClass &rinfo, int num_drawables, Drawable **drawables)
{
    Drawable::Friend_Lock_Dirty_Stuff_For_Iteration();
    int index;

    if (g_thePlayerList) {
        index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
    } else {
        index = 0;
    }

    RefMultiListIterator<RenderObjClass> iter(&m_renderList);

    for (iter.First(); !iter.Is_Done(); iter.Next()) {
        RenderObjClass *robj = iter.Peek_Obj();
        DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());
        Drawable *drawable = nullptr;

        if (info) {
            drawable = info->drawable;
        }

        if (drawable) {
            bool render = false;

            for (int i = 0; i < num_drawables; i++) {
                if (drawables[i] == drawable) {
                    render = 1;
                    break;
                }
            }

            if (render) {
                Render_One_Object(rinfo, robj, index);
            }
        }
    }

    Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration();
}

void RTS3DScene::Render_One_Object(RenderInfoClass &rinfo, RenderObjClass *robj, int local_player_index)
{
    Drawable *drawable = nullptr;
    bool hidden = false;
    const Object *object = nullptr;
    ObjectShroudStatus shrouded = SHROUDED_INVALID;
    bool popmaterialpass = false;
    bool popoverrideflags = false;
    LightClass **light = m_globalLight;

    if (robj->Class_ID() == RenderObjClass::CLASSID_TERRAINTRACKS) {
        robj->Render(rinfo);
        return;
    }

    LightEnvironmentClass lenv;
    const SphereClass &sphere = robj->Get_Bounding_Sphere();
    DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());

    if (info) {
        drawable = info->drawable;

        if (!drawable) {
            shrouded = SHROUDED_SEEN;
        }
    }

    Vector3 ambient(Get_Ambient_Light());
    if (!drawable || (hidden = drawable->Is_Hidden())) {
        if (hidden) {
            return;
        }

        if (shrouded == SHROUDED_SEEN) {
            rinfo.m_lightEnvironment = &m_sceneLightEnv;
            robj->Render(rinfo);
            rinfo.m_lightEnvironment = nullptr;
            return;
        }

        lenv.Reset(sphere.Center, ambient);

        for (int i = 0; i < m_numGlobalLights; i++) {
            lenv.Add_Light(*m_globalLight[i]);
        }
    } else {
        object = drawable->Get_Object();

        if (object) {
            shrouded = object->Get_Shrouded_Status(local_player_index);

            if (shrouded == SHROUDED_NONE) {
                drawable->Set_Remain_Visible_Frames(g_theGameLogic->Get_Frame());
            } else if (shrouded >= SHROUDED_SEEN) {
                if (drawable->Get_Remain_Visible_Frames()) {
                    unsigned int frames = 60;

                    if (object->Is_Effectively_Dead()) {
                        frames += 90;
                    }

                    if (g_theGameLogic->Get_Frame() < drawable->Get_Remain_Visible_Frames() + frames) {
                        shrouded = SHROUDED_TRANSITION;
                    }
                }
            }

            if (!robj->Peek_Scene()) {
                return;
            }
        } else {
            shrouded = SHROUDED_NONE;

            if (info->object_id) {
                object = g_theGameLogic->Find_Object_By_ID(info->object_id);
                if (object) {
                    if (object->Get_Shrouded_Status(local_player_index) >= SHROUDED_SEEN) {
                        shrouded = SHROUDED_NEVERSEEN;
                    }
                }
            }
        }

        if (drawable->Is_KindOf(KINDOF_INFANTRY)) {
            light = m_infantryLight;
        }

        lenv.Reset(sphere.Center, ambient);
        const Vector3 *tint = drawable->Get_Tint_Color();
        const Vector3 *selection = drawable->Get_Selection_Color();

        if (tint || selection) {
            Vector3 v1;
            Vector3 v2;
            Vector3 v3;
            v1.Set(0.0f, 0.0f, 0.0f);

            if (tint) {
                Vector3::Add(v1, *tint, &v1);
            }

            if (selection) {
                Vector3::Add(v1, *selection, &v1);
            }

            for (int i = 0; i < m_numGlobalLights; i++) {
                light[i]->Get_Diffuse(&v2);
                v3 = v2;
                v2 = v1 + v2;
                light[i]->Set_Diffuse(v2);
                lenv.Add_Light(*light[i]);
                light[i]->Set_Diffuse(v3);
            }

            v2 = lenv.Get_Equivalent_Ambient();
            Vector3::Add(v1, v2, &v2);
            lenv.Set_Equivalent_Ambient(v2);
        } else {
            for (int i = 0; i < m_numGlobalLights; i++) {
                lenv.Add_Light(*light[i]);
            }
        }

        if (drawable->Get_Stealth_Emissive_Scale() != 0.0f) {
            rinfo.m_emissiveScale = drawable->Get_Stealth_Emissive_Scale();

            if (drawable->Get_Stealth_Look() == STEALTHLOOK_VISIBLE_DETECTED) {
                rinfo.m_emissiveScale = drawable->Get_Stealth_Emissive_Scale();
                rinfo.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
                rinfo.Push_Material_Pass(m_stealthDetectedMatPass);
                popoverrideflags = true;
            } else {
                rinfo.m_emissiveScale = drawable->Get_Stealth_Emissive_Scale();
                rinfo.Push_Material_Pass(m_normalMatPass);
            }

            popmaterialpass = true;
        }
    }

    if (!hidden) {
        RefMultiListIterator<RenderObjClass> iter(&m_lightList);

        for (iter.First(); !iter.Is_Done(); iter.Next()) {
            LightClass *light_class = static_cast<LightClass *>(iter.Peek_Obj());
            SphereClass s(light_class->Get_Bounding_Sphere());

            if (light_class->Get_Type() != LightClass::POINT || Spheres_Intersect(sphere, s)) {
                lenv.Add_Light(*light_class);
            }
        }

        if (drawable) {
            if (drawable->Recieves_Dynamic_Lights()) {
                RefMultiListIterator<RenderObjClass> iter2(&m_dynamicLightList);

                for (iter2.First(); !iter2.Is_Done(); iter2.Next()) {
                    W3DDynamicLight *dyn_light = static_cast<W3DDynamicLight *>(iter2.Peek_Obj());

                    if (dyn_light->Is_Enabled()) {
                        SphereClass s(dyn_light->Get_Bounding_Sphere());

                        if (dyn_light->Get_Type() != LightClass::POINT || Spheres_Intersect(sphere, s)) {
                            LightClass *light_class = static_cast<LightClass *>(iter2.Peek_Obj());
                            lenv.Add_Light(*light_class);
                        }
                    }
                }
            }
        }

        lenv.Pre_Render_Update(rinfo.m_camera.Get_Transform());
        rinfo.m_lightEnvironment = &lenv;

        if (info) {
#ifdef GAME_DEBUG_STRUCTS
            if (!g_theWriteableGlobalData->m_shroudOn) {
                shrouded = SHROUDED_NONE;
            }
#endif
            if (m_customScenePassMode == MODE_DEFAULT) {
                if (shrouded <= SHROUDED_NONE) {
                    robj->Render(rinfo);
                } else {
                    rinfo.Push_Material_Pass(m_shroudMaterialPass);
                    robj->Render(rinfo);
                    rinfo.Pop_Material_Pass();
                }
            } else if (m_maskMaterialPass) {
                rinfo.Push_Material_Pass(m_maskMaterialPass);
                rinfo.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
                robj->Render(rinfo);
                rinfo.Pop_Override_Flags();
                rinfo.Pop_Material_Pass();
            }
        } else {
            robj->Render(rinfo);
        }
    }

    rinfo.m_lightEnvironment = nullptr;

    if (popmaterialpass) {
        rinfo.Pop_Material_Pass();
    }

    if (popoverrideflags) {
        rinfo.Pop_Override_Flags();
    }
}

void RTS3DScene::Flush(RenderInfoClass &rinfo)
{
    if (m_customScenePassMode == MODE_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        Do_Shadows(rinfo, false);
    }

    g_theDX8MeshRenderer.Flush();

    if (DX8Wrapper::Has_Stencil()) {
        Flush_Occluded_Objects_Into_Stencil(rinfo);
    }

    Do_Trees(rinfo);

    if (m_customScenePassMode == MODE_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        Do_Shadows(rinfo, true);
    }

    W3D::Render_And_Clear_Static_Sort_Lists(rinfo);

    if (m_customScenePassMode == MODE_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        flush_Translucent_Objects(rinfo);
    }

    if (m_customScenePassMode == MODE_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        Do_Particles(rinfo);
    }

    SortingRendererClass::Flush();
    g_theDX8MeshRenderer.Clear_Pending_Delete_Lists();
}

void RTS3DScene::Update_Fixed_Light_Environments(RenderInfoClass &rinfo)
{
    float f1 = (float)g_theWriteableGlobalData->m_fogAlpha / (float)g_theWriteableGlobalData->m_clearAlpha;
    float f2;

    if (g_theWriteableGlobalData->m_infantryLightOverride != -1.0f) {
        f2 = g_theWriteableGlobalData->m_infantryLightOverride;
    } else {
        f2 = g_theWriteableGlobalData->m_infantryLight[g_theWriteableGlobalData->m_timeOfDay];
    }

    m_globalLightEnv.Reset(Vector3(0.0f, 0.0f, 0.0f), Get_Ambient_Light());
    m_sceneLightEnv.Reset(Vector3(0.0f, 0.0f, 0.0f), Get_Ambient_Light() * f1);
    Vector3 diffuse;
    Vector3 ambient;

    for (int i = 0; i < m_numGlobalLights; i++) {

        m_globalLightEnv.Add_Light(*m_globalLight[i]);

        *m_infantryLight[i] = *m_globalLight[i];

        m_infantryLight[i]->Set_Transform(m_globalLight[i]->Get_Transform());

        m_globalLight[i]->Get_Diffuse(&diffuse);
        m_globalLight[i]->Get_Ambient(&ambient);

        diffuse *= f2;
        ambient *= f2;

        static Vector3 id(1.0f, 1.0f, 1.0f);
        diffuse.Cap_Absolute_To(id);
        ambient.Cap_Absolute_To(id);

        m_infantryLight[i]->Set_Ambient(ambient);
        m_infantryLight[i]->Set_Diffuse(diffuse);

        m_sceneLight->Set_Transform(m_globalLight[i]->Get_Transform());

        m_globalLight[i]->Get_Diffuse(&diffuse);
        m_sceneLight->Set_Diffuse(diffuse * f1);

        m_globalLight[i]->Get_Ambient(&ambient);
        m_sceneLight->Set_Ambient(ambient * f1);

        m_sceneLightEnv.Add_Light(*m_sceneLight);
    }

    m_globalLightEnv.Pre_Render_Update(rinfo.m_camera.Get_Transform());
    m_sceneLightEnv.Pre_Render_Update(rinfo.m_camera.Get_Transform());
    m_ambient = Get_Ambient_Light();
}
void RTS3DScene::Update_Player_Color_Passes() {}

void RTS3DScene::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Fog(m_fogEnabled, m_fogColor, m_fogStart, m_fogEnd);
    g_theWriteableGlobalData->m_useBehindBuildingMarker =
        g_theWriteableGlobalData->m_useBehindBuildingMarker && DX8Wrapper::Has_Stencil();

    if (Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        if (m_customScenePassMode == MODE_DEFAULT) {
            Update_Player_Color_Passes();
            Update_Fixed_Light_Environments(rinfo);
            Customized_Render(rinfo);
            Flush(rinfo);
        } else if (m_customScenePassMode == MODE_MASK) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 8);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
            m_maskMaterialPass->Reset_Shader(false);
            Customized_Render(rinfo);
            Flush(rinfo);
            m_maskMaterialPass->Reset_Shader(true);
            m_maskMaterialPass->UnInstall_Materials();
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
            ShaderClass::Invalidate();
        }
    } else {
        bool texturing = W3D::Is_Texturing_Enabled();

        if (SceneClass::Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_CLEAR_LINE) {
            DX8Wrapper::Clear(true, false, Vector3(0.0f, 0.0f, 0.0f), 1.0f);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 8);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
            m_customScenePassMode = MODE_MASK;
            m_maskMaterialPass->Reset_Shader(false);
            Customized_Render(rinfo);
            Flush(rinfo);
            m_maskMaterialPass->Reset_Shader(true);
            m_maskMaterialPass->UnInstall_Materials();

            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
            W3D::Enable_Coloring(0xFF008000);
            W3D::Enable_Texturing(false);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

            float zmin;
            float zmax;
            rinfo.m_camera.Get_Depth_Range(&zmin, &zmax);
            rinfo.m_camera.Set_Depth_Range(zmin, zmax - GAMEMATH_EPSILON);
            rinfo.m_camera.Apply();
            Customized_Render(rinfo);
            Flush(rinfo);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID);
            rinfo.m_camera.Set_Depth_Range(zmin, zmax);
            rinfo.m_camera.Apply();
            W3D::Enable_Texturing(texturing);
            W3D::Enable_Coloring(0);
            ShaderClass::Invalidate();
        } else {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
            Customized_Render(rinfo);
            Flush(rinfo);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);

            if (SceneClass::Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_LINE) {
                W3D::Enable_Texturing(false);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7);
                Customized_Render(rinfo);
            } else if (SceneClass::Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_CLEAR_LINE) {
                DX8Wrapper::Clear(true, false, Vector3(0.0f, 0.0f, 0.0f), 0.0f);
                W3D::Enable_Texturing(false);
                W3D::Enable_Coloring(0xFF008000);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7);
                Customized_Render(rinfo);
            }

            Flush(rinfo);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
            W3D::Enable_Texturing(texturing);
            W3D::Enable_Coloring(0);
            ShaderClass::Invalidate();
        }
    }
#endif
}

void RTS3DScene::Customized_Render(RenderInfoClass &rinfo)
{
    Drawable::Friend_Lock_Dirty_Stuff_For_Iteration();
    RenderObjClass *robj = nullptr;
    m_translucentObjectsCount = 0;
    m_flaggedOccludedCount = 0;
    int index;

    if (g_thePlayerList) {
        index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
    } else {
        index = 0;
    }

    if (!m_visibilityChecked) {
        Visibility_Check(&rinfo.m_camera);
    }

    m_visibilityChecked = false;
    RefMultiListIterator<RenderObjClass> iter(&m_updateList);

    for (iter.First(); !iter.Is_Done(); iter.Next()) {
        RenderObjClass *r = iter.Peek_Obj();

        if (r->Class_ID() == RenderObjClass::CLASSID_HEIGHTMAP) {
            robj = r;
        }

        if (!ShaderClass::Is_Backface_Culling_Inverted()) {
            iter.Peek_Obj()->On_Frame_Update();
        }
    }

    if (robj) {
        rinfo.m_lightEnvironment = nullptr;
        rinfo.m_camera.Set_User_Data(this, false);

        if (m_customScenePassMode == MODE_DEFAULT && m_shroudMaterialPass) {
            rinfo.Push_Material_Pass(m_shroudMaterialPass);
            robj->Render(rinfo);
            rinfo.Pop_Material_Pass();
        } else {
            if (m_customScenePassMode == MODE_MASK && m_maskMaterialPass) {
                rinfo.Push_Material_Pass(m_maskMaterialPass);
                robj->Render(rinfo);
                rinfo.Pop_Material_Pass();
            } else {
                robj->Render(rinfo);
            }
        }
    }

    if (!m_drawTerrainOnly) {
        RefMultiListIterator<RenderObjClass> iter2(&m_renderList);

        while (!iter2.Is_Done()) {
            RenderObjClass *r = iter2.Peek_Obj();
            iter2.Next();

            if (r->Class_ID() != RenderObjClass::CLASSID_HEIGHTMAP) {
                if (r->Is_Really_Visible()) {
                    DrawableInfo *info = static_cast<DrawableInfo *>(r->Get_User_Data());
                    Drawable *drawable = nullptr;

                    if (info) {
                        drawable = info->drawable;
                    }

                    if (!drawable || (info->flags & 30) == 0) {
                        Render_One_Object(rinfo, r, index);
                    }
                }
            }
        }

        if (g_theW3DShadowManager) {
            if (robj) {
                if (!ShaderClass::Is_Backface_Culling_Inverted() && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
                    g_theW3DShadowManager->Set_Is_Shadow_Scene(true);
                }
            }
        }

        if (robj) {
            if (g_theParticleSystemManager) {
                if (Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
                    g_theParticleSystemManager->Queue_Particle_Render();
                }
            }
        }
    }

    Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration();
}

int Player_Index_To_Color_Index(int player_index)
{
    int index = 0;

    for (int i = 0; i < 4; i++) {
        int i1 = 3 - i;
        int i2;

        if (3 - i <= i) {
            i2 = ((1 << i) & player_index) >> (i - i1);
        } else {
            i2 = ((1 << i) & player_index) << (i1 - i);
        }

        index |= i2;
    }

    return index;
}

void Render_Stenciled_Player_Color(unsigned int color, unsigned int reference, bool b)
{
#ifdef BUILD_WITH_D3D8
    struct StencilVertex
    {
        Vector4 vert;
        int color;
    };

    StencilVertex vertices[4];
    int x;
    int y;
    g_theTacticalView->Get_Origin(&x, &y);
    int width = g_theTacticalView->Get_Width();
    int height = g_theTacticalView->Get_Height();

    vertices[0].vert.Set((float)(width + x), (float)(height + y), 0.0f, 1.0f);
    vertices[1].vert.Set((float)(width + x), 0.0f, 0.0f, 1.0f);
    vertices[2].vert.Set((float)x, (float)(height + y), 0.0f, 1.0f);
    vertices[3].vert.Set((float)x, 0.0f, 0.0f, 1.0f);
    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    DX8Wrapper::Set_Shader(g_playerColorShader);
    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);
    DX8Wrapper::Apply_Render_State_Changes();
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    if (device) {
        device->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
        DWORD colorwrite = 0x12345678;

        if (b) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, 0x80808080);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILMASK, g_theW3DShadowManager->Get_Stencil_Mask());
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_LESS);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DCMP_LESS);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_NEVER);

            if ((DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps().PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) != 0) {
                DX8Wrapper::Get_D3D_Device8()->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0);
            } else {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ONE);
            }
        } else {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, reference);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILMASK, 0xFFFFFFFF);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }

        if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
            device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(StencilVertex));
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ONE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);

        if (colorwrite != 0x12345678) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, colorwrite);
        }
    }
#endif
}

void RTS3DScene::Flush_Occluded_Objects_Into_Stencil(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    Vector3 hsv;
    Vector3 rgb;
    int i1 = 1;
    int i2 = 0;
    RenderObjClass *objects[16][512];
    RenderObjClass **objectptrs[16];
    unsigned int references[16];
    int colors[16];

    for (int i = 0; i < 16; i++) {
        objectptrs[i] = objects[i];
        references[i] = 0xFFFFFFFF;
    }

    g_theW3DShadowManager->Set_Stencil_Mask(0);
    int index;

    if (g_thePlayerList) {
        index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
    } else {
        index = 0;
    }

    if (m_occludedObjectsCount && m_occludedBuildingsCount) {
        for (int i = 0; i < m_occludedObjectsCount; i++) {
            RenderObjClass *robj = m_occludedObjectsBuffer[i];
            DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());
            int playerindex = info->drawable->Get_Object()->Get_Controlling_Player()->Get_Player_Index();

            if (objectptrs[playerindex] - objects[playerindex] < 512) {
                *objectptrs[playerindex]++ = robj;
            } else {
                captainslog_debug("Exceeded Maximum Number of potentially occluded models");
            }
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILMASK, 0xFFFFFFFF);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

        for (int i = 0; i < 16; i++) {
            int count = objectptrs[i] - objects[i];

            if (count) {
                if (references[i] == 0xFFFFFFFF) {
                    references[i] = Player_Index_To_Color_Index(i1++);
                    DrawableInfo *info = static_cast<DrawableInfo *>(objects[i][0]->Get_User_Data());
                    int color = info->drawable->Get_Object()->Get_Controlling_Player()->Get_Color();
                    float blue = (color & 0xFF) / 255.0f;
                    float green = ((color >> 8) & 0xFF) / 255.0f;
                    float red = ((color >> 16) & 0xFF) / 255.0f;
                    RGB_To_HSV(hsv, Vector3(red, green, blue));
                    hsv.Z *= g_theWriteableGlobalData->m_occludedColorLuminanceScale;
                    HSV_To_RGB(rgb, hsv);
                    colors[i2++] = DX8Wrapper::Convert_Color(rgb, 0.5f);
                }

                DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, 8 * references[i]);
                RenderObjClass **o = objects[i];

                for (int j = 0; j < count; j++) {
                    DrawableInfo *info = static_cast<DrawableInfo *>((*o)->Get_User_Data());
                    if ((info->flags & 8) != 0) {
                        g_theDX8MeshRenderer.Flush();
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_NEVER);
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE);
                        Render_One_Object(rinfo, *o, index);
                        g_theDX8MeshRenderer.Flush();
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
                    } else {
                        Render_One_Object(rinfo, *o, index);
                    }

                    o++;
                }

                g_theDX8MeshRenderer.Flush();
            }
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);

        RenderObjClass **o = m_occludedOthersBuffer;

        for (int i = 0; i < m_occludedOthersCount; i++) {
            Render_One_Object(rinfo, *o, index);
            o++;
        }

        g_theDX8MeshRenderer.Flush();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, 0xFFFFFFFF);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILMASK, 0xFFFFFFFF);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0x80);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
        o = m_occludedBuildingsBuffer;

        for (int i = 0; i < m_occludedBuildingsCount; i++) {
            Render_One_Object(rinfo, *o, index);
            o++;
        }

        g_theDX8MeshRenderer.Flush();
        int mask = 0;

        for (int i = 0; i < i2; i++) {
            int color = colors[i];
            int reference = (8 * Player_Index_To_Color_Index(i + 1)) | 0x80;
            Render_Stenciled_Player_Color(color, reference, false);
            mask |= reference;
        }

        g_theW3DShadowManager->Set_Stencil_Mask(mask);

        if (i2 >= 8 && g_theWriteableGlobalData->m_shadowVolumes) {
            Render_Stenciled_Player_Color(0, 0, true);
            g_theW3DShadowManager->Set_Stencil_Mask(0x80808080);
        }

        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
    } else if (m_occludedOthersCount || m_occludedBuildingsCount || m_occludedObjectsCount) {
        RenderObjClass **o = m_occludedObjectsBuffer;

        for (int i = 0; i < m_occludedObjectsCount; i++) {
            Render_One_Object(rinfo, *o, index);
            o++;
        }

        o = m_occludedBuildingsBuffer;

        for (int i = 0; i < m_occludedBuildingsCount; i++) {
            Render_One_Object(rinfo, *o, index);
            o++;
        }

        o = m_occludedOthersBuffer;

        for (int i = 0; i < m_occludedOthersCount; i++) {
            Render_One_Object(rinfo, *o, index);
            o++;
        }

        g_theDX8MeshRenderer.Flush();
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENT, DX8Wrapper::Convert_Color(Get_Ambient_Light(), 0.0f));
#endif
}

void RTS3DScene::Flush_Occluded_Objects(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    g_theW3DShadowManager->Set_Stencil_Mask(0);

    if (m_flaggedOccludedCount) {
        int index;

        if (g_thePlayerList) {
            index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            index = 0;
        }

        if (DX8Wrapper::Has_Stencil()) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, TRUE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, 0x80);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILMASK, 0xFFFFFFFF);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
        }

        rinfo.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);

        for (int i = 0; i < m_flaggedOccludedCount; i++) {
            RenderObjClass *robj = m_occludedObjectsBuffer[i];
            DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());
            int player_index = info->drawable->Get_Object()->Get_Controlling_Player()->Get_Player_Index();
            rinfo.Push_Material_Pass(m_occludedMatPassesPerPlayer[player_index]);
            robj->Render(rinfo);
            rinfo.Pop_Material_Pass();
        }

        rinfo.Pop_Override_Flags();
        g_theDX8MeshRenderer.Flush();

        if (DX8Wrapper::Has_Stencil()) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILREF, 0);
        }

        for (int i = 0; i < m_flaggedOccludedCount; i++) {
            Render_One_Object(rinfo, m_occludedObjectsBuffer[i], index);
        }

        g_theDX8MeshRenderer.Flush();
        m_flaggedOccludedCount = 0;
        DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
        g_theW3DShadowManager->Set_Stencil_Mask(0x80808080);
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENT, DX8Wrapper::Convert_Color(Get_Ambient_Light(), 0.0f));
#endif
}

void RTS3DScene::flush_Translucent_Objects(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (m_translucentObjectsCount) {
        int index;

        if (g_thePlayerList) {
            index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            index = 0;
        }

        for (int i = 0; i < m_translucentObjectsCount; i++) {
            RenderObjClass *robj = m_translucentObjectsBuffer[i];
            DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());
            rinfo.m_alphaOverride = info->drawable->Get_Alpha_Override();
            Render_One_Object(rinfo, robj, index);
        }

        g_theDX8MeshRenderer.Flush();
        W3D::Render_And_Clear_Static_Sort_Lists(rinfo);
        rinfo.m_alphaOverride = 1.0f;
        m_translucentObjectsCount = 0;
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENT, DX8Wrapper::Convert_Color(Get_Ambient_Light(), 0.0f));
#endif
}

RefMultiListIterator<RenderObjClass> *RTS3DScene::Create_Lights_Iterator()
{
    return new RefMultiListIterator<RenderObjClass>(&m_lightList);
}

void RTS3DScene::Destroy_Lights_Iterator(RefMultiListIterator<RenderObjClass> *it)
{
    delete it;
}

void RTS3DScene::Add_Dynamic_Light(W3DDynamicLight *obj)
{
    m_dynamicLightList.Add(obj);
    m_updateList.Add(obj);
}

W3DDynamicLight *RTS3DScene::Get_A_Dynamic_Light()
{
    RefMultiListIterator<RenderObjClass> iter(&m_dynamicLightList);

    for (iter.First(); !iter.Is_Done(); iter.Next()) {
        W3DDynamicLight *l = static_cast<W3DDynamicLight *>(iter.Peek_Obj());

        if (!l->Is_Enabled()) {
            l->Set_Enabled(true);
            return l;
        }
    }

    W3DDynamicLight *l = new W3DDynamicLight();
    Add_Dynamic_Light(l);
    l->Release_Ref();
    l->Set_Enabled(true);
    return l;
}

void RTS3DScene::Remove_Dynamic_Light(W3DDynamicLight *obj)
{
    m_dynamicLightList.Remove(obj);
}

void RTS3DScene::Do_Render(CameraClass *camera)
{
    m_camera = camera;
    // Calls some debug stuff in WB
    Draw();
    m_camera = nullptr;
}

void RTS3DScene::Draw()
{
    if (m_camera) {
        W3D::Render(this, m_camera);
    } else {
        captainslog_dbgassert(m_camera, "Null m_camera in RTS3DScene::draw");
    }
}

RTS2DScene::RTS2DScene()
{
    Set_Name("RTS2DScene");
    m_status = new W3DStatusCircle();
    Add_Render_Object(m_status);
    // #BUGFIX Initialize all members
    m_camera = nullptr;
}

RTS2DScene::~RTS2DScene()
{
    Remove_Render_Object(m_status);
    Ref_Ptr_Release(m_status);
}

void RTS2DScene::Customized_Render(RenderInfoClass &rinfo)
{
    SimpleSceneClass::Customized_Render(rinfo);
}

void RTS2DScene::Draw()
{
    if (m_camera) {
        W3D::Render(this, m_camera);
    } else {
        captainslog_dbgassert(m_camera, "Null m_camera in RTS2DScene::draw");
    }
}

void RTS2DScene::Do_Render(CameraClass *camera)
{
    m_camera = camera;
    // Calls some debug stuff in WB
    Draw();
    m_camera = nullptr;
}

RTS3DInterfaceScene::RTS3DInterfaceScene() {}

RTS3DInterfaceScene::~RTS3DInterfaceScene() {}

void RTS3DInterfaceScene::Customized_Render(RenderInfoClass &rinfo)
{
    SimpleSceneClass::Customized_Render(rinfo);
}
