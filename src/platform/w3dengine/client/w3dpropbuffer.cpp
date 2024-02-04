/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Prop Drawing Code
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dpropbuffer.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "geometry.h"
#include "globaldata.h"
#include "light.h"
#include "lightenv.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "rinfo.h"
#include "w3dshroud.h"

W3DPropBuffer::W3DPropBuffer() : m_propsUpdated(false), m_redoCull(false)
{
    m_isInited = false;
    Clear_All_Props();
    m_light = new LightClass(LightClass::DIRECTIONAL);
    m_shroudMaterial = new W3DShroudMaterialPassClass();
    m_isInited = true;
}

W3DPropBuffer::~W3DPropBuffer()
{
    for (int i = 0; i < MAX_PROP_TYPES; i++) {
        Ref_Ptr_Release(m_propTypes[i].render_obj);
    }

    Ref_Ptr_Release(m_light);
    Ref_Ptr_Release(m_shroudMaterial);
}

void W3DPropBuffer::Cull(CameraClass *camera)
{
    for (int i = 0; i < m_numProps; i++) {
        m_props[i].is_visible = !camera->Cull_Sphere(m_props[i].bounding_sphere);
    }
}

void W3DPropBuffer::Clear_All_Props()
{
    m_numProps = 0;

    for (int i = 0; i < MAX_PROP_TYPES; i++) {
        Ref_Ptr_Release(m_propTypes[i].render_obj);
        m_propTypes[i].name.Clear();
    }

    m_numPropTypes = 0;
}

int W3DPropBuffer::Add_Prop_Type(const Utf8String &name)
{
    if (m_numPropTypes >= MAX_PROP_TYPES) {
        captainslog_dbgassert(false, "Too many kinds of props in map.  Reduce kinds of props, or raise prop limit.");
        return 0;
    } else {
        m_propTypes[m_numPropTypes].render_obj = W3DAssetManager::Get_Instance()->Create_Render_Obj(name.Str());

        if (m_propTypes[m_numPropTypes].render_obj == nullptr) {
            captainslog_dbgassert(false, "Unable to find model for prop %s", name.Str());
            return -1;
        }

        m_propTypes[m_numPropTypes].name = name;
        m_propTypes[m_numPropTypes].bounding_sphere = m_propTypes[m_numPropTypes].render_obj->Get_Bounding_Sphere();
        return m_numPropTypes++;
    }
}

void W3DPropBuffer::Add_Prop(int id, Coord3D position, float orientation, float scale, const Utf8String &name)
{
    if (m_numProps < MAX_PROPS && m_isInited) {
        int index = -1;

        for (int i = 0; i < m_numPropTypes; i++) {
            if (m_propTypes[i].name.Compare_No_Case(name) == 0) {
                index = i;
                break;
            }
        }

        if (index >= 0 || (index = Add_Prop_Type(name), index >= 0)) {
            Matrix3D tm(true);
            tm.Rotate_Z(orientation);
            tm.Scale(scale);
            tm.Set_Translation(Vector3(position.x, position.y, position.z));
            m_props[m_numProps].position = position;
            m_props[m_numProps].id = id;
            m_props[m_numProps].shroud_status = SHROUDED_INVALID;
            m_props[m_numProps].render_obj = m_propTypes[index].render_obj->Clone();
            m_props[m_numProps].render_obj->Set_Transform(tm);
            m_props[m_numProps].render_obj->Set_ObjectScale(scale);
            m_props[m_numProps].prop_type = index;
            m_props[m_numProps].bounding_sphere = m_propTypes[index].bounding_sphere;
            m_props[m_numProps].bounding_sphere.Center += Vector3(position.x, position.y, position.z);
            m_props[m_numProps].is_visible = false;
            m_numProps++;
        }
    }
}

bool W3DPropBuffer::Update_Prop_Position(int id, const Coord3D &position, float orientation, float scale)
{
    for (int i = 0; i < m_numProps; i++) {
        if (m_props[i].id == id) {
            Matrix3D tm(true);
            tm.Rotate_Z(orientation);
            tm.Scale(scale);
            tm.Set_Translation(Vector3(position.x, position.y, position.z));
            m_props[i].position = position;
            m_props[i].render_obj->Set_Transform(tm);
            m_props[i].render_obj->Set_ObjectScale(scale);
            m_props[i].bounding_sphere = m_propTypes[m_props[i].prop_type].bounding_sphere;
            m_props[i].bounding_sphere.Center += Vector3(position.x, position.y, position.z);
            m_propsUpdated = true;
            return true;
        }
    }

    return false;
}

void W3DPropBuffer::Remove_Prop(int id)
{
    for (int i = 0; i < m_numProps; i++) {
        if (m_props[i].id == id) {
            m_props[i].position.Set(0.0f, 0.0f, 0.0f);
            m_props[i].prop_type = -1;
            Ref_Ptr_Release(m_props[i].render_obj);
            m_props[i].bounding_sphere.Center = Vector3(0.0f, 0.0f, 0.0f);
            m_props[i].bounding_sphere.Radius = 1.0f;
            m_propsUpdated = true;
        }
    }
}

void W3DPropBuffer::Remove_Props_For_Construction(const Coord3D *position, const GeometryInfo &geometry, float angle)
{
    for (int i = 0; i < m_numProps; i++) {
        if (m_props[i].render_obj != nullptr) {
            float radius = m_props[i].bounding_sphere.Radius;
            GeometryInfo geometry2(GEOMETRY_CYLINDER, false, 5.0f * radius, 2.0f * radius, 2.0f * radius);

            if (g_thePartitionManager->Geom_Collides_With_Geom(
                    position, geometry, angle, &m_props[i].position, geometry2, 0.0f)) {
                m_props[i].position.Set(0.0f, 0.0f, 0.0f);
                m_props[i].prop_type = -1;
                Ref_Ptr_Release(m_props[i].render_obj);
                m_props[i].bounding_sphere.Center = Vector3(0.0f, 0.0f, 0.0f);
                m_props[i].bounding_sphere.Radius = 1.0f;
                m_propsUpdated = true;
            }
        }
    }
}

void W3DPropBuffer::Notify_Shroud_Changed()
{
    for (int i = 0; i < m_numProps; i++) {
        m_props[i].shroud_status = g_thePartitionManager == nullptr ? SHROUDED_NONE : SHROUDED_INVALID;
    }
}

void W3DPropBuffer::Draw_Props(RenderInfoClass &rinfo)
{
    if (m_redoCull) {
        Cull(&rinfo.m_camera);
    }

    GlobalData::TerrainLighting *lighting =
        g_theWriteableGlobalData->m_terrainObjectLighting[g_theWriteableGlobalData->m_timeOfDay];
    LightEnvironmentClass lightenv;
    Vector3 object_center(0.0f, 0.0f, 0.0f);
    Vector3 ambient(lighting[0].ambient.red, lighting[0].ambient.green, lighting[0].ambient.blue);
    lightenv.Reset(object_center, ambient);
    Matrix3D tm;
    Vector3 v(0.0f, 0.0f, 0.0f);
    Vector3 x(1.0f, 0.0f, 0.0f);
    Vector3 y(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < LIGHT_COUNT; i++) {
        m_light->Set_Ambient(v);
        m_light->Set_Diffuse(Vector3(lighting[i].diffuse.red, lighting[i].diffuse.green, lighting[i].diffuse.blue));
        m_light->Set_Specular(v);
        tm.Set(x, y, Vector3(lighting[i].lightPos.x, lighting[i].lightPos.y, lighting[i].lightPos.z), v);
        m_light->Set_Transform(tm);
        lightenv.Add_Light(*m_light);
    }

    rinfo.m_lightEnvironment = &lightenv;

    for (int i = 0; i < m_numProps; i++) {
        if (m_props[i].is_visible && m_props[i].render_obj != nullptr) {
            if (g_thePlayerList == nullptr || g_thePartitionManager == nullptr) {
                m_props[i].shroud_status = SHROUDED_NONE;
            }

            if (m_props[i].shroud_status == SHROUDED_INVALID) {
                int index;

                if (g_thePlayerList != nullptr) {
                    index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
                } else {
                    index = 0;
                }

                m_props[i].shroud_status =
                    g_thePartitionManager->Get_Prop_Shroud_Status_For_Player(index, &m_props[i].position);
            }

            if (m_props[i].shroud_status < SHROUDED_NEVERSEEN && m_props[i].shroud_status > SHROUDED_INVALID) {
                if (g_theTerrainRenderObject->Get_Shroud() != nullptr && m_props[i].shroud_status != SHROUDED_INVALID) {
                    rinfo.Push_Material_Pass(m_shroudMaterial);
                    m_props[i].render_obj->Render(rinfo);
                    rinfo.Pop_Material_Pass();
                } else {
                    m_props[i].render_obj->Render(rinfo);
                }
            }
        }
    }

    rinfo.m_lightEnvironment = nullptr;
}
