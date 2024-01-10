/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dshadow.h"
#include "camera.h"
#ifdef BUILD_WITH_D3D8
#include "dx8wrapper.h"
#endif
#include "frustum.h"
#include "globaldata.h"
#include "rinfo.h"
#include "w3dprojectedshadow.h"
#include "w3dvolumetricshadow.h"

Vector3 g_lightPosWorld[1] = { Vector3(94.016098f, 50.499001f, 200.0f) };

#ifndef GAME_DLL
const FrustumClass *g_shadowCameraFrustum;
W3DShadowManager *g_theW3DShadowManager = nullptr;
#endif

void Do_Shadows(RenderInfoClass &rinfo, bool stencil_pass)
{
    g_shadowCameraFrustum = &rinfo.m_camera.Get_Frustum();
    int shadow_count = 0;

    if (!stencil_pass && g_theW3DProjectedShadowManager != nullptr && g_theW3DShadowManager->Get_Is_Shadow_Scene()) {
        shadow_count = g_theW3DProjectedShadowManager->Render_Shadows(rinfo);
    }

    if (stencil_pass && g_theW3DVolumetricShadowManager != nullptr && g_theW3DShadowManager->Get_Is_Shadow_Scene()) {
        g_theW3DVolumetricShadowManager->Render_Shadows(shadow_count != 0);
    }

    if (g_theW3DShadowManager != nullptr) {
        if (stencil_pass) {
            g_theW3DShadowManager->Set_Is_Shadow_Scene(false);
        }
    }
}

W3DShadowManager::W3DShadowManager() : m_isShadowScene(false), m_shadowColor(0x7FA0A0A0), m_stencilMask(0)
{
    captainslog_dbgassert(g_theW3DVolumetricShadowManager == nullptr && g_theW3DProjectedShadowManager == nullptr,
        "Creating new shadow managers without deleting old ones");
    Vector3 light_pos(-g_theWriteableGlobalData->m_terrainLightPos[0].x,
        -g_theWriteableGlobalData->m_terrainLightPos[0].y,
        -g_theWriteableGlobalData->m_terrainLightPos[0].z);
    light_pos.Normalize();
    g_lightPosWorld[0] = light_pos * 10000.0f;
    g_theW3DVolumetricShadowManager = new W3DVolumetricShadowManager();
    g_theW3DProjectedShadowManager = new W3DProjectedShadowManager();
    g_theProjectedShadowManager = g_theW3DProjectedShadowManager;
}

W3DShadowManager::~W3DShadowManager()
{
    if (g_theW3DVolumetricShadowManager != nullptr) {
        delete g_theW3DVolumetricShadowManager;
        g_theW3DVolumetricShadowManager = nullptr;
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        delete g_theW3DProjectedShadowManager;
        g_theW3DProjectedShadowManager = nullptr;
        g_theProjectedShadowManager = nullptr;
    }
}

bool W3DShadowManager::Init()
{
    if (g_theW3DVolumetricShadowManager != nullptr && g_theW3DVolumetricShadowManager->Init()) {
        g_theW3DVolumetricShadowManager->Re_Acquire_Resources();
    }

    if (g_theW3DProjectedShadowManager != nullptr && g_theW3DProjectedShadowManager->Init()) {
        g_theW3DProjectedShadowManager->Re_Acquire_Resources();
    }

    return true;
}

void W3DShadowManager::Reset()
{
    if (g_theW3DVolumetricShadowManager != nullptr) {
        g_theW3DVolumetricShadowManager->Reset();
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        g_theW3DProjectedShadowManager->Reset();
    }
}

bool W3DShadowManager::Re_Acquire_Resources()
{
    bool ret = true;

    if (g_theW3DVolumetricShadowManager != nullptr && !g_theW3DVolumetricShadowManager->Re_Acquire_Resources()) {
        ret = false;
    }

    if (g_theW3DProjectedShadowManager != nullptr && !g_theW3DProjectedShadowManager->Re_Acquire_Resources()) {
        ret = false;
    }

    return ret;
}

void W3DShadowManager::Release_Resources()
{
    if (g_theW3DVolumetricShadowManager != nullptr) {
        g_theW3DVolumetricShadowManager->Release_Resources();
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        g_theW3DProjectedShadowManager->Release_Resources();
    }
}

Shadow *W3DShadowManager::Add_Shadow(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable)
{
    ShadowType type = SHADOW_VOLUME;

    if (shadow_info != nullptr) {
        type = shadow_info->m_type;
    }

    if (type != SHADOW_DECAL) {
        if (type == SHADOW_VOLUME) {
            if (g_theW3DVolumetricShadowManager != nullptr) {
                return g_theW3DVolumetricShadowManager->Add_Shadow(robj, shadow_info, drawable);
            }

            return nullptr;
        }

        if (type != SHADOW_PROJECTION) {
            return nullptr;
        }
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        return g_theW3DProjectedShadowManager->Add_Shadow(robj, shadow_info, drawable);
    }

    return nullptr;
}

void W3DShadowManager::Remove_Shadow(Shadow *shadow)
{
    shadow->Release();
}

void W3DShadowManager::Remove_All_Shadows()
{
    if (g_theW3DVolumetricShadowManager != nullptr) {
        g_theW3DVolumetricShadowManager->Remove_All_Shadows();
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        g_theW3DProjectedShadowManager->Remove_All_Shadows();
    }
}

void W3DShadowManager::Invalidate_Cached_Light_Positions()
{
    if (g_theW3DVolumetricShadowManager != nullptr) {
        g_theW3DVolumetricShadowManager->Invalidate_Cached_Light_Positions();
    }

    if (g_theW3DProjectedShadowManager != nullptr) {
        g_theW3DProjectedShadowManager->Invalidate_Cached_Light_Positions();
    }
}

Vector3 &W3DShadowManager::Get_Light_Pos_World(int light_index)
{
    return g_lightPosWorld[light_index];
}

void W3DShadowManager::Set_Light_Position(int light_index, float x, float y, float z)
{
    if (light_index == 0) {
        g_lightPosWorld[0] = Vector3(x, y, z);
    }
}

void W3DShadowManager::Set_Time_Of_Day(TimeOfDayType tod)
{
    Vector3 light_pos(-g_theWriteableGlobalData->m_terrainObjectLighting[tod][0].lightPos.x,
        -g_theWriteableGlobalData->m_terrainObjectLighting[tod][0].lightPos.y,
        -g_theWriteableGlobalData->m_terrainObjectLighting[tod][0].lightPos.z);
    light_pos.Normalize();
    light_pos *= 10000.0f;
    Set_Light_Position(0, light_pos.X, light_pos.Y, light_pos.Z);
}
