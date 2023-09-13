/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Implements manager for drawing snow/rain effects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "w3dsnow.h"

#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "rinfo.h"
#include "view.h"
#include "w3d.h"
#include <cmath>

class FrustumClass;
class Coord3D;

W3DSnowManager::W3DSnowManager() :
    m_indexBuffer(nullptr),
    m_texture(nullptr),
#ifdef BUILD_WITH_D3D8
    m_vertexBuffer(nullptr),
#endif
    // BUGFIX, original only initialized the 3 above
    m_vertexBufferOffset(0),
    m_maxParticleCount(0),
    m_numVertices(0),
    m_unk5(0),
    m_unk6(0.0f),
    m_unk7(0.0f),
    m_currentParticleCount(0),
    m_unk9(0.0f)
{
}

W3DSnowManager::~W3DSnowManager()
{
    Release_Resources();
}

void W3DSnowManager::Init()
{
    SnowManager::Init();
    Re_Acquire_Resources();
}

void W3DSnowManager::Release_Resources()
{
    Ref_Ptr_Release(m_texture);

#ifdef BUILD_WITH_D3D8
    if (m_vertexBuffer) {
        m_vertexBuffer->Release();
    }
    m_vertexBuffer = nullptr;
#endif

    Ref_Ptr_Release(m_indexBuffer);
}

bool W3DSnowManager::Re_Acquire_Resources()
{
#ifdef BUILD_WITH_D3D8
    Release_Resources();

    if (!g_theWeatherSetting->m_snowEnabled) {
        return true;
    }

    if (g_theWeatherSetting->m_snowPointSprites && DX8Wrapper::Get_Current_Caps()->Supports_Large_Point_Size()) {
        IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();
        captainslog_dbgassert(device != nullptr, "Trying to ReAquireResources on W3DSnowManager without device");

        if (m_vertexBuffer == nullptr
            && device->CreateVertexBuffer(VERTEX_COUNT * sizeof(Vector3),
                D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY,
                DX8_FVF_XYZ,
                D3DPOOL_DEFAULT,
                &m_vertexBuffer)) {
            return false;
        }
    } else {
        m_indexBuffer = new DX8IndexBufferClass(MAX_PARTICLE_COUNT * 6, DX8IndexBufferClass::USAGE_DEFAULT);

        {
            IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);

            unsigned short *indices = lock.Get_Index_Array();
            unsigned short vert = 0;

            for (int i = 0; i < MAX_PARTICLE_COUNT; ++i) {
                indices[0] = vert + 3;
                indices[1] = vert + 0;
                indices[2] = vert + 2;
                indices[3] = vert + 2;
                indices[4] = vert + 0;
                indices[5] = vert + 1;
                vert += 4;
                indices += 6;
            }
        }
    }

    m_texture = W3DAssetManager::Get_Instance()->Get_Texture(g_theWeatherSetting->m_snowTexture.Str());
    m_vertexBufferOffset = VERTEX_COUNT;
    m_numVertices = VERTEX_COUNT;
    m_maxParticleCount = MAX_PARTICLE_COUNT;

    return true;
#else
    return false;
#endif
}

inline unsigned long F2DW(float f)
{
    return *((unsigned *)&f);
}

void W3DSnowManager::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (g_theWeatherSetting->m_snowEnabled && m_isVisible) {

        bool use_point_sprites =
            g_theWeatherSetting->m_snowPointSprites && DX8Wrapper::Get_Current_Caps()->Supports_Large_Point_Size();

        Coord3D cam_coord = g_theTacticalView->Get_3D_Camera_Position();

        Vector3 cam_pos(cam_coord.x, cam_coord.y, cam_coord.z);

        captainslog_assert((abs(m_boxDensity) >= FLT_EPSILON));

        int dist = (int)floor(m_boxDimensions / m_boxDensity * 0.5f);
        int cam_x = (int)floor(cam_pos.X / m_boxDensity);
        int cam_y = (int)floor(cam_pos.Y / m_boxDensity);

        int x1 = cam_x - dist;
        int y1 = cam_y - dist;
        int x2 = dist + cam_x;
        int y2 = dist + cam_y;

        AABoxClass aabox;
        g_theTerrainRenderObject->Get_Maximum_Visible_Box(rinfo.m_camera.Get_Frustum(), &aabox, true);

        aabox.m_extent.X = m_amplitude + m_particleSize + aabox.m_extent.X;
        aabox.m_extent.Y = m_amplitude + m_particleSize + aabox.m_extent.Y;

        if (aabox.m_center.X - aabox.m_extent.X > (float)x1 * m_boxDensity) {
            x1 = (int)floor((aabox.m_center.X - aabox.m_extent.X) / m_boxDensity);
        }

        if (aabox.m_center.Y - aabox.m_extent.Y > (float)y1 * m_boxDensity) {
            y1 = (int)floor((aabox.m_center.Y - aabox.m_extent.Y) / m_boxDensity);
        }

        if (aabox.m_center.X + aabox.m_extent.X < (float)x2 * m_boxDensity) {
            x2 = (int)floor((aabox.m_center.X + aabox.m_extent.X) / m_boxDensity);
        }

        if (aabox.m_center.Y + aabox.m_extent.Y < (float)y2 * m_boxDensity) {
            y2 = (int)floor((aabox.m_center.Y + aabox.m_extent.Y) / m_boxDensity);
        }

        if (y2 - y1 >= 0 && x2 - x1 >= 0 && (x2 - x1) * (y2 - y1) > 0) {

            m_unk6 = m_boxDimensions / 2.0f + cam_pos.Z;

            m_unk7 = m_elapsedTime * m_velocity + fmod(cam_pos.Z, m_boxDimensions);

            Matrix4 world(true);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, world);

            DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);

            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);
            Ref_Ptr_Release(material);

            if (use_point_sprites && m_vertexBuffer == nullptr) {
                Re_Acquire_Resources();
            }

            if (!use_point_sprites && m_indexBuffer == nullptr) {
                Re_Acquire_Resources();
            }

            DX8Wrapper::Set_Texture(0, m_texture);

            if (!use_point_sprites) {
                Render_As_Quads(rinfo, x1, y1, x2, y2);
            } else {
                DX8Wrapper::Apply_Render_State_Changes();

                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSPRITEENABLE, TRUE);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSCALEENABLE, TRUE);

                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSIZE, F2DW(m_pointSize));
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSIZE_MIN, F2DW(m_minPointSize));
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSIZE_MAX, F2DW(m_maxPointSize));

                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSCALE_A, F2DW(0.0));
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSCALE_B, F2DW(0.0));
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSCALE_C, F2DW(1.0));

                DX8Wrapper::Get_D3D_Device8()->SetStreamSource(0, m_vertexBuffer, sizeof(Vector3));
                DX8Wrapper::Get_D3D_Device8()->SetVertexShader(DX8_FVF_XYZ);

                m_vertexBufferOffset = VERTEX_COUNT;
                m_unk5 = 45;
                m_currentParticleCount = 0;

                m_unk9 = m_amplitude + m_particleSize;

                Render_Sub_Box(rinfo, x1, y1, x2, y2);

                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSPRITEENABLE, FALSE);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_POINTSCALEENABLE, FALSE);
            }
        }
    }
#endif
}

inline void W3DSnowManager::Render_Sub_Box_Internal(
    RenderInfoClass &rinfo, MinMaxAABoxClass &minmax_aabox, int x1, int y1, int x2, int y2)
{
    CameraClass &camera = rinfo.m_camera;

    float min_z = m_unk6 - m_boxDimensions;
    float min_y = (float)y1 * m_boxDensity - m_unk9;
    float min_x = (float)x1 * m_boxDensity - m_unk9;
    minmax_aabox.m_minCorner.Set(min_x, min_y, min_z);

    float max_y = (float)y2 * m_boxDensity + m_unk9;
    float max_x = (float)x2 * m_boxDensity + m_unk9;
    minmax_aabox.m_maxCorner.Set(max_x, max_y, m_unk6);

    if (CollisionMath::Overlap_Test(camera.Get_Frustum(), AABoxClass(minmax_aabox)) != CollisionMath::OUTSIDE) {
        Render_Sub_Box(rinfo, x1, y1, x2, y2);
    }
}

void W3DSnowManager::Render_Sub_Box(RenderInfoClass &rinfo, int x1, int y1, int x2, int y2)
{
#ifdef BUILD_WITH_D3D8
    int x_delta = x2 - x1;
    int y_delta = y2 - y1;

    int x_offset = GameMath::Fast_To_Int_Ceil(float(x2 - x1) / 2.0f);
    int y_offset = GameMath::Fast_To_Int_Ceil(float(y2 - y1) / 2.0f);

    MinMaxAABoxClass minmax_aabox;

    if (x_delta > m_unk5) {
        if (y_delta > m_unk5) {

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x1, y_offset + y1, x_offset + x1, y2);

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x_offset + x1, y_offset + y1, x2, y2);

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x1, y1, x_offset + x1, y_offset + y1);

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x_offset + x1, y1, x2, y_offset + y1);

        } else {

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x1, y1, x_offset + x1, y2);

            Render_Sub_Box_Internal(rinfo, minmax_aabox, x_offset + x1, y1, x2, y2);
        }
    } else if (y_delta > m_unk5) {

        Render_Sub_Box_Internal(rinfo, minmax_aabox, x1, y_offset + y1, x2, y2);

        Render_Sub_Box_Internal(rinfo, minmax_aabox, x1, y1, x2, y_offset + y1);

    } else {
        int count = (x2 - x1) * (y2 - y1);

        if (count != 0) {
            int start_y = y1;
            int start_x = x1;

            Vector3 vertex;
            Vector3 *points;

            m_currentParticleCount += count;

            while (count != 0) {
                int point_count = count;

                if (count > m_maxParticleCount) {
                    point_count = m_maxParticleCount;
                }

                if (m_vertexBufferOffset + point_count > m_numVertices) {
                    m_vertexBufferOffset = 0;
                }

                if (m_vertexBuffer->Lock(m_vertexBufferOffset * sizeof(Vector3),
                        point_count * sizeof(Vector3),
                        (BYTE **)&points,
                        m_vertexBufferOffset != 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)) {
                    break;
                }

                int points_written = 0;

                int y = start_y;
                while (y < y2 && points_written < point_count) {
                    int x = start_x;
                    while (x < x2 && points_written < point_count) {

                        int buffer_index = Get_Dimension_Buffer_Index(x, y);

                        if (buffer_index > DIMENSION_BUFFER_SIZE) {
                            buffer_index = 0;
                        }

                        float vz = m_unk6 - fmod(m_unk7 + m_dimensionsBuffer[buffer_index], m_boxDimensions);
                        float vy = (float)y * m_boxDensity;
                        float vx = (float)x * m_boxDensity;

                        vertex.Set(vx, vy, vz);

                        float f1 = vz * m_freqScaleX + (float)x;
                        float sin_x = GameMath::Fast_Sin(f1);
                        vertex.X = sin_x * m_amplitude + vertex.X;

                        float f2 = vz * m_freqScaleY + (float)y;
                        float sin_y = GameMath::Fast_Sin(f2);
                        vertex.Y = sin_y * m_amplitude + vertex.Y;

                        *points = vertex;
                        ++points;

                        ++points_written;
                        ++x;
                    }
                    ++y;
                }

                m_vertexBuffer->Unlock();
                if (points_written) {
                    DX8Wrapper::Get_D3D_Device8()->DrawPrimitive(D3DPT_POINTLIST, m_vertexBufferOffset, points_written);
                    count -= points_written;
                    m_vertexBufferOffset += points_written;
                }
            }
        }
    }
#endif
}

void W3DSnowManager::Render_As_Quads(RenderInfoClass &rinfo, int x1, int y1, int x2, int y2)
{
#ifdef BUILD_WITH_D3D8

    Matrix3D view;
    rinfo.m_camera.Get_View_Matrix(&view);

    Matrix4 proj;
    // nothing is done with it but may matter cause can modify the camera...
    rinfo.m_camera.Get_Projection_Matrix(&proj);

    Vector3 corners[4]{
        { -0.5f, 0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
        { 0.5f, -0.5f, 0.0f },
        { 0.5f, 0.5f, 0.0f },
    };

    Vector2 uv[4]{
        { 0.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
    };

    for (int i = 0; i < 4; ++i) {
        corners[i] *= m_particleSize;
    }

    Matrix4 identity(true);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);

    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);

    int start_y = y1;
    int start_x = x1;

    int count = (x2 - x1) * (y2 - y1);
    m_currentParticleCount += count;

    while (count) {

        int quad_count = count;
        if (count > MAX_PARTICLE_COUNT) {
            quad_count = MAX_PARTICLE_COUNT;
        }

        int quads_written = 0;
        DynamicVBAccessClass dynamic_vb_access(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, 4 * quad_count);
        {
            DynamicVBAccessClass::WriteLockClass lock(&dynamic_vb_access);

            VertexFormatXYZNDUV2 *vertexes = lock.Get_Formatted_Vertex_Array();

            int y = start_y;
            while (y < y2 && quads_written < quad_count) {
                int x = start_x;
                while (x < x2 && quads_written < quad_count) {

                    int buffer_index = Get_Dimension_Buffer_Index(x, y);

                    if (buffer_index > DIMENSION_BUFFER_SIZE) {
                        buffer_index = 0;
                    }

                    float val = m_unk6;
                    float fz = val - fmod(m_unk7 + m_dimensionsBuffer[buffer_index], m_boxDimensions);
                    float fy = (float)y * m_boxDensity;
                    float fx = (float)x * m_boxDensity;
                    Vector3 in;

                    in.Set(fx, fy, fz);

                    Vector3 pos;
                    Matrix3D::Transform_Vector(view, in, &pos);

                    float f1 = fz * m_freqScaleX + (float)x;

                    float sin_x = GameMath::Fast_Sin(f1);

                    pos.X = sin_x * m_amplitude + pos.X;

                    float f2 = fz * m_freqScaleY + (float)y;

                    float sin_y = GameMath::Fast_Sin(f2);

                    pos.Y = sin_y * m_amplitude + pos.Y;

                    for (int k = 0; k < 4; ++k) {
                        Vector3 vertex = pos + corners[k];
                        vertexes->x = vertex.X;
                        vertexes->y = vertex.Y;
                        vertexes->z = vertex.Z;
                        vertexes->nx = 0.0f;
                        vertexes->ny = 0.0f;
                        vertexes->nz = 0.0f;
                        vertexes->diffuse = 0xFFFFFFFF;
                        vertexes->u1 = uv[k].X;
                        vertexes->v1 = uv[k].Y;
                        vertexes->u2 = 0.0f;
                        vertexes->v2 = 0.0f;
                        ++vertexes;
                    }
                    ++quads_written;
                    ++x;
                }
                ++y;
            }
        }

        if (quads_written) {
            DX8Wrapper::Set_Vertex_Buffer(dynamic_vb_access);
            DX8Wrapper::Draw_Triangles(0, 2 * quads_written, 0, 4 * quads_written);
            count -= quads_written;
        }
    }
#endif
}

void W3DSnowManager::Update_INI_Settings()
{
    SnowManager::Update_INI_Settings();

    if (m_texture != nullptr) {
        if (strcasecmp(m_texture->Get_Name(), g_theWeatherSetting->m_snowTexture.Str()) != 0) {
            Ref_Ptr_Release(m_texture);
            m_texture = W3DAssetManager::Get_Instance()->Get_Texture(g_theWeatherSetting->m_snowTexture.Str());
        }
    }
}

void W3DSnowManager::Reset()
{
    SnowManager::Reset();
}

void W3DSnowManager::Update()
{
    m_elapsedTime += (float)W3D::Get_Frame_Time() / 1000.0f;
    m_elapsedTime = fmod(m_elapsedTime, m_lifetime);
}
