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
#include "w3dsmudge.h"
#include "camera.h"
#include "display.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "rinfo.h"
#include "shadermanager.h"
#include "sortingrenderer.h"
#include "vertmaterial.h"
#include "view.h"

#ifndef GAME_DLL
W3DSmudgeManager *g_theSmudgeManager = nullptr;
#endif

W3DSmudgeManager::~W3DSmudgeManager()
{
    Release_Resources();
}

void W3DSmudgeManager::Init()
{
    SmudgeManager::Init();
    Re_Acquire_Resources();
}

void W3DSmudgeManager::Reset()
{
    SmudgeManager::Reset();
}

void W3DSmudgeManager::Release_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
}

void W3DSmudgeManager::Re_Acquire_Resources()
{
#ifdef BUILD_WITH_D3D8
    Release_Resources();
    SurfaceClass *buffer = DX8Wrapper::Get_DX8_Back_Buffer();
    SurfaceClass::SurfaceDescription desc;
    buffer->Get_Description(desc);
    Ref_Ptr_Release(buffer);

    m_surfaceWidth = desc.width;
    m_surfaceHeight = desc.height;
    m_indexBuffer = new DX8IndexBufferClass(NUM_SMUDGES * SMUDGE_INDEX_COUNT, DX8IndexBufferClass::USAGE_DEFAULT);

    IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
    unsigned short *indices = lock.Get_Index_Array();
    int index = 0;

    for (int i = 0; i < NUM_SMUDGES; i++) {
        indices[0] = index;
        indices[1] = index + 4;
        indices[2] = index + 3;
        indices[3] = index + 3;
        indices[4] = index + 4;
        indices[5] = index + 2;
        indices[6] = index + 2;
        indices[7] = index + 4;
        indices[8] = index + 1;
        indices[9] = index + 1;
        indices[10] = index + 4;
        indices[11] = index;
        index += SMUDGE_VERTEX_COUNT;
        indices += SMUDGE_INDEX_COUNT;
    }
#endif
}

int Copy_Rect(unsigned char *dest_buffer, int size, int x, int y, int width, int height)
{
#ifdef BUILD_WITH_D3D8
    IDirect3DSurface8 *source = nullptr;
    IDirect3DSurface8 *dest = nullptr;
    int ret_size = 0;
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    if (device != nullptr) {
        device->GetRenderTarget(&source);

        if (source != nullptr) {
            D3DSURFACE_DESC desc;
            source->GetDesc(&desc);

            RECT rects;
            rects.left = x;
            rects.top = y;
            rects.right = width + x;
            rects.bottom = height + y;

            POINT points;
            points.x = 0;
            points.y = 0;

            HRESULT res = device->CreateImageSurface(width, height, desc.Format, &dest);

            if (SUCCEEDED(res)) {
                res = device->CopyRects(source, &rects, 1, dest, &points);

                if (SUCCEEDED(res)) {
                    D3DLOCKED_RECT locked;
                    res = dest->LockRect(&locked, nullptr, D3DLOCK_READONLY);

                    if (SUCCEEDED(res)) {
                        dest->GetDesc(&desc);

                        if (desc.Size < (unsigned int)size) {
                            size = desc.Size;
                        }

                        memcpy(dest_buffer, locked.pBits, size);
                        ret_size = size;
                    }
                }
            }
        }
    }

    if (source != nullptr) {
        source->Release();
    }

    if (dest != nullptr) {
        dest->Release();
    }

    return ret_size;
#else
    return 0;
#endif
}

bool W3DSmudgeManager::Test_Hardware_Support()
{
#ifdef BUILD_WITH_D3D8
    struct HardwareTestVertex
    {
        Vector4 p;
        unsigned long color;
        float u;
        float v;
    };

    if (m_hardwareTestResult == HARDWARE_TEST_NOT_RUN) {
        IDirect3DTexture8 *texture = W3DShaderManager::Get_Render_Texture();

        if (texture == nullptr) {
            m_hardwareTestResult = HARDWARE_TEST_FAIL;
            return false;
        }

        if (!W3DShaderManager::Get_Render_To_Texture()) {
            return false;
        }

        VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(material);
        Ref_Ptr_Release(material);
        ShaderClass shader(ShaderClass::s_presetOpaqueShader);
        shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
        shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Apply_Render_State_Changes();

        HardwareTestVertex vertices[4];
        vertices[0].p = Vector4(7.5f, 7.5f, 0.0f, 1.0f);
        vertices[0].u = 8.0f / (float)g_theDisplay->Get_Width();
        vertices[0].v = 8.0f / (float)g_theDisplay->Get_Height();

        vertices[1].p = Vector4(7.5f, -0.5f, 0.0f, 1.0f);
        vertices[1].u = 8.0f / (float)g_theDisplay->Get_Width();
        vertices[1].v = 0.0f;

        vertices[2].p = Vector4(-0.5f, 7.5f, 0.0f, 1.0f);
        vertices[2].u = 0.0f;
        vertices[2].v = 8.0f / (float)g_theDisplay->Get_Height();

        vertices[3].p = Vector4(-0.5f, -0.5f, 0.0f, 1.0f);
        vertices[3].u = 0.0f;
        vertices[3].v = 0.0f;

        vertices[0].color = 0x12345678;
        vertices[1].color = 0x12345678;
        vertices[2].color = 0x12345678;
        vertices[3].color = 0x12345678;

        IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();
        device->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(HardwareTestVertex));
        unsigned char dest[256];
        memset(dest, 0, sizeof(dest));
        unsigned int size = Copy_Rect(dest, 256, 0, 0, 8, 8);

        if (!size) {
            m_hardwareTestResult = HARDWARE_TEST_FAIL;
            return false;
        }

        DX8Wrapper::Set_DX8_Texture(0, texture);
        unsigned char dest2[256];
        memset(dest2, 255, sizeof(dest2));
        vertices[0].color = 0xFFFFFFFF;
        vertices[1].color = 0xFFFFFFFF;
        vertices[2].color = 0xFFFFFFFF;
        vertices[3].color = 0xFFFFFFFF;
        device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(HardwareTestVertex));
        size = Copy_Rect(dest2, 256, 0, 0, 8, 8);

        if (!size) {
            m_hardwareTestResult = HARDWARE_TEST_FAIL;
            return false;
        }

        if (!memcmp(dest2, dest, size)) {
            m_hardwareTestResult = HARDWARE_TEST_SUCCEED;
            return true;
        }

        m_hardwareTestResult = HARDWARE_TEST_FAIL;
    }

    return m_hardwareTestResult == HARDWARE_TEST_SUCCEED;
#else
    return false;
#endif
}

void W3DSmudgeManager::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (Test_Hardware_Support()) {
        CameraClass &camera = rinfo.m_camera;
        Vector3 v1;
        Vector4 v2;

        Vector3 v3[4] = {
            Vector3(-0.5f, 0.5f, 0.0f), Vector3(-0.5f, -0.5f, 0.0f), Vector3(0.5f, -0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.0f)
        };
        static_assert(ARRAY_SIZE(v3) <= ARRAY_SIZE(Smudge::m_smudgeStructs));

        unsigned int color[SMUDGE_VERTEX_COUNT] = { 0xFFEEDD, 0xFFEEDD, 0xFFEEDD, 0xFFEEDD, 0xFFEEDD };
        Matrix4 m1;
        Matrix3D m2;
        camera.Get_View_Matrix(&m2);
        camera.Get_Projection_Matrix(&m1);
        IDirect3DTexture8 *texture = W3DShaderManager::Get_Render_Texture();

        if (texture != nullptr) {
            if (W3DShaderManager::Get_Render_To_Texture()) {
                D3DSURFACE_DESC desc;
                texture->GetLevelDesc(0, &desc);
                int width = desc.Width;
                int height = desc.Height;
                float f1 = (float)g_theTacticalView->Get_Width() / (float)width;
                float f2 = (float)g_theTacticalView->Get_Height() / (float)height;
                float f3 = f1 * 0.5f;
                float f4 = f2 * 0.5f;
                SmudgeSet *set = m_smudgeList.Head();
                int count = 0;

                if (set != nullptr) {
                    SortingRendererClass::Flush();
                }

                while (set != nullptr) {
                    DLListClass<Smudge> &list = set->Get_Smudge_List();

                    for (Smudge *smudge = list.Head(); smudge != nullptr; smudge = smudge->Succ()) {
                        Matrix3D::Transform_Vector(m2, smudge->m_smudgePos, &v1);
                        smudge->m_smudgeStructs[4].m_position = v1;

                        for (int i = 0; i < ARRAY_SIZE(v3); i++) {
                            smudge->m_smudgeStructs[i].m_position = v1 + (v3[i] * smudge->m_smudgeSize);
                            v2 = m1 * smudge->m_smudgeStructs[i].m_position;
                            v2 *= 1.0f / v2.W;
                            smudge->m_smudgeStructs[i].m_uv.Set((v2.X + 1.0f) * f3, (1.0f - v2.Y) * f4);
                            Vector2 *uv = &smudge->m_smudgeStructs[i].m_uv;

                            if (uv->X > (float)f1) {
                                smudge->m_smudgeRandom.X = 0.0f;
                            } else if (uv->X < 0.0f) {
                                smudge->m_smudgeRandom.X = 0.0f;
                            }

                            if (uv->Y > (float)f2) {
                                smudge->m_smudgeRandom.Y = 0.0f;
                            } else if (uv->Y < 0.0f) {
                                smudge->m_smudgeRandom.Y = 0.0f;
                            }
                        }

                        float f5 = smudge->m_smudgeStructs[3].m_uv.X - smudge->m_smudgeStructs->m_uv.X;
                        float f6 = smudge->m_smudgeStructs[1].m_uv.Y - smudge->m_smudgeStructs->m_uv.Y;
                        smudge->m_smudgeStructs[4].m_uv.X =
                            (smudge->m_smudgeRandom.X + 0.5f) * f5 + smudge->m_smudgeStructs->m_uv.X;
                        smudge->m_smudgeStructs[4].m_uv.Y =
                            (smudge->m_smudgeRandom.X + 0.5f) * f6 + smudge->m_smudgeStructs->m_uv.Y;
                        count++;
                    }

                    set = set->Succ();
                }

                if (count != 0) {
                    Matrix4 m3(true);
                    DX8Wrapper::Set_Transform(D3DTS_WORLD, m3);
                    DX8Wrapper::Set_Transform(D3DTS_VIEW, m3);
                    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
                    DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);

                    DX8Wrapper::Set_DX8_Texture(0, texture);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);

                    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
                    DX8Wrapper::Set_Material(material);
                    Ref_Ptr_Release(material);

                    DX8Wrapper::Apply_Render_State_Changes();
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
                    int count2 = count;

                    set = m_smudgeList.Head();
                    Smudge *smudge = set->Get_Smudge_List().Head();

                    while (count2) {
                        count = count2;

                        if (count2 > NUM_SMUDGES) {
                            count = NUM_SMUDGES;
                        }

                        int count3 = 0;
                        DynamicVBAccessClass vb(
                            VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, SMUDGE_VERTEX_COUNT * count);

                        {
                            DynamicVBAccessClass::WriteLockClass lock(&vb);
                            VertexFormatXYZNDUV2 *vertices = lock.Get_Formatted_Vertex_Array();

                            while (set != nullptr) {
                                for (Smudge *s = smudge; s != nullptr; s = s->Succ()) {
                                    SmudgeCoords *c = s->m_smudgeStructs;

                                    if (count3 >= count) {
                                        smudge = s;
                                        goto done;
                                    }

                                    color[4] = ((unsigned int)(s->m_smudgeAlpha * 255.0f) << 24) | 0xFFEEDD;

                                    for (int k = 0; k < SMUDGE_VERTEX_COUNT; k++) {
                                        vertices->x = c->m_position.X;
                                        vertices->y = c->m_position.Y;
                                        vertices->z = c->m_position.Z;
                                        vertices->nx = 0.0f;
                                        vertices->ny = 0.0f;
                                        vertices->nz = 0.0f;
                                        vertices->diffuse = color[k];
                                        vertices->u1 = c->m_uv.X;
                                        vertices->v1 = c->m_uv.Y;
                                        vertices->u2 = 0.0f;
                                        vertices->v2 = 0.0f;
                                        vertices++;
                                        c++;
                                    }

                                    count3++;
                                }

                                set = set->Succ();

                                if (set) {
                                    smudge = set->Get_Smudge_List().Head();
                                }
                            }

                        done:
                            DX8Wrapper::Set_Vertex_Buffer(vb);
                        }

                        DX8Wrapper::Draw_Triangles(0, SMUDGE_POLY_COUNT * count3, 0, SMUDGE_VERTEX_COUNT * count3);
                        count2 -= count3;
                    }

                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                }
            }
        }
    }
#endif
}
