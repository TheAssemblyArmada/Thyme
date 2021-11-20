/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Water Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dwater.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "light.h"
#include "matinfo.h"
#include "mesh.h"
#include "polygontrigger.h"
#include "rinfo.h"
#include "rtsutils.h"
#include "shadermanager.h"
#include "surfaceclass.h"
#include "texture.h"
#include "vertmaterial.h"
#include "w3dpoly.h"
#include "w3dscene.h"
#include "w3dshroud.h"
#include "w3dwatertracks.h"
#include "water.h"
#include "xfer.h"

#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

ShaderClass g_zFillAlphaShader(0x184BB);

void Do_Sky_Box_Set(bool b)
{
    if (g_theWriteableGlobalData) {
        g_theWriteableGlobalData->m_drawSkyBox = b;
    }
}

WaterRenderObjClass::WaterRenderObjClass() :
    m_settings{},
    m_dx(0.0f),
    m_dy(0.0f),
    m_indexBuffer(nullptr),
    m_waterTrackSystem(nullptr),
    m_doWaterGrid(false),
    m_meshVertexMaterialClass(nullptr),
    m_meshLight(nullptr),
    m_vertexMaterialClass(nullptr),
    m_alphaClippingTexture(nullptr),
    m_useCloudLayer(true),
    m_waterType(WATER_TYPE_0_TRANSLUCENT),
    m_tod(TIME_OF_DAY_AFTERNOON),
    m_pReflectionTexture(nullptr),
    m_skyBox(nullptr),
    m_vertexBufferD3D(nullptr),
    m_indexBufferD3D(nullptr),
    m_vertexBufferD3DOffset(0),
    m_dwWavePixelShader(0),
    m_meshData(nullptr),
    m_meshDataCount(0),
    m_meshInMotion(false),
    m_gridOrigin(0.0f, 0.0f),
    m_gridDirectionX(1.0f, 0.0f),
    m_gridDirectionY(1.0f, 0.0f),
    m_gridCellSize(10.0f),
    m_gridCellsX(128),
    m_gridCellsY(128),
    m_gridWidth(m_gridCellsX * m_gridCellSize),
    m_gridHeight(m_gridCellsY * m_gridCellSize),
    m_riverVOrigin(0.0f),
    m_riverTexture(nullptr),
    m_unkTexture(nullptr),
    m_waterNoiseTexture(nullptr),
    m_riverAlphaEdge(nullptr),
    m_waterPixelShader(0),
    m_riverWaterPixelShader(0),
    m_trapezoidWaterPixelShader(0),
    m_waterSparklesTexture(nullptr),
    m_riverXOffset(0.0f),
    m_riverYOffset(0.0f)
{
}

WaterRenderObjClass::~WaterRenderObjClass()
{
    Ref_Ptr_Release(m_meshVertexMaterialClass);
    Ref_Ptr_Release(m_vertexMaterialClass);
    Ref_Ptr_Release(m_meshLight);
    Ref_Ptr_Release(m_alphaClippingTexture);
    Ref_Ptr_Release(m_skyBox);
    Ref_Ptr_Release(m_riverTexture);
    Ref_Ptr_Release(m_unkTexture);
    Ref_Ptr_Release(m_waterNoiseTexture);
    Ref_Ptr_Release(m_riverAlphaEdge);
    Ref_Ptr_Release(m_waterSparklesTexture);

    for (int i = 0; i < 5; i++) {
        Ref_Ptr_Release(m_settings[i].sky_texture);
        Ref_Ptr_Release(m_settings[i].water_texture);
    }

#ifdef BUILD_WITH_D3D8
    for (int i = 0; i < 32; i++) {
        if (m_pBumpTexture[i]) {
            m_pBumpTexture[i]->Release();
            m_pBumpTexture[i] = nullptr;
        }

        if (m_pBumpTexture2[i]) {
            m_pBumpTexture2[i]->Release();
            m_pBumpTexture2[i] = nullptr;
        }
    }
#endif

    if (m_meshData) {
        delete m_meshData;
    }

    m_meshData = nullptr;
    m_meshDataCount = 0;

    for (int i = 0; i < 5; i++) {
        g_waterSettings[i].m_skyTextureFile.Clear();
        g_waterSettings[i].m_waterTextureFile.Clear();
    }

    WaterTransparencySetting *w = g_theWaterTransparency;
    w->Delete_Instance();
    g_theWaterTransparency = nullptr;
    Release_Resources();

    if (m_waterTrackSystem) {
        delete m_waterTrackSystem;
    }
}

void WaterRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(50000.0f, 50000.0f, 0.0f).Length());
}

void WaterRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(50000.0f, 50000.0f, 0.001f * m_dx));
}

int WaterRenderObjClass::Class_ID() const
{
    return CLASSID_UNKNOWN;
}

RenderObjClass *WaterRenderObjClass::Clone() const
{
    return nullptr;
}

long WaterRenderObjClass::Init_Bump_Map(IDirect3DTexture8 **tex, TextureClass *bump_source)
{
#ifdef BUILD_WITH_D3D8
    SurfaceClass::SurfaceDescription surface_desc;
    bump_source->Get_Level_Description(surface_desc, 0);

    if (Get_Bytes_Per_Pixel(surface_desc.format) != 4) {
        return 0;
    }

    if (!bump_source->Peek_Platform_Base_Texture()) {
        return 0;
    }

    IDirect3DBaseTexture8 *base = bump_source->Peek_Platform_Base_Texture();
    int levels = base->GetLevelCount();
    *tex = DX8Wrapper::Create_Texture(
        surface_desc.width, surface_desc.height, WW3D_FORMAT_U8V8, MIP_LEVELS_ALL, D3DPOOL_MANAGED, false);

    for (int level = 0; level < levels; level++) {
        SurfaceClass *l = bump_source->Get_Surface_Level(level);
        l->Get_Description(surface_desc);
        int pitch;
        unsigned char *surface = (unsigned char *)l->Lock(&pitch);
        D3DLOCKED_RECT r;
        (*tex)->LockRect(level, &r, nullptr, 0);
        int p = r.Pitch;
        unsigned char *b = (unsigned char *)r.pBits;

        for (unsigned int i = 0; i < surface_desc.height; i++) {
            unsigned char *b1 = b;
            unsigned char *b2 = surface;
            unsigned char *b3 = &surface[pitch];
            unsigned char *b4 = &surface[-pitch];

            if (i == surface_desc.height - 1) {
                b3 = surface;
            }

            if (!i) {
                b4 = surface;
            }

            for (unsigned int j = 0; j < surface_desc.width; j++) {
                int i1 = 0x100 - *b2;
                int i2 = 0x100 - b2[4];
                int i3 = 0x100 - b2[-4];
                int i4 = b2[4] - b2[-4];
                int i5 = *b3 - *b4;

                if (i1 < i3 && i1 < i2) {
                    i4 = *b2 - b2[-4];

                    if (i3 - i1 < i1 - i2) {
                        i4 = b2[4] - *b2;
                    }
                }

                *b1++ = i4;
                *b1++ = i5;
                b2 += 4;
                b3 += 4;
                b4 += 4;
            }

            surface += pitch;
            b += p;
        }

        (*tex)->UnlockRect(level);
        l->Unlock();
        Ref_Ptr_Release(l);
    }

#endif
    return 0;
}

#ifdef BUILD_WITH_D3D8
HRESULT WaterRenderObjClass::Generate_Vertex_Buffer(int size_x, int size_y, int vertex_size, bool do_static)
{
    m_numVertices = size_y * size_x;
    Setting *setting = &m_settings[m_tod];
    D3DPOOL pool = D3DPOOL_DEFAULT;
    DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    DWORD fvf = DX8_FVF_XYZDUV2;

    if (do_static) {
        pool = D3DPOOL_MANAGED;
        usage = D3DUSAGE_WRITEONLY;
        fvf = 0;
    }

    if (!m_vertexBufferD3D) {
        HRESULT error = m_pDev->CreateVertexBuffer(vertex_size * m_numVertices, usage, fvf, pool, &m_vertexBufferD3D);

        if (error < 0) {
            return error;
        }
    }

    m_vertexBufferD3DOffset = 0;

    if (!do_static) {
        return 0;
    }

    SEA_PATCH_VERTEX *vertices;
    HRESULT error = m_vertexBufferD3D->Lock(0, m_numVertices * sizeof(SEA_PATCH_VERTEX), (BYTE **)&vertices, 0);

    if (error < 0) {
        return error;
    }

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            vertices->x = j;
            vertices->y = m_level;
            vertices->z = i;
            vertices->tu = j * 3.0f;
            vertices->tv = i * 3.0f;
            vertices->c = setting->transparent_water_diffuse;
            vertices++;
        }
    }

    error = m_vertexBufferD3D->Unlock();

    if (error < 0) {
        return error;
    }
    return 0;
}
#endif

#ifdef BUILD_WITH_D3D8
HRESULT WaterRenderObjClass::Generate_Index_Buffer(int size_x, int size_y)
{
    m_numIndices = (2 * size_x + 2) * (size_y - 1) - 2;
    HRESULT error = m_pDev->CreateIndexBuffer(
        2 * m_numIndices + 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_indexBufferD3D);

    if (error < 0) {
        return error;
    }

    unsigned short *indices;
    error = m_indexBufferD3D->Lock(0, 2 * m_numIndices, (BYTE **)&indices, 0);

    if (error < 0) {
        return error;
    }

    int i1 = 0;
    int i2 = 0;
    int i3 = 0;

    while (i1 < m_numIndices) {
        while (i3 < (i2 + 1) * size_x) {
            indices[i1] = size_x + i3;
            indices[i1 + 1] = i3++;
            i1 += 2;
        }

        if (i1 < m_numIndices) {
            indices[i1] = i3 - 1;
            indices[i1 + 1] = size_x + i3;
            i1 += 2;
        }

        i2++;
    }

    error = m_indexBufferD3D->Unlock();

    if (error < 0) {
        return error;
    }
    return 0;
}
#endif

void WaterRenderObjClass::Release_Resources()
{
#ifdef BUILD_WITH_D3D8
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_pReflectionTexture);

    if (m_vertexBufferD3D) {
        m_vertexBufferD3D->Release();
        m_vertexBufferD3D = nullptr;
    }

    if (m_indexBufferD3D) {
        m_indexBufferD3D->Release();
        m_indexBufferD3D = nullptr;
    }

    if (m_waterTrackSystem) {
        m_waterTrackSystem->Release_Resources();
    }

    if (m_dwWavePixelShader) {
        m_pDev->DeletePixelShader(m_dwWavePixelShader);
    }

    if (m_dwWaveVertexShader) {
        m_pDev->DeleteVertexShader(m_dwWaveVertexShader);
    }

    if (m_waterPixelShader) {
        m_pDev->DeletePixelShader(m_waterPixelShader);
    }

    if (m_trapezoidWaterPixelShader) {
        m_pDev->DeletePixelShader(m_trapezoidWaterPixelShader);
    }

    if (m_riverWaterPixelShader) {
        m_pDev->DeletePixelShader(m_riverWaterPixelShader);
    }

    m_dwWavePixelShader = 0;
    m_dwWaveVertexShader = 0;
    m_waterPixelShader = 0;
    m_trapezoidWaterPixelShader = 0;
    m_riverWaterPixelShader = 0;
#endif
}

void WaterRenderObjClass::Re_Acquire_Resources()
{
#ifdef BUILD_WITH_D3D8
    m_indexBuffer = new DX8IndexBufferClass(6, DX8IndexBufferClass::USAGE_DEFAULT);

    {
        IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
        unsigned short *indices = lock.Get_Index_Array();
        indices[0] = 3;
        indices[1] = 0;
        indices[2] = 2;
        indices[3] = 2;
        indices[4] = 0;
        indices[5] = 1;
    }

    m_pDev = DX8Wrapper::Get_D3D_Device8();

    if (!m_meshData) {
        if (m_waterType == WATER_TYPE_2_PVSHADER) {
            HRESULT error = Generate_Index_Buffer(15, 15);

            if (error < 0) {
                return;
            }

            error = Generate_Vertex_Buffer(15, 15, 24, true);

            if (error < 0) {
                return;
            }

            DWORD decl[] = { D3DVSD_STREAM(0),
                D3DVSD_REG(0, D3DVSDT_FLOAT3),
                D3DVSD_REG(1, D3DVSDT_D3DCOLOR),
                D3DVSD_REG(2, D3DVSDT_FLOAT2),
                D3DVSD_END() };
            error = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\wave.pso", decl, 0, false, &m_dwWavePixelShader);

            if (error < 0) {
                return;
            }

            error = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\wave.vso", decl, 0, true, &m_dwWaveVertexShader);

            if (error < 0) {
                return;
            }

            m_pReflectionTexture = DX8Wrapper::Create_Render_Target(256, 256, WW3D_FORMAT_UNKNOWN);
        }
    label1:

        if (m_waterTrackSystem) {
            m_waterTrackSystem->Re_Acquire_Resources();
        }

        if (W3DShaderManager::Get_Chipset() > GPU_PS11) {
            ID3DXBuffer *shader;
            const char *ps1 = "ps.1.1\n"
                              " \t\t\ttex t0 \n"
                              "\t\t\ttex t1\t\n"
                              "\t\t\ttex t2\t\n"
                              "\t\t\ttex t3\n"
                              "\t\t\tmul r0,v0,t0 ; blend vertex color into t0. \n"
                              "\t\t\tmul r1, t1, t2 ; mul\n"
                              "\t\t\tadd r0.rgb, r0, t3\n"
                              "\t\t\t+mul r0.a, r0, t3\n"
                              "\t\t\tadd r0.rgb, r0, r1\n";
            HRESULT error = D3DXAssembleShader(ps1, strlen(ps1), 0, nullptr, &shader, nullptr);

            if (error == 0) {
                error = DX8Wrapper::Get_D3D_Device8()->CreatePixelShader(
                    (DWORD *)shader->GetBufferPointer(), &m_riverWaterPixelShader);
                shader->Release();
            }

            const char *ps2 = "ps.1.1\n"
                              " \t\t\ttex t0 \n"
                              "\t\t\ttex t1\t\n"
                              "\t\t\ttexbem t2, t1 ; use t1 as env map adjustment on t2.\n"
                              "\t\t\tmul r0,v0,t0 ; blend vertex color into t0. \n"
                              "\t\t\tmul r1.rgb,t2,c0 ; reduce t2 (environment mapped reflection) by constant\n"
                              "\t\t\tadd r0.rgb, r0, r1";
            error = D3DXAssembleShader(ps2, strlen(ps2), 0, nullptr, &shader, nullptr);

            if (error == 0) {
                error = DX8Wrapper::Get_D3D_Device8()->CreatePixelShader(
                    (DWORD *)shader->GetBufferPointer(), &m_waterPixelShader);
                shader->Release();
            }

            const char *ps3 = "ps.1.1\n"
                              " \t\t\ttex t0 ;get water texture\n"
                              "\t\t\ttex t1 ;get white highlights on black background\n"
                              "\t\t\ttex t2 ;get white highlights with more tiling\n"
                              "\t\t\ttex t3\t; get black shroud \n"
                              "\t\t\tmul r0,v0,t0 ; blend vertex color and alpha into base texture. \n"
                              "\t\t\tmad r0.rgb, t1, t2, r0\t; blend sparkles and noise \n"
                              "\t\t\tmul r0.rgb, r0, t3 ; blend in black shroud \n"
                              "\t\t\t;\n";
            error = D3DXAssembleShader(ps3, strlen(ps3), 0, nullptr, &shader, nullptr);

            if (error == 0) {
                error = DX8Wrapper::Get_D3D_Device8()->CreatePixelShader(
                    (DWORD *)shader->GetBufferPointer(), &m_trapezoidWaterPixelShader);
                shader->Release();
            }
        }

        if (m_riverTexture) {
            if (!m_riverTexture->Is_Initialized()) {
                m_riverTexture->Init();
            }
        }

        if (m_waterNoiseTexture) {
            if (!m_waterNoiseTexture->Is_Initialized()) {
                m_waterNoiseTexture->Init();
            }
        }

        if (m_riverAlphaEdge) {
            if (!m_riverAlphaEdge->Is_Initialized()) {
                m_riverAlphaEdge->Init();
            }
        }

        if (m_waterSparklesTexture) {
            if (!m_waterSparklesTexture->Is_Initialized()) {
                m_waterSparklesTexture->Init();
            }
        }

        if (m_unkTexture) {
            if (!m_unkTexture->Is_Initialized()) {
                m_unkTexture->Init();
                SurfaceClass *surface = m_unkTexture->Get_Surface_Level(0);
                surface->Draw_Pixel(0, 0, 0xFFFFFFFF);
                Ref_Ptr_Release(surface);
            }
        }

        return;
    }
    if (WaterRenderObjClass::Generate_Index_Buffer(m_gridCellsX + 1, m_gridCellsY + 1) >= 0
        && WaterRenderObjClass::Generate_Vertex_Buffer(m_gridCellsX + 1, m_gridCellsY + 1, 32, 0) >= 0) {
        goto label1;
    }
#endif
}

void WaterRenderObjClass::Load()
{
    if (m_waterTrackSystem) {
        m_waterTrackSystem->Load_Tracks();
    }
}

int WaterRenderObjClass::Init(float water_level, float dx, float dy, SceneClass *parent_scene, WaterType type)
{
    m_iBumpFrame = 0;
    m_fBumpScale = 0.059999999f;
    m_dx = dx;
    m_dy = dy;
    m_level = water_level;
    m_lastUpdateTime = rts::Get_Time();
    m_uScrollPerMs = 0.001f;
    m_vScrollPerMs = 0.001f;
    m_uOffset = 0.0f;
    m_vOffset = 0.0f;
    m_parentScene = parent_scene;
    m_waterType = type;
    m_planeNormal = Vector3(0.0f, 0.0f, 1.0f);
    m_planeDistance = m_level;

    m_meshLight = new LightClass(LightClass::DIRECTIONAL);
    m_meshLight->Set_Ambient(Vector3(0.1f, 0.1f, 0.1f));
    m_meshLight->Set_Diffuse(Vector3(1.0f, 1.0f, 1.0f));
    m_meshLight->Set_Specular(Vector3(1.0f, 1.0f, 1.0f));
    m_meshLight->Set_Position(Vector3(1000.0f, 1000.0f, 1000.0f));
    m_meshLight->Set_Spot_Direction(Vector3(-0.56999999f, -0.56999999f, -0.56999999f));

    m_meshVertexMaterialClass = new VertexMaterialClass();
    m_meshVertexMaterialClass->Set_Shininess(20.0f);
    m_meshVertexMaterialClass->Set_Ambient(1.0f, 1.0f, 1.0f);
    m_meshVertexMaterialClass->Set_Diffuse(1.0f, 1.0f, 1.0f);
    m_meshVertexMaterialClass->Set_Specular(0.5f, 0.5f, 0.5f);
    m_meshVertexMaterialClass->Set_Opacity(0.5f);
    m_meshVertexMaterialClass->Set_Lighting(true);

    Load_Setting(&m_settings[1], TIME_OF_DAY_MORNING);
    Load_Setting(&m_settings[2], TIME_OF_DAY_AFTERNOON);
    Load_Setting(&m_settings[3], TIME_OF_DAY_EVENING);
    Load_Setting(&m_settings[4], TIME_OF_DAY_NIGHT);
    Set_Sort_Level(2);
    Set_Force_Visible(true);
    Re_Acquire_Resources();
    m_vertexMaterialClass = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    m_shaderClass = g_zFillAlphaShader;
    m_shaderClass.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    m_alphaClippingTexture = W3DAssetManager::Get_Instance()->Get_Texture("TSMoonLarg.tga");
    m_skyBox = ((GameAssetManager *)W3DAssetManager::Get_Instance())
                   ->Create_Render_Obj("new_skybox", g_theWriteableGlobalData->m_skyBoxScale, 0, nullptr, nullptr);

    if (m_skyBox && m_skyBox->Class_ID() == RenderObjClass::CLASSID_MESH) {
        MeshClass *mesh = (MeshClass *)m_skyBox;
        MaterialInfoClass *matinfo = mesh->Get_Material_Info();

        for (int i = 0; i < matinfo->Texture_Count(); i++) {
            if (matinfo->Peek_Texture(i)) {
                matinfo->Peek_Texture(i)->Get_Texture_Filter()->Set_U_Address_Mode(
                    TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                matinfo->Peek_Texture(i)->Get_Texture_Filter()->Set_V_Address_Mode(
                    TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            }
        }

        Ref_Ptr_Release(matinfo);
    }

    m_riverTexture = W3DAssetManager::Get_Instance()->Get_Texture(g_theWaterTransparency->m_standingWaterTexture);

    m_unkTexture = new TextureClass(1, 1, WW3D_FORMAT_A4R4G4B4, MIP_LEVELS_1, POOL_MANAGED, false, true);
    SurfaceClass *surface = m_unkTexture->Get_Surface_Level(0);
    surface->Draw_Pixel(0, 0, 0xFFFFFFFF);
    Ref_Ptr_Release(surface);

    m_waterNoiseTexture = W3DAssetManager::Get_Instance()->Get_Texture("Noise0000.tga");
    m_riverAlphaEdge = W3DAssetManager::Get_Instance()->Get_Texture("TWAlphaEdge.tga");
    m_waterSparklesTexture = W3DAssetManager::Get_Instance()->Get_Texture("WaterSurfaceBubbles.tga");
    m_waterTrackSystem = new WaterTracksRenderSystem();
    m_waterTrackSystem->Init();
    return 0;
}

void WaterRenderObjClass::Update_Map_Overrides()
{
    if (m_riverTexture) {
        if (g_theWaterTransparency->m_standingWaterTexture.Compare_No_Case(m_riverTexture->Get_Name())) {
            Ref_Ptr_Release(m_riverTexture);
            m_riverTexture = W3DAssetManager::Get_Instance()->Get_Texture(g_theWaterTransparency->m_standingWaterTexture);
        }
    }
}

void WaterRenderObjClass::Reset()
{
    if (m_meshData) {
        int x = m_gridCellsX + 1;
        int y = m_gridCellsY + 1;
        WaterMeshData *m = m_meshData;

        for (int i = 0; i < y + 2; i++) {
            for (int j = 0; j < x + 2; j++) {
                m->velocity = 0.0f;
                m->height = 0.0f;
                m->preferred_height = 0;
                m->status = 0;
                m++;
            }
        }

        m_meshInMotion = false;
    }

    if (m_waterTrackSystem) {
        m_waterTrackSystem->Reset();
    }
}

void WaterRenderObjClass::Enable_Water_Grid(bool state)
{
#ifdef BUILD_WITH_D3D8
    m_doWaterGrid = state;
    m_drawingRiver = false;
    m_disableRiver = false;

    if (state && !m_meshData) {
        m_meshDataCount = (m_gridCellsY + 3) * (m_gridCellsX + 3);
        m_meshData = new WaterMeshData[m_meshDataCount];
        memset(m_meshData, 0, m_meshDataCount * sizeof(WaterMeshData));
        Reset();

        if (m_vertexBufferD3D) {
            m_vertexBufferD3D->Release();
            m_vertexBufferD3D = nullptr;
        }

        if (m_indexBufferD3D) {
            m_indexBufferD3D->Release();
            m_indexBufferD3D = nullptr;
        }

        if (Generate_Index_Buffer(m_gridCellsX + 1, m_gridCellsY + 1) >= 0) {
            Generate_Vertex_Buffer(m_gridCellsX + 1, m_gridCellsY + 1, 32, false);
        }
    }
#endif
}

void WaterRenderObjClass::Update()
{
    unsigned int frame = 0;
    static unsigned int lastLogicFrame;

    if (g_theGameLogic) {
        frame = g_theGameLogic->Get_Frame();
    }

    m_riverVOrigin = m_riverVOrigin + 0.0020000001f;
    m_riverXOffset = m_riverXOffset + 0.000082500002f;
    m_riverYOffset = m_riverYOffset + 0.000165f;

    if (m_riverXOffset > 1.0) {
        m_riverXOffset = m_riverXOffset - 1.0;
    }

    if (m_riverYOffset > 1.0) {
        m_riverYOffset = m_riverYOffset - 1.0;
    }

    if (m_riverXOffset < -1.0) {
        m_riverXOffset = m_riverXOffset + 1.0;
    }

    if (m_riverYOffset < -1.0) {
        m_riverYOffset = m_riverYOffset + 1.0;
    }

    m_iBumpFrame++;

    if (m_iBumpFrame >= 32) {
        m_iBumpFrame = 0;
    }

    if (lastLogicFrame != frame) {
        if (m_doWaterGrid) {
            if (m_meshInMotion) {
                int x = m_gridCellsX + 1;
                int y = m_gridCellsY + 1;
                m_meshInMotion = false;
                WaterMeshData *m = m_meshData;

                for (int i = 0; i < y + 1; i++) {
                    for (int j = 0; j < x + 1; j++) {
                        if ((m->status & 1) != 0) {
                            m->velocity = 0.93000001f * m->velocity;

                            if (m->preferred_height <= m->height) {
                                m->velocity = g_theWriteableGlobalData->m_gravity * 3.0f + m->velocity;
                            } else {
                                m->velocity = m->velocity - g_theWriteableGlobalData->m_gravity * 3.0f;
                            }

                            m->height = m->height + m->velocity;

                            if (GameMath::Fabs(m->height - m->preferred_height) >= 1.0f
                                || GameMath::Fabs(m->velocity) >= 1.0f) {
                                m_meshInMotion = true;
                            } else {
                                m->status &= ~1;
                                m->height = m->preferred_height;
                                m->velocity = 0.0f;
                            }
                        }

                        m++;
                    }
                }
            }
        }
        lastLogicFrame = frame;
    }
}

void WaterRenderObjClass::Replace_Skybox_Texture(Utf8String const &oldname, Utf8String const &newname)
{
    ((GameAssetManager *)W3DAssetManager::Get_Instance())->Replace_Prototype_Texture(m_skyBox, oldname, newname);

    if (m_skyBox && m_skyBox->Class_ID() == RenderObjClass::CLASSID_MESH) {
        MeshClass *mesh = (MeshClass *)m_skyBox;
        MaterialInfoClass *matinfo = mesh->Get_Material_Info();

        for (int i = 0; i < matinfo->Texture_Count(); i++) {
            if (matinfo->Peek_Texture(i)) {
                matinfo->Peek_Texture(i)->Get_Texture_Filter()->Set_U_Address_Mode(
                    TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                matinfo->Peek_Texture(i)->Get_Texture_Filter()->Set_V_Address_Mode(
                    TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            }
        }

        Ref_Ptr_Release(matinfo);
    }
}

void WaterRenderObjClass::Set_Time_Of_Day(TimeOfDayType tod)
{
#ifdef BUILD_WITH_D3D8
    m_tod = tod;

    if (m_waterType == WATER_TYPE_2_PVSHADER) {
        Generate_Vertex_Buffer(15, 15, 24, true);
    }
#endif
}

void WaterRenderObjClass::Load_Setting(Setting *setting, TimeOfDayType tod)
{
    captainslog_assert(setting);

    setting->sky_texture = W3DAssetManager::Get_Instance()->Get_Texture(g_waterSettings[tod].m_skyTextureFile);
    setting->water_texture = W3DAssetManager::Get_Instance()->Get_Texture(g_waterSettings[tod].m_waterTextureFile);
    setting->sky_texels_per_unit = g_waterSettings[tod].m_skyTexelsPerUnit;
    SurfaceClass::SurfaceDescription surface_desc;
    setting->water_texture->Get_Level_Description(surface_desc, 0);
    setting->sky_texels_per_unit = setting->sky_texels_per_unit / surface_desc.width;
    setting->water_repeat_count = g_waterSettings[tod].m_waterRepeatCount;
    setting->u_scroll_per_ms = g_waterSettings[tod].m_uScrollPerMs;
    setting->v_scroll_per_ms = g_waterSettings[tod].m_vScrollPerMs;

    setting->vertex00_diffuse = g_waterSettings[tod].m_vertex00Diffuse.blue
        | (g_waterSettings[tod].m_vertex00Diffuse.green << 8) | (g_waterSettings[tod].m_vertex00Diffuse.red << 16);
    setting->vertex01_diffuse = g_waterSettings[tod].m_vertex01Diffuse.blue
        | (g_waterSettings[tod].m_vertex01Diffuse.green << 8) | (g_waterSettings[tod].m_vertex01Diffuse.red << 16);
    setting->vertex10_diffuse = g_waterSettings[tod].m_vertex10Diffuse.blue
        | (g_waterSettings[tod].m_vertex10Diffuse.green << 8) | (g_waterSettings[tod].m_vertex10Diffuse.red << 16);
    setting->vertex11_diffuse = g_waterSettings[tod].m_vertex11Diffuse.blue
        | (g_waterSettings[tod].m_vertex11Diffuse.green << 8) | (g_waterSettings[tod].m_vertex11Diffuse.red << 16);
    setting->water_diffuse = g_waterSettings[tod].m_waterDiffuseColor.blue
        | (g_waterSettings[tod].m_waterDiffuseColor.green << 8) | (g_waterSettings[tod].m_waterDiffuseColor.red << 16)
        | (g_waterSettings[tod].m_waterDiffuseColor.alpha << 24);
    setting->transparent_water_diffuse = g_waterSettings[tod].m_transparentWaterDiffuse.blue
        | (g_waterSettings[tod].m_transparentWaterDiffuse.green << 8)
        | (g_waterSettings[tod].m_transparentWaterDiffuse.red << 16)
        | (g_waterSettings[tod].m_transparentWaterDiffuse.alpha << 24);
}

void WaterRenderObjClass::Update_Render_Target_Textures(CameraClass *cam)
{
    if (m_waterType == WATER_TYPE_2_PVSHADER) {
        if (Get_Clipped_Water_Plane(cam, nullptr)) {
            if (g_theTerrainRenderObject) {
                if (g_theTerrainRenderObject->Get_Map()) {
                    Render_Mirror(cam);
                }
            }
        }
    }
}

void WaterRenderObjClass::Render_Mirror(CameraClass *cam)
{
#ifdef BUILD_WITH_D3D8
    Matrix3D m(cam->Get_Transform());
    Matrix4 m2(cam->Get_Transform());
    Vector3 v(0.0f, 0.0f, 0.0f);
    Vector4 v2(v.X, v.Y, v.Z, m_level);
    Vector3 v3;
    Vector3 v4;
    Vector3 v5;
    Vector3 v6;
    Matrix4 m3 = m2.Transpose();
    float f1 = ((Vector3 &)m3[0]) * v;
    float f2 = 2.0f / f1;
    v3 = ((Vector3 &)m3[0]) - (f2 * v);

    f1 = ((Vector3 &)m3[1]) * v;
    f2 = 2.0f / f1;
    v4 = ((Vector3 &)m3[1]) - (f2 * v);

    f1 = ((Vector3 &)m3[2]) * v;
    f2 = 2.0f / f1;
    v5 = ((Vector3 &)m3[2]) - (f2 * v);

    f1 = ((Vector3 &)m3[3]) * v;
    f2 = 2.0f / f1;
    v6 = ((Vector3 &)m3[3]) - (f2 * v);

    Matrix3D m4(v3, v4, v5, v6);
    DX8Wrapper::Set_Render_Target_With_Z(m_pReflectionTexture, nullptr);
    W3D::Begin_Render();
    cam->Set_Transform(m4);
    Vector2 v7;
    Vector2 v8;
    Vector2 v9;
    Vector2 v10;
    cam->Get_Viewport(v10, v9);
    v8.Y = 1.0f;
    v8.X = 1.0f;
    v7.Y = 0.0f;
    v7.X = 0.0f;
    cam->Set_Viewport(v7, v8);
    cam->Apply();
    ShaderClass::Invert_Backface_Culling(true);
    Render_Sky();

    if (m_tod == TIME_OF_DAY_NIGHT) {
        Render_Sky_Body(&m4);
    }

    W3D::Render(m_parentScene, cam);
    cam->Set_Transform(m);
    cam->Set_Viewport(v10, v9);
    cam->Apply();
    ShaderClass::Invert_Backface_Culling(false);
    W3D::End_Render(false);
    DX8Wrapper::Set_Render_Target(nullptr);
#endif
}

void WaterRenderObjClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (!g_theTerrainRenderObject || g_theTerrainRenderObject->Get_Map()) {
        if (((RTS3DScene *)rinfo.m_camera.Get_User_Data())->Get_Custom_Scene_Pass_Mode() != MODE_MASK) {
            if (((RTS3DScene *)rinfo.m_camera.Get_User_Data())->Get_Extra_Pass_Polygon_Mode()
                    != SceneClass::EXTRA_PASS_CLEAR_LINE
                && !ShaderClass::Is_Backface_Culling_Inverted()) {
                int sort = Get_Sort_Level();

                if (W3D::Are_Static_Sort_Lists_Enabled() && sort) {
                    W3D::Add_To_Static_Sort_List(this, sort);
                } else {
                    switch (m_waterType) {
                        case WATER_TYPE_0_TRANSLUCENT:
                        case WATER_TYPE_3_GRIDMESH: {
                            Render_Water();

                            if (!m_drawingRiver || m_disableRiver) {
                                Render_Water_Mesh();
                            }
                            break;
                        }
                        case WATER_TYPE_1_FB_REFLECTION: {
                            Matrix3D m(rinfo.m_camera.Get_Transform());
                            Matrix4 m2(rinfo.m_camera.Get_Transform());
                            Vector3 v(0.0f, 0.0f, 0.0f);
                            Vector4 v2(v.X, v.Y, v.Z, m_level);
                            Vector3 v3;
                            Vector3 v4;
                            Vector3 v5;
                            Vector3 v6;
                            Matrix4 m3 = m2.Transpose();
                            float f1 = ((Vector3 &)m3[0]) * v;
                            float f2 = 2.0f / f1;
                            v3 = ((Vector3 &)m3[0]) - (f2 * v);

                            f1 = ((Vector3 &)m3[1]) * v;
                            f2 = 2.0f / f1;
                            v4 = ((Vector3 &)m3[1]) - (f2 * v);

                            f1 = ((Vector3 &)m3[2]) * v;
                            f2 = 2.0f / f1;
                            v5 = ((Vector3 &)m3[2]) - (f2 * v);

                            f1 = ((Vector3 &)m3[3]) * v;
                            f2 = 2.0f / f1;
                            v6 = ((Vector3 &)m3[3]) - (f2 * v);

                            Matrix3D m4(v3, v4, v5, v6);
                            ShaderClass::Invert_Backface_Culling(true);
                            ShaderClass::Invert_Backface_Culling(false);
                            ShaderClass::Invalidate();
                            Render_Water();
                            break;
                        }
                        case WATER_TYPE_2_PVSHADER: {
                            Draw_Sea(rinfo);
                            break;
                        }
                    }

                    if (g_theWriteableGlobalData) {
                        if (g_theWriteableGlobalData->m_drawSkyBox != 0.0f) {
                            Vector3 v = rinfo.m_camera.Get_Position();
                            v.Z = g_theWriteableGlobalData->m_skyBoxPositionZ;
                            m_skyBox->Set_Position(v);
                            m_skyBox->Render(rinfo);
                        }
                    }

                    DX8Wrapper::Apply_Render_State_Changes();
                    DX8Wrapper::Invalidate_Cached_Render_States();

                    if (m_waterTrackSystem) {
                        m_waterTrackSystem->Flush(rinfo);
                    }
                }
            }
        }
    }
#endif
}

bool WaterRenderObjClass::Get_Clipped_Water_Plane(CameraClass *cam, AABoxClass *box)
{
    const FrustumClass &f = cam->Get_Frustum();
    ClipPolyClass poly;
    ClipPolyClass poly2;
    poly.Reset();
    poly.Add_Vertex(Vector3(0.0f, 0.0f, m_level));
    poly.Add_Vertex(Vector3(0.0f, m_dy, m_level));
    poly.Add_Vertex(Vector3(m_dx, m_dy, m_level));
    poly.Add_Vertex(Vector3(m_dx, 0.0f, m_level));
    poly.Clip(f.m_planes[0], poly2);
    poly2.Clip(f.m_planes[1], poly);
    poly.Clip(f.m_planes[2], poly2);
    poly2.Clip(f.m_planes[3], poly);
    poly.Clip(f.m_planes[4], poly2);
    poly2.Clip(f.m_planes[5], poly);
    int count = poly.m_verts.Count();

    if (count < 3) {
        return false;
    } else {
        if (box) {
            box->Init(&poly.m_verts[0], count);
        }

        return true;
    }
}

inline unsigned long F2DW(float f)
{
    return *((unsigned *)&f);
}

void WaterRenderObjClass::Draw_Sea(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    AABoxClass box;

    if (Get_Clipped_Water_Plane(&rinfo.m_camera, &box)) {
        D3DXMATRIX m;
        D3DXMATRIX m2;
        D3DXMATRIX m3;
        memset(&m3, 0, sizeof(m3));
        m3.m[0][0] = 1.0f;
        m3.m[2][1] = 1.0f;
        m3.m[1][2] = 1.0f;
        m3.m[3][3] = 1.0f;
        Matrix3D m4(m_transform);
        DX8Wrapper::Set_Transform(D3DTS_WORLD, m4);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Set_Texture(1, nullptr);
        DX8Wrapper::Apply_Render_State_Changes();
        Vector3 v;
        rinfo.m_camera.Get_Transform().Get_Translation(&v);
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, (Matrix4 &)m2);
        DX8Wrapper::Get_DX8_Transform(D3DTS_PROJECTION, (Matrix4 &)m);
        m_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        m_pDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        m_pDev->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pDev->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2);
        m_pDev->SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pDev->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 3);
        m_pDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        m_pDev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        m_pDev->SetRenderState(D3DRS_WRAP0, D3DWRAP_V | D3DWRAP_U);
        m_pDev->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        m_pDev->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        m_pDev->SetTexture(0, m_pBumpTexture[m_iBumpFrame]);
        m_pDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        m_pDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        m_pDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVMAT00, F2DW(m_fBumpScale));
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVMAT01, F2DW(0.0f));
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVMAT10, F2DW(0.0f));
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVMAT11, F2DW(m_fBumpScale));
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f));
        m_pDev->SetTextureStageState(1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f));
        m_pDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        D3DXMATRIX m5;
        m5.m[0][0] = 0.5f;
        m5.m[0][1] = -0.5f;
        m5.m[0][2] = 0.5f;
        m5.m[0][3] = 0.5f;
        m5.m[1][0] = 0.5f;
        m5.m[1][1] = 0.5f;
        m5.m[1][2] = 0.0f;
        m5.m[1][3] = 0.0f;
        m5.m[2][0] = 0.0f;
        m5.m[2][1] = 0.0f;
        m5.m[2][2] = 0.0f;
        m5.m[2][3] = 1.0f;
        m5.m[3][0] = 0.0f;
        m5.m[3][1] = 0.0f;
        m5.m[3][2] = 0.0f;
        m5.m[3][3] = 1.0f;
        m_pDev->SetVertexShaderConstant(6, &m5, 4);
        D3DXVECTOR4 v2(0.0f, 0.0f, 0.0f, 0.0f);
        m_pDev->SetVertexShaderConstant(0, (float *)v2, 1);
        D3DXVECTOR4 v3(1.0f, 1.0f, 1.0f, 1.0f);
        m_pDev->SetVertexShaderConstant(0, (float *)v3, 1);
        m_pDev->SetVertexShader(m_dwWaveVertexShader);
        m_pDev->SetPixelShader(m_dwWavePixelShader);
        m_pDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pDev->SetTexture(1, m_pReflectionTexture->Peek_Platform_Base_Texture());
        D3DXMATRIX m6;
        memset(&m6, 0, sizeof(m6));
        m6.m[0][0] = 40.0;
        m6.m[1][1] = 1.0;
        m6.m[2][2] = 40.0;
        m6.m[3][3] = 1.0;
        m_pDev->SetStreamSource(0, m_vertexBufferD3D, 24);
        m_pDev->SetIndices(m_indexBufferD3D, 0);

        for (int i = ((box.m_center.Y - box.m_extent.Y) / 560.0f); box.m_center.Y + box.m_extent.Y > (float)(14 * i) * 40.0f;
             i++) {
            for (int j = ((box.m_center.X - box.m_extent.X) / 560.0f);
                 box.m_center.X + box.m_extent.X > (float)(14 * j) * 40.0f;
                 j++) {
                D3DXMATRIX m7;
                D3DXMATRIX m8;
                D3DXMATRIX m9;
                m6.m[3][0] = (float)(14 * j) * 40.0f;
                m6.m[3][2] = (float)(14 * i) * 40.0f;
                D3DXMatrixMultiply(&m9, &m6, &m3);
                D3DXMatrixMultiply(&m8, &m9, &m2);
                D3DXMatrixMultiply(&m7, &m8, &m);
                D3DXMatrixTranspose(&m7, &m7);
                m_pDev->SetVertexShaderConstant(2, &m7, 4);
                m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_numVertices, 0, m_numIndices);
            }
        }

        m_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        m_pDev->SetTexture(0, nullptr);
        m_pDev->SetTexture(1, nullptr);
        m_pDev->SetTexture(2, nullptr);
        m_pDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        m_pDev->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pDev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        m_pDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        m_pDev->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        m_pDev->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        m_pDev->SetRenderState(D3DRS_WRAP0, 0);
        m_pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_pDev->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Transform(D3DTS_VIEW, (Matrix4 &)m2);
        DX8Wrapper::Set_DX8_Transform(D3DTS_PROJECTION, (Matrix4 &)m);
        m_pDev->SetPixelShader(0);
        m_pDev->SetVertexShader(DX8_FVF_XYZDUV1);
        DX8Wrapper::Invalidate_Cached_Render_States();

        if (g_theTerrainRenderObject->Get_Shroud()) {
            W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
            W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
            m_pDev->SetStreamSource(0, m_vertexBufferD3D, 24);
            m_pDev->SetIndices(m_indexBufferD3D, 0);

            for (int i = ((box.m_center.Y - box.m_extent.Y) / 560.0f);
                 box.m_center.Y + box.m_extent.Y > (float)(14 * i) * 40.0f;
                 i++) {
                for (int j = ((box.m_center.X - box.m_extent.X) / 560.0f);
                     box.m_center.X + box.m_extent.X > (float)(14 * j) * 40.0f;
                     j++) {
                    D3DXMATRIX m10;
                    m10.m[3][0] = (float)(14 * j) * 40.0f;
                    m10.m[3][2] = (float)(14 * i) * 40.0f;
                    D3DXMatrixMultiply(&m10, &m6, &m3);
                    DX8Wrapper::Set_DX8_Transform(D3DTS_WORLD, (Matrix4 &)m10);
                    m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_numVertices, 0, m_numIndices);
                }
            }

            W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
        }
    }
#endif
}

void WaterRenderObjClass::Render_Water()
{
    for (PolygonTrigger *i = PolygonTrigger::Get_First_Polygon_Trigger(); i; i = i->Get_Next()) {
        if (i->Is_Water_Area()) {
            if (i->Get_Num_Points() > 2) {
                if (i->Is_River()) {
                    Draw_River_Water(i);
                } else {
                    for (int j = 1; j < i->Get_Num_Points() - 1; j++) {
                        ICoord3D p1 = *i->Get_Point(0);
                        ICoord3D p2 = *i->Get_Point(j);
                        ICoord3D p3 = *i->Get_Point(j + 1);
                        ICoord3D p4 = *i->Get_Point(j + 1);

                        if (j + 2 < i->Get_Num_Points()) {
                            ICoord3D p4 = *i->Get_Point(j + 2);
                        }

                        Vector3 v[4];
                        v[0].Set(p4.x, p4.y, p4.z);
                        v[1].Set(p3.x, p3.y, p3.z);
                        v[2].Set(p2.x, p2.y, p2.z);
                        v[3].Set(p1.x, p1.y, p1.z);

                        if (g_theWriteableGlobalData->m_featherWater) {
                            for (int k = 0; k < g_theWriteableGlobalData->m_featherWater; k++) {
                                Draw_Trapezoid_Water(v);
                                v[0].Z = 4.0f / (float)g_theWriteableGlobalData->m_featherWater + v[0].Z;
                            }
                        } else {
                            Draw_Trapezoid_Water(v);
                        }
                    }
                }
            }
        }
    }
}

void WaterRenderObjClass::Render_Sky()
{
#ifdef BUILD_WITH_D3D8
    Setting *s = &m_settings[m_tod];
    int newtime = rts::Get_Time();
    int time = newtime - m_lastUpdateTime;
    m_lastUpdateTime = newtime;
    m_uOffset = (float)time * s->u_scroll_per_ms * s->sky_texels_per_unit + m_uOffset;
    m_vOffset = (float)time * s->v_scroll_per_ms * s->sky_texels_per_unit + m_vOffset;
    m_uOffset = m_uOffset - (float)(int)m_uOffset;
    m_vOffset = m_vOffset - (float)(int)m_vOffset;
    float u = 7680.0 * s->sky_texels_per_unit + m_uOffset;
    float v = 7680.0 * s->sky_texels_per_unit + m_vOffset;
    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);
    ShaderClass shader(ShaderClass::s_presetOpaqueShader);
    shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
    DX8Wrapper::Set_Shader(shader);
    DX8Wrapper::Set_Texture(0, s->sky_texture);
    DynamicVBAccessClass dyn_vb_access(
        VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE), 4);

    {
        DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
        VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();

        if (dest_verts) {
            dest_verts[0].x = -3840.0f;
            dest_verts[0].y = 3840.0f;
            dest_verts[0].z = 30.0f;
            dest_verts[0].u1 = m_uOffset;
            dest_verts[0].v1 = v;
            dest_verts[0].diffuse = s->vertex01_diffuse;
            dest_verts[1].x = 3840.0f;
            dest_verts[1].y = 3840.0f;
            dest_verts[1].z = 30.0f;
            dest_verts[1].u1 = u;
            dest_verts[1].v1 = v;
            dest_verts[1].diffuse = s->vertex11_diffuse;
            dest_verts[2].x = 3840.0f;
            dest_verts[2].y = -3840.0f;
            dest_verts[2].z = 30.0f;
            dest_verts[2].u1 = u;
            dest_verts[2].v1 = m_vOffset;
            dest_verts[2].diffuse = s->vertex10_diffuse;
            dest_verts[3].x = -3840.0f;
            dest_verts[3].y = -3840.0f;
            dest_verts[3].z = 30.0f;
            dest_verts[3].u1 = m_uOffset;
            dest_verts[3].v1 = m_vOffset;
            dest_verts[3].diffuse = s->vertex00_diffuse;
        }
    }

    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
    DX8Wrapper::Set_Vertex_Buffer(dyn_vb_access);
    Matrix3D m(true);
    m.Set_Translation(Vector3(0.0f, 0.0f, 0.0f));
    DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
    DX8Wrapper::Draw_Triangles(0, 2, 0, 4);
#endif
}

void WaterRenderObjClass::Render_Sky_Body(Matrix3D *mat)
{
#ifdef BUILD_WITH_D3D8
    Vector3 v1;
    Vector3 v2;
    Vector3 v3;
    Vector3 v4;
    Vector3 v5(150.0f, 550.0f, 30.0f);
    mat->Get_Translation(&v1);
    v2 = v1 - v5;
    v2.Normalize();
    Vector3 v6(0.0f, 0.0f, -1.0f);
    Vector3 v7;
    Vector3::Normalized_Cross_Product(v6, v2, &v7);
    Matrix3D m(true);
    m.Set(v7, GameMath::Acos(v6 * v2));
    m.Adjust_Translation(Vector3(150.0f, 550.0f, 30.0f));
    DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);
    ShaderClass shader(ShaderClass::s_presetOpaqueShader);
    shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
    DX8Wrapper::Set_Shader(shader);
    DX8Wrapper::Set_Texture(0, m_alphaClippingTexture);
    DynamicVBAccessClass dyn_vb_access(
        VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE), 4);

    {
        DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
        VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();

        if (dest_verts) {
            dest_verts[0].x = -45.0f;
            dest_verts[0].y = 45.0f;
            dest_verts[0].z = 0.0f;
            dest_verts[0].u2 = 0.0f;
            dest_verts[0].v2 = 1.0f;
            dest_verts[0].diffuse = 0xFFFFFFFF;
            dest_verts[1].x = 45.0f;
            dest_verts[1].y = 45.0f;
            dest_verts[1].z = 0.0f;
            dest_verts[1].u2 = 1.0f;
            dest_verts[1].v2 = 1.0f;
            dest_verts[1].diffuse = 0xFFFFFFFF;
            dest_verts[2].x = 45.0f;
            dest_verts[2].y = -45.0f;
            dest_verts[2].z = 0.0f;
            dest_verts[2].u2 = 1.0f;
            dest_verts[2].v2 = 0.0f;
            dest_verts[2].diffuse = 0xFFFFFFFF;
            dest_verts[3].x = -45.0f;
            dest_verts[3].y = -45.0f;
            dest_verts[3].z = 0.0f;
            dest_verts[3].u2 = 0.0f;
            dest_verts[3].v2 = 0.0f;
            dest_verts[3].diffuse = 0xFFFFFFFF;
        }
    }
    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
    DX8Wrapper::Set_Vertex_Buffer(dyn_vb_access);
    DX8Wrapper::Draw_Triangles(0, 2, 0, 4);
#endif
}

void WaterRenderObjClass::Render_Water_Mesh()
{
#ifdef BUILD_WITH_D3D8
    if (m_doWaterGrid) {
        m_vertexBufferD3DOffset = 0xFFFF;
        Setting *s = &m_settings[m_tod];
        int x = m_gridCellsX + 1;
        int y = m_gridCellsY + 1;
        float u = (float)s->water_repeat_count / 128.0f * m_gridCellSize / 10.0f * 0.2f;
        float v = (float)s->water_repeat_count / 128.0f * m_gridCellSize / 10.0f * 0.2f;
        Vector3 v1(m_gridCellSize * m_gridCellSize, 0.0f, 0.0f);
        Vector3 v2(0.0f, m_gridCellSize * m_gridCellSize, 0.0f);
        Vector3 v3;
        VertexFormatXYZDUV2 *verts;

        if (m_vertexBufferD3DOffset > m_numVertices) {
            if (m_vertexBufferD3D->Lock(0, 32 * y * x, (BYTE **)&verts, D3DLOCK_DISCARD)) {
                return;
            }
            m_vertexBufferD3DOffset = 0;
        } else if (m_vertexBufferD3D->Lock(32 * m_vertexBufferD3DOffset, 32 * y * x, (BYTE **)&verts, D3DLOCK_NOOVERWRITE)) {
            return;
        }

        unsigned int color = s->water_diffuse & 0xFFFFFF;
        unsigned int color2 = (s->water_diffuse & 0xFF000000) >> 24;
        color2 -= 32;
        color |= color2 << 24;
        float f1 = GameMath::Cos(3.0f * m_riverVOrigin) * 0.02f;
        float f2 = 25.0f * m_riverVOrigin;
        float f3 = m_riverVOrigin / v;
        WaterMeshData *mesh = &m_meshData[x + 3];

        for (int i = 0; i < y; i++) {
            float f4 = (float)i * m_gridCellSize;
            float f5 = (float)i * v + m_riverVOrigin;
            float f6 = f4 * 3.1415927f / 80.0f + f2;
            float f7 = GameMath::Fast_Sin(f6) * f1 + f5;
            float f8 = 0.30000001f * m_gridCellSize / 50.0f;
            float f9 = m_gridCellSize / 50.0f;
            float f10 = ((float)i + f3) * f9 + (float)i * f8;

            for (int j = 0; j < x; j++) {
                verts->x = (float)j * m_gridCellSize;
                verts->y = f4;
                verts->z = mesh->height;
                verts->diffuse = color;
                verts->u1 = (float)j * u;
                verts->v1 = f7;
                verts->u2 = (float)j * m_gridCellSize / 50.0f;
                verts->v2 = f10;
                verts++;
                mesh++;
            }

            mesh += 2;
        }

        m_vertexBufferD3D->Unlock();
        Matrix3D m(m_transform);
        DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
        DX8Wrapper::Set_Material(m_meshVertexMaterialClass);
        ShaderClass::CullModeType cull = m_shaderClass.Get_Cull_Mode();
        ShaderClass::DepthMaskType mask = m_shaderClass.Get_Depth_Mask();
        m_shaderClass.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
        m_shaderClass.Set_Cull_Mode(ShaderClass::CULL_MODE_ENABLE);
        DX8Wrapper::Set_Shader(m_shaderClass);
        Setup_Flat_Water_Shader();
        m_pDev->SetIndices(m_indexBufferD3D, m_vertexBufferD3DOffset);
        m_pDev->SetStreamSource(0, m_vertexBufferD3D, 32);
        m_pDev->SetVertexShader(DX8_FVF_XYZDUV2);

        if (!g_theTerrainRenderObject->Get_Shroud() || m_trapezoidWaterPixelShader) {
            m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, y * x, 0, m_numIndices - 2);
        } else {
            W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
            W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
            m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, y * x, 0, m_numIndices - 2);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
            W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
        }

        if (m_trapezoidWaterPixelShader) {
            DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
        }

        m_vertexBufferD3DOffset += y * x;
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Set_Texture(1, nullptr);
        ShaderClass::Invalidate();
        m_shaderClass.Set_Cull_Mode(cull);
        m_shaderClass.Set_Depth_Mask(mask);
    }
#endif
}

void WaterRenderObjClass::Set_Grid_Height_Clamps(float min_z, float max_z)
{
    m_minGridHeight = min_z;
    m_maxGridHeight = max_z;
}

void WaterRenderObjClass::Add_Velocity(float world_x, float world_y, float z_velocity, float preferred_height)
{
    if (m_doWaterGrid) {
        m_disableRiver = true;
        float gx;
        float gy;

        if (World_To_Grid_Space(world_x, world_y, gx, gy)) {
            float minx = GameMath::Floor(gx - m_gridChangeMaxRange);

            if (minx < 0.0f) {
                minx = 0.0f;
            }

            float maxx = GameMath::Ceil(gx + m_gridChangeMaxRange);

            if (m_gridCellsX < maxx) {
                maxx = m_gridCellsX;
            }

            float miny = GameMath::Floor(gy - m_gridChangeMaxRange);

            if (miny < 0.0f) {
                miny = 0.0f;
            }

            float maxy = GameMath::Ceil(gy + m_gridChangeMaxRange);

            if (m_gridCellsY < maxy) {
                maxy = m_gridCellsY;
            }

            for (int i = miny; i < maxy; i++) {
                for (int j = minx; j < maxx; j++) {
                    WaterMeshData *mesh = &m_meshData[(m_gridCellsX + 3) * (i + 1) + 1 + j];
                    mesh->preferred_height = preferred_height;
                    mesh->velocity = z_velocity + mesh->velocity;
                    mesh->status |= 1;
                }
            }

            m_meshInMotion = true;
        }
    }
}

void WaterRenderObjClass::Change_Grid_Height(float wx, float wy, float delta)
{
    float gx;
    float gy;

    if (World_To_Grid_Space(wx, wy, gx, gy)) {
        float minx = GameMath::Floor(gx - m_gridChangeMaxRange);

        if (minx < 0.0f) {
            minx = 0.0f;
        }

        float maxx = GameMath::Ceil(gx + m_gridChangeMaxRange);

        if (m_gridCellsX < maxx) {
            maxx = m_gridCellsX;
        }

        float miny = GameMath::Floor(gy - m_gridChangeMaxRange);

        if (miny < 0.0f) {
            miny = 0.0f;
        }

        float maxy = GameMath::Ceil(gy + m_gridChangeMaxRange);

        if (m_gridCellsY < maxy) {
            maxy = m_gridCellsY;
        }

        for (int i = miny; i < maxy; i++) {
            for (int j = minx; j < maxx; j++) {
                WaterMeshData *mesh = &m_meshData[(m_gridCellsX + 3) * (i + 1) + 1 + j];
                float f1 = GameMath::Sqrt((gx - j) * (gx - j) + (gy - i) * (gy - i));
                float f2 =
                    1.0f / (f1 * m_gridChangeAtt1 + m_gridChangeAtt0 + f1 * f1 * m_gridChangeAtt2) * delta + mesh->height;

                if (f2 < m_minGridHeight) {
                    f2 = m_minGridHeight;
                }

                if (f2 > m_maxGridHeight) {
                    f2 = m_maxGridHeight;
                }

                mesh->height = f2;
            }
        }
    }
}

void WaterRenderObjClass::Set_Grid_Change_Attenuation_Factors(float a, float b, float c, float range)
{
    m_gridChangeAtt0 = a;
    m_gridChangeAtt1 = b;
    m_gridChangeAtt2 = c;
    m_gridChangeMaxRange = range / m_gridCellSize;
}

void WaterRenderObjClass::Set_Grid_Transform(float angle, float x, float y, float z)
{
    m_gridDirectionX = Vector2(1.0f, 0.0f);
    m_gridOrigin.X = x;
    m_gridOrigin.Y = y;
    Matrix3D m(true);
    m.Rotate_Z(angle);
    m_gridDirectionX.X = m.Get_X_Vector().X;
    m_gridDirectionX.Y = m.Get_X_Vector().Y;
    m_gridDirectionY.X = m.Get_Y_Vector().X;
    m_gridDirectionY.Y = m.Get_Y_Vector().Y;
    m.Set_Translation(Vector3(x, y, z));
    Set_Transform(m);
}

void WaterRenderObjClass::Set_Grid_Transform(Matrix3D const *transform)
{
    if (transform) {
        Set_Transform(*transform);
    }
}

void WaterRenderObjClass::Get_Grid_Transform(Matrix3D *transform)
{
    if (transform) {
        *transform = Get_Transform();
    }
}

void WaterRenderObjClass::Set_Grid_Resolution(float grid_cells_x, float grid_cells_y, float cell_size)
{
    m_gridCellSize = cell_size;

    if (m_gridCellsX != grid_cells_x) {
        m_gridCellsX = grid_cells_x;
        m_gridCellsY = grid_cells_y;

        if (m_meshData) {
            delete[] m_meshData;
            m_meshData = nullptr;
            m_meshDataCount = 0;
            bool b = m_doWaterGrid;
            Enable_Water_Grid(true);
            m_doWaterGrid = b;
        }
    }
}

void WaterRenderObjClass::Get_Grid_Resolution(float *grid_cells_x, float *grid_cells_y, float *cell_size)
{
    if (grid_cells_x) {
        *grid_cells_x = m_gridCellsX;
    }

    if (grid_cells_y) {
        *grid_cells_y = m_gridCellsY;
    }

    if (cell_size) {
        *cell_size = m_gridCellSize;
    }
}

float WaterRenderObjClass::Get_Water_Height(float x, float y)
{
    const WaterHandle *h = nullptr;
    float f = 0.0f;
    ICoord3D c;
    c.x = GameMath::Fast_To_Int_Floor(x + 0.5f);
    c.y = GameMath::Fast_To_Int_Floor(y + 0.5f);
    c.z = 0;

    for (PolygonTrigger *i = PolygonTrigger::Get_First_Polygon_Trigger(); i; i = i->Get_Next()) {
        if (i->Is_Water_Area()) {
            if (i->Point_In_Trigger(c) && i->Get_Point(0)->z >= f) {
                f = i->Get_Point(0)->z;
                h = i->Get_Water_Handle();
            }
        }
    }

    if (h) {
        return h->m_polygon->Get_Point(0)->z;
    } else {
        return 0.0f;
    }
}

bool WaterRenderObjClass::World_To_Grid_Space(float wx, float wy, float &gx, float &gy)
{
    float f1 = 1.0 / m_gridCellSize;
    float f2 = wx - m_gridOrigin.X;
    float f3 = wy - m_gridOrigin.Y;
    gx = (f2 * m_gridDirectionX.X + f3 * m_gridDirectionX.Y) * f1;
    gy = (f2 * m_gridDirectionY.X + f3 * m_gridDirectionY.Y) * f1;

    if (gx < 0.0f) {
        return false;
    }

    if (m_gridCellsX - 1 < gx) {
        return false;
    }

    if (gy < 0.0f) {
        return false;
    }

    if (m_gridCellsY - 1 < gx) {
        return false;
    }

    return true;
}

void WaterRenderObjClass::Draw_River_Water(PolygonTrigger *ptrig)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Invalidate_Cached_Render_States();
    int count = ptrig->Get_Num_Points() / 2 - 1;

    if (!m_disableRiver) {
        m_drawingRiver = true;
        DynamicIBAccessClass dyn_ib_access(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 3 * (2 * count + 2));

        {
            DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
            unsigned short *indices = lock.Get_Index_Array();

            for (int i = 0; i < count; i++) {
                indices[0] = 2 * i;
                indices[1] = 2 * i + 1;
                indices[2] = 2 * i + 3;
                indices[3] = 2 * i;
                indices[4] = 2 * i + 3;
                indices[5] = 2 * i + 2;
                indices += 6;
            }
        }

        float red = g_theWaterTransparency->m_standingWaterColor.red;
        float green = g_theWaterTransparency->m_standingWaterColor.green;
        float blue = g_theWaterTransparency->m_standingWaterColor.blue;

        if (red == 1.0f && green == 1.0f && blue == 1.0f) {
            red = g_theWriteableGlobalData->m_terrainAmbient[0].red;
            green = g_theWriteableGlobalData->m_terrainAmbient[0].green;
            blue = g_theWriteableGlobalData->m_terrainAmbient[0].blue;

            for (int l = 0; l < g_theWriteableGlobalData->m_numberGlobalLights; l++) {
                if (-g_theWriteableGlobalData->m_terrainLightPos[l].z > 0.0f) {
                    red =
                        -g_theWriteableGlobalData->m_terrainLightPos[l].z * g_theWriteableGlobalData->m_terrainDiffuse[l].red
                        + red;
                    green = -g_theWriteableGlobalData->m_terrainLightPos[l].z
                            * g_theWriteableGlobalData->m_terrainDiffuse[l].green
                        + green;
                    blue = -g_theWriteableGlobalData->m_terrainLightPos[l].z
                            * g_theWriteableGlobalData->m_terrainDiffuse[l].blue
                        + blue;
                }
            }
            int r1 = (m_settings[m_tod].water_diffuse & 0xFF);
            float r2 = r1 / 255.0f;
            int g1 = ((m_settings[m_tod].water_diffuse >> 8) & 0xFF);
            float g2 = g1 / 255.0f;
            int b1 = ((m_settings[m_tod].water_diffuse >> 16) & 0xFF);
            float b2 = b1 / 255.0f;
            red = red * r2 * 255.0f;
            green = green * g2 * 255.0f;
            blue = blue * b2 * 255.0f;
        } else {
            red = red * 255.0f;
            green = green * 255.0f;
            blue = blue * 255.0f;

            if (red == 0.0f && green == 0.0f && blue == 0.0f) {
                red = 255.0f;
                green = 255.0f;
                blue = 255.0f;
            }
        }

        int color = (GameMath::Fast_To_Int_Truncate(red) << 16) | (GameMath::Fast_To_Int_Truncate(green) << 8)
            | GameMath::Fast_To_Int_Truncate(blue);
        color |= m_settings[m_tod].water_diffuse & 0xFF000000;

        int start = ptrig->Get_River_Start();
        int start2 = start + 1;
        float f1 = 0.0f;
        float f2 = 0.0f;

        for (int i = 0; i < ptrig->Get_Num_Points() - 1; i++) {
            ICoord3D p1 = *ptrig->Get_Point(i);
            ICoord3D p2 = *ptrig->Get_Point(i + 1);
            float f3 = p1.x - p2.x;
            float f4 = p1.y - p2.y;
            float f5 = sqrt(f4 * f4 + f3 * f3);
            f2 = f2 + f5;

            if (i == start) {
                f1 = f5;
            }
        }

        float f6 = f2 / 2.0f - f1;
        float f7 = (f6 / f1) / (float)count;

        if (start < ptrig->Get_Num_Points() - 1) {
            DynamicVBAccessClass dyn_vb_access(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
                (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE),
                2 * count + 2);

            {
                DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
                VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();
                float f8 = 3.0f * m_riverVOrigin;

                for (int i = 0; i < ptrig->Get_Num_Points() / 2; i++) {
                    ICoord3D p1 = *ptrig->Get_Point(start2);
                    ICoord3D p2 = *ptrig->Get_Point(start);
                    start2++;
                    start--;

                    if (start < 0) {
                        start = ptrig->Get_Num_Points() - 1;
                    }

                    if (start2 >= ptrig->Get_Num_Points()) {
                        start2 = 0;
                    }

                    dest_verts->x = p1.x;
                    dest_verts->y = p1.y;
                    dest_verts->z = p1.z;
                    dest_verts->diffuse = color;
                    float v = GameMath::Fast_Sin(i * f7 * 6.2831855f - f8) / 22.0f + i * f7 - m_riverVOrigin;
                    dest_verts->v1 = v;
                    dest_verts->u1 = 0.5f;
                    dest_verts->v2 = v;
                    dest_verts->u2 = 1.0f;
                    dest_verts->nx = 0.0f;
                    dest_verts->ny = 0.0f;
                    dest_verts->nz = 1.0f;
                    dest_verts++;

                    dest_verts->x = p1.x;
                    dest_verts->y = p1.y;
                    dest_verts->z = p1.z;
                    dest_verts->diffuse = color;
                    dest_verts->v1 = v;
                    dest_verts->u1 = 0.0f;
                    dest_verts->v2 = v;
                    dest_verts->u2 = 0.0f;
                    dest_verts->nx = 0.0f;
                    dest_verts->ny = 0.0f;
                    dest_verts->nz = 1.0f;
                    dest_verts++;
                }
            }

            Matrix3D m(true);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
            DX8Wrapper::Set_Index_Buffer(dyn_ib_access, 0);
            DX8Wrapper::Set_Vertex_Buffer(dyn_vb_access);
            DX8Wrapper::Set_Texture(0, m_riverTexture);
            Setup_Jba_Water_Shader();

            if (g_theWaterTransparency->m_additiveBlending) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            }

            if (m_riverWaterPixelShader) {
                DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_riverWaterPixelShader);
            }

            DWORD cull;
            DX8Wrapper::Get_D3D_Device8()->GetRenderState(D3DRS_CULLMODE, &cull);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            DX8Wrapper::Draw_Triangles(0, 2 * count, 0, 2 * count + 2);

            if (m_riverWaterPixelShader) {
                DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
            }

            if (g_theWaterTransparency->m_additiveBlending) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ONE);
            }

            if (g_theTerrainRenderObject->Get_Shroud()) {
                W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
                W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
                DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
                DX8Wrapper::Draw_Triangles(0, 2 * count, 0, 2 * count + 2);
                W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
                DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
            }

            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, cull);
        }
    }
#endif
}

void WaterRenderObjClass::Setup_Flat_Water_Shader()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Texture(0, m_riverTexture);

    if (g_theWaterTransparency->m_additiveBlending) {
        DX8Wrapper::Set_Shader(ShaderClass::s_presetAdditiveShader);
    } else {
        DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);
    }

    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);

    m_riverTexture->Get_Texture_Filter()->Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    m_riverTexture->Get_Texture_Filter()->Set_Min_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    m_riverTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_BEST);
    DX8Wrapper::Apply_Render_State_Changes();

    if (m_trapezoidWaterPixelShader) {
        if (g_theTerrainRenderObject->Get_Shroud()) {
            W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
            W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 3);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        } else {
            if (!m_unkTexture->Is_Initialized()) {
                m_unkTexture->Init();
                SurfaceClass *surface = m_unkTexture->Get_Surface_Level(0);
                surface->Draw_Pixel(0, 0, 0xFFFFFFFF);
                Ref_Ptr_Release(surface);
            }

            DX8Wrapper::Get_D3D_Device8()->SetTexture(3, m_unkTexture->Peek_Platform_Base_Texture());
        }
    }

    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);

    if (m_trapezoidWaterPixelShader) {
        if (!m_waterSparklesTexture->Is_Initialized()) {
            m_waterSparklesTexture->Init();
        }

        DX8Wrapper::Get_D3D_Device8()->SetTexture(1, m_waterSparklesTexture->Peek_Platform_Base_Texture());

        if (!m_waterNoiseTexture->Is_Initialized()) {
            m_waterNoiseTexture->Init();
        }

        DX8Wrapper::Get_D3D_Device8()->SetTexture(2, m_waterNoiseTexture->Peek_Platform_Base_Texture());
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        D3DXMATRIX m;
        D3DXMATRIX m2;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, (Matrix4 &)m2);
        float f;
        D3DXMatrixInverse(&m, &f, &m2);
        D3DXMATRIX m3;
        D3DXMatrixScaling(&m3, 0.0625f, 0.0625f, 1.0f);
        D3DXMATRIX m4 = m * m3;
        D3DXMatrixTranslation(&m3, m_riverVOrigin, m_riverVOrigin, 0.0f);
        m4 = m4 * m3;
        DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE2, (Matrix4 &)m4);
    }

    m_pDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(1u, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(1u, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(2u, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(2u, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

    if (m_trapezoidWaterPixelShader) {
        DX8Wrapper::Get_D3D_Device8()->SetPixelShaderConstant(0, (float *)D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f), 1);
        DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_trapezoidWaterPixelShader);
    }
#endif
}

void WaterRenderObjClass::Draw_Trapezoid_Water(Vector3 *const points)
{
#ifdef BUILD_WITH_D3D8
    Vector3 v1(points[0]);
    Vector3 v2(points[1]);
    Vector3 v3(points[3]);
    Vector3 v4(points[2]);
    Vector3 v5(points[2]);
    v4 -= v3;
    v5 -= v2;
    v2 -= v1;
    v3 -= v1;
    int i1 = (v2.Length() + v4.Length()) / 80.0f;

    if (i1 < 1) {
        i1 = 1;
    }

    int i2 = (v3.Length() + v5.Length()) / 80.0f;

    if (i2 < 1) {
        i2 = 1;
    }

    if (i1 > 50) {
        i1 = 50;
    }

    if (i2 > 50) {
        i2 = 50;
    }

    int i3 = i2 * i1;
    i1++;
    i2++;
    DynamicIBAccessClass dyn_ib_access(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 3 * (2 * i3 + 2));

    {
        DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
        unsigned short *indices = lock.Get_Index_Array();

        for (int i = 0; i < i2 - 1; i++) {
            for (int j = 0; j < i1 - 1; j++) {
                indices[0] = j + i1 * i;
                indices[1] = i1 * (i + 1) + j + 1;
                indices[2] = j + i1 * (i + 1);
                indices[3] = j + i1 * i;
                indices[4] = i1 * i + j + 1;
                indices[5] = i1 * (i + 1) + j + 1;
                indices += 6;
            }
        }
    }

    float red = g_theWaterTransparency->m_standingWaterColor.red;
    float green = g_theWaterTransparency->m_standingWaterColor.green;
    float blue = g_theWaterTransparency->m_standingWaterColor.blue;

    if (red == 1.0f && green == 1.0f && blue == 1.0f) {
        red = g_theWriteableGlobalData->m_terrainAmbient[0].red;
        green = g_theWriteableGlobalData->m_terrainAmbient[0].green;
        blue = g_theWriteableGlobalData->m_terrainAmbient[0].blue;

        for (int l = 0; l < g_theWriteableGlobalData->m_numberGlobalLights; l++) {
            if (-g_theWriteableGlobalData->m_terrainLightPos[l].z > 0.0f) {
                red = -g_theWriteableGlobalData->m_terrainLightPos[l].z * g_theWriteableGlobalData->m_terrainDiffuse[l].red
                    + red;
                green =
                    -g_theWriteableGlobalData->m_terrainLightPos[l].z * g_theWriteableGlobalData->m_terrainDiffuse[l].green
                    + green;
                blue = -g_theWriteableGlobalData->m_terrainLightPos[l].z * g_theWriteableGlobalData->m_terrainDiffuse[l].blue
                    + blue;
            }
        }

        int r1 = (m_settings[m_tod].water_diffuse & 0xFF);
        float r2 = r1 / 255.0f;
        int g1 = ((m_settings[m_tod].water_diffuse >> 8) & 0xFF);
        float g2 = g1 / 255.0f;
        int b1 = ((m_settings[m_tod].water_diffuse >> 16) & 0xFF);
        float b2 = b1 / 255.0f;
        red = red * r2 * 255.0f;
        green = green * g2 * 255.0f;
        blue = blue * b2 * 255.0f;
    } else {
        red = red * 255.0f;
        green = green * 255.0f;
        blue = blue * 255.0f;

        if (red == 0.0f && green == 0.0f && blue == 0.0f) {
            red = 255.0f;
            green = 255.0f;
            blue = 255.0f;
        }
    }

    int color = (GameMath::Fast_To_Int_Truncate(red) << 16) | (GameMath::Fast_To_Int_Truncate(green) << 8)
        | GameMath::Fast_To_Int_Truncate(blue);
    color |= m_settings[m_tod].water_diffuse & 0xFF000000;

    DynamicVBAccessClass dyn_vb_access(
        VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE), 2 * i3 + 2);

    if (g_theWriteableGlobalData->m_featherWater) {
        {
            DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
            VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();
            float f1 = 0.0f;
            float f2 = 0.078539819f;
            float f3 = 0.0f;
            float f4 = 0.5f;
            int i4 = 0;

            if (g_theWriteableGlobalData->m_featherWater == 5) {
                i4 = 80;
            }

            if (g_theWriteableGlobalData->m_featherWater == 4) {
                i4 = 110;
            }

            if (g_theWriteableGlobalData->m_featherWater == 3) {
                i4 = 140;
            }

            if (g_theWriteableGlobalData->m_featherWater == 2) {
                i4 = 200;
            }

            if (g_theWriteableGlobalData->m_featherWater == 1) {
                i4 = 255;
            }

            int color2 = (i4 << 24) | color & 0xFFFFFF;

            for (int i = 0; i < i2; i++) {
                float f5 = i;
                f5 = f5 / (float)(i2 - 1);

                for (int j = 0; j < i1; j++) {
                    float f6 = j;
                    f6 = f6 / (float)(i1 - 1);
                    Vector3 v6(v1);
                    v6 += v2 * f6;
                    v6 += v3 * f5;
                    v6 += (f5 * f6) * (v5 - v3);
                    dest_verts->x = v6.X;
                    dest_verts->y = v6.Y;
                    f1 = v6.X * f2 + 25.0f * m_riverVOrigin;
                    f3 = (GameMath::Sin(f1) - 1.0f) * f4;
                    dest_verts->z = v6.Z + f3;
                    dest_verts->diffuse = color2;
                    dest_verts->u1 = GameMath::Cos(11.0 * m_riverVOrigin) * 0.02f * f3 + v6.X / 150.0f;
                    dest_verts->v1 = GameMath::Cos(5.0 * m_riverVOrigin) * 0.02f * f3 + v6.Y / 150.0f;
                    dest_verts->u2 = v6.X / 50.0f;
                    dest_verts->v2 = 0.30000001 * v6.X / 50.0f + v6.Y / 50.0f;
                    dest_verts->nx = 0.0f;
                    dest_verts->ny = 0.0f;
                    dest_verts->nz = 1.0f;
                    dest_verts++;
                }
            }
        }
    } else {
        {
            DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
            VertexFormatXYZNDUV2 *dest_verts = lock.Get_Formatted_Vertex_Array();
            float f1 = cos(11.0f * m_riverVOrigin) * 0.02f;
            float f2 = cos(5.0f * m_riverVOrigin) * 0.02f;
            float f3 = 25.0f * m_riverVOrigin;
            float f4 = 1.0f / 150.0f;
            float f5 = 0.078539819f;
            float f6 = 1.0f / (float)(i2 - 1);
            float f7 = 1.0 / (float)(i1 - 1);

            for (int i = 0; i < i2; i++) {
                float f8 = (float)i * f6;

                for (int j = -0; j < i1; j++) {
                    float f9 = (float)j * f7;
                    Vector3 v6(v1);
                    v6 += v2 * f9;
                    v6 += v3 * f8;
                    v6 += (f8 * f9) * (v5 - v3);
                    dest_verts->x = v6.X;
                    dest_verts->y = v6.Y;
                    dest_verts->z = v6.Z;
                    dest_verts->diffuse = color;
                    dest_verts->u1 = GameMath::Fast_Sin(v6.X * f5 + f3) * f1 + v6.X * f4;
                    dest_verts->v1 = GameMath::Fast_Sin(v6.Y * f5 + f3) * f2 + v6.Y * f4;
                    dest_verts->u2 = v6.X / 50.0f;
                    dest_verts->v2 = (0.30000001f * v6.X + v6.Y) / 50.0f;
                    dest_verts->nx = 0.0f;
                    dest_verts->ny = 0.0f;
                    dest_verts->nz = 1.0f;
                    dest_verts++;
                }
            }
        }
    }

    Matrix3D m(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
    DX8Wrapper::Set_Index_Buffer(dyn_ib_access, 0);
    DX8Wrapper::Set_Vertex_Buffer(dyn_vb_access);
    Setup_Flat_Water_Shader();

    if (DX8Wrapper::Get_Back_Buffer_Format() == WW3D_FORMAT_A8R8G8B8 && g_theWriteableGlobalData->m_showSoftWaterEdge
        && g_theWaterTransparency->m_transparentWaterDepth != 0.0f) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);

        if (!g_theWaterTransparency->m_additiveBlending) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);
        }
    }

    DWORD cull;
    DX8Wrapper::Get_D3D_Device8()->GetRenderState(D3DRS_CULLMODE, &cull);
    DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    DX8Wrapper::Draw_Triangles(0, 2 * i3, 0, 2 * i3 + 2);

    if (m_riverWaterPixelShader) {
        DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
    }

    if (g_theWaterTransparency->m_additiveBlending) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ONE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ONE);
    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    if (g_theTerrainRenderObject->Get_Shroud()) {
        if (m_trapezoidWaterPixelShader) {
            W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
            DX8Wrapper::Get_D3D_Device8()->SetTexture(3, nullptr);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
        } else {
            W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
            W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
            DX8Wrapper::Draw_Triangles(0, 2 * i3, 0, 2 * i3 + 2);
            DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
            W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
        }
    }

    DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_CULLMODE, cull);
#endif
}

int WaterRenderObjClass::Get_Sort_Level() const
{
    return m_sortLevel;
}

void WaterRenderObjClass::Set_Sort_Level(int level)
{
    m_sortLevel = level;
}

void WaterRenderObjClass::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    int x = m_gridCellsX;
    xfer->xferInt(&x);
    captainslog_assert(x != m_gridCellsX);
    int y = m_gridCellsY;
    xfer->xferInt(&y);
    captainslog_assert(y != m_gridCellsY);

    for (int i = 0; i < m_meshDataCount; i++) {
        xfer->xferReal(&m_meshData[i].height);
        xfer->xferReal(&m_meshData[i].velocity);
        xfer->xferUnsignedByte(&m_meshData[i].status);
        xfer->xferUnsignedByte(&m_meshData[i].preferred_height);
    }
}

void WaterRenderObjClass::Setup_Jba_Water_Shader()
{
#ifdef BUILD_WITH_D3D8
    if (g_theWaterTransparency->m_additiveBlending) {
        DX8Wrapper::Set_Shader(ShaderClass::s_presetAdditiveShader);
    } else {
        DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);
    }

    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);

    m_riverTexture->Get_Texture_Filter()->Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    m_riverTexture->Get_Texture_Filter()->Set_Min_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    m_riverTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_BEST);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_ADD);

    if (!m_riverAlphaEdge->Is_Initialized()) {
        m_riverAlphaEdge->Init();
    }

    DX8Wrapper::Get_D3D_Device8()->SetTexture(3, m_riverAlphaEdge->Peek_Platform_Base_Texture());
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, 1);

    if (m_riverWaterPixelShader) {
        if (!m_waterSparklesTexture->Is_Initialized()) {
            m_waterSparklesTexture->Init();
        }

        DX8Wrapper::Get_D3D_Device8()->SetTexture(1, m_waterSparklesTexture->Peek_Platform_Base_Texture());

        if (!m_waterNoiseTexture->Is_Initialized()) {
            m_waterNoiseTexture->Init();
        }

        DX8Wrapper::Get_D3D_Device8()->SetTexture(2, m_waterNoiseTexture->Peek_Platform_Base_Texture());
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

        D3DXMATRIX m;
        D3DXMATRIX m2;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, (Matrix4 &)m2);
        float f;
        D3DXMatrixInverse(&m, &f, &m2);
        D3DXMATRIX m3;
        D3DXMatrixScaling(&m3, 0.0625f, 0.0625f, 1.0f);
        D3DXMATRIX m4 = m * m3;
        D3DXMatrixTranslation(&m3, m_riverVOrigin, m_riverVOrigin, 0.0f);
        m4 = m4 * m3;
        DX8Wrapper::Set_Transform(D3DTS_TEXTURE2, (Matrix4 &)m4);
    }

    m_pDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

    if (m_riverWaterPixelShader) {
        DX8Wrapper::Get_D3D_Device8()->SetPixelShaderConstant(0, (float *)D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f), 1);
        DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_riverWaterPixelShader);
    }
#endif
}
