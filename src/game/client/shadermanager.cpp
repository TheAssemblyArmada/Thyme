/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shader Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shadermanager.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "display.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "shader.h"
#include "vertmaterial.h"
#include "view.h"
#include "w3dshroud.h"
#include "w3dsmudge.h"
#include "water.h"
#include <captainslog.h>

#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

ScreenDefaultFilter g_screenDefaultFilter;
ScreenBWFilter g_screenBWFilter;
ScreenBWFilterDOT3 g_screenBWFilterDOT3;
ScreenCrossFadeFilter g_screenCrossFadeFilter;
ScreenMotionBlurFilter g_screenMotionBlurFilter;

W3DFilterInterface *g_screenDefaultFilterList[] = { &g_screenDefaultFilter, nullptr };
W3DFilterInterface *g_screenBWFilterList[] = { &g_screenBWFilter, &g_screenBWFilterDOT3, nullptr };
W3DFilterInterface *g_screenCrossFadeFilterList[] = { &g_screenCrossFadeFilter, nullptr };
W3DFilterInterface *g_screenMotionBlurFilterList[] = { &g_screenMotionBlurFilter, nullptr };

W3DFilterInterface **g_masterFilterList[] = {
    g_screenDefaultFilterList, g_screenBWFilterList, g_screenMotionBlurFilterList, g_screenCrossFadeFilterList, nullptr
};

ShroudTextureShader g_shroudTextureShader;
FlatShroudTextureShader g_flatShroudTextureShader;
MaskTextureShader g_maskTextureShader;
TerrainShader2Stage g_terrainShader2Stage;
FlatTerrainShader2Stage g_flatTerrainShader2Stage;
FlatTerrainShaderPixelShader g_flatTerrainShaderPixelShader;
TerrainShader8Stage g_terrainShader8Stage;
TerrainShaderPixelShader g_terrainShaderPixelShader;
CloudTextureShader g_cloudTextureShader;
RoadShaderPixelShader g_roadShaderPixelShader;
RoadShader2Stage g_roadShader2Stage;

W3DShaderInterface *g_shroudShaderList[] = { &g_shroudTextureShader, nullptr };
W3DShaderInterface *g_flatShroudShaderList[] = { &g_flatShroudTextureShader, nullptr };
W3DShaderInterface *g_maskShaderList[] = { &g_maskTextureShader, nullptr };
W3DShaderInterface *g_terrainShaderList[] = {
    &g_terrainShaderPixelShader, &g_terrainShader8Stage, &g_terrainShader2Stage, nullptr
};
W3DShaderInterface *g_flatTerrainShaderList[] = { &g_flatTerrainShaderPixelShader, &g_flatTerrainShader2Stage, nullptr };
W3DShaderInterface *g_cloudShaderList[] = { &g_cloudTextureShader, nullptr };
W3DShaderInterface *g_roadShaderList[] = { &g_roadShaderPixelShader, &g_roadShader2Stage, nullptr };

W3DShaderInterface **g_masterShaderList[] = { g_terrainShaderList,
    g_shroudShaderList,
    g_flatShroudShaderList,
    g_roadShaderList,
    g_maskShaderList,
    g_cloudShaderList,
    g_flatTerrainShaderList,
    nullptr };

int W3DShaderManager::s_currentShaderPass;
FilterTypes W3DShaderManager::s_currentFilter;
w3dsurface_t W3DShaderManager::s_oldRenderSurface;
w3dtexture_t W3DShaderManager::s_renderTexture;
w3dsurface_t W3DShaderManager::s_newRenderSurface;
w3dsurface_t W3DShaderManager::s_oldDepthSurface;

W3DFilterInterface *g_w3dFilters[FT_MAX];
W3DShaderInterface *g_w3dShaders[W3DShaderManager::ST_MAX];
int g_w3dShadersPassCount[W3DShaderManager::ST_MAX];

#ifndef GAME_DLL
TextureClass *W3DShaderManager::s_textures[MAX_TEXTURE_STAGES];
bool W3DShaderManager::s_renderingToTexture;
W3DShaderManager::ShaderTypes W3DShaderManager::s_currentShader;
#endif

int W3DShaderInterface::Set(int pass)
{
    return 1;
}

void W3DShaderInterface::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, nullptr);
#endif
}

int W3DShaderInterface::Shutdown()
{
    return 1;
}

void W3DShaderManager::Init()
{
#ifdef BUILD_WITH_D3D8
    if (Get_Chipset() != GPU_UNKNOWN) {
        DX8Wrapper::Get_D3D_Device8()->GetRenderTarget(&s_oldRenderSurface);
        D3DSURFACE_DESC desc;
        s_oldRenderSurface->GetDesc(&desc);

        if (FAILED(DX8Wrapper::Get_D3D_Device8()->CreateTexture(
                desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &s_renderTexture))) {
            if (s_oldRenderSurface) {
                s_oldRenderSurface->Release();
            }
            s_oldRenderSurface = nullptr;
            s_renderTexture = nullptr;
        } else if (FAILED(s_renderTexture->GetSurfaceLevel(0, &s_newRenderSurface))) {
            if (s_renderTexture) {
                s_renderTexture->Release();
            }
            s_renderTexture = nullptr;
            s_newRenderSurface = nullptr;
        } else {
            if (FAILED(DX8Wrapper::Get_D3D_Device8()->GetDepthStencilSurface(&s_oldDepthSurface))) {
                if (s_newRenderSurface) {
                    s_newRenderSurface->Release();
                }
                if (s_renderTexture) {
                    s_renderTexture->Release();
                }
                s_renderTexture = nullptr;
                s_newRenderSurface = nullptr;
                s_oldDepthSurface = nullptr;
            }
        }

        for (int i = 0; g_masterShaderList[i]; i++) {
            W3DShaderInterface **list = g_masterShaderList[i];

            for (int j = 0; list[j] && !list[j]->Init(); j++) {
            }
        }

        for (int i = 0; g_masterFilterList[i]; i++) {
            W3DFilterInterface **list = g_masterFilterList[i];

            for (int j = 0; list[j] && !list[j]->Init(); j++) {
            }
        }
    }
#endif
}

void W3DShaderManager::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (s_newRenderSurface) {
        s_newRenderSurface->Release();
    }

    if (s_renderTexture) {
        s_renderTexture->Release();
    }

    if (s_oldRenderSurface) {
        s_oldRenderSurface->Release();
    }

    if (s_oldDepthSurface) {
        s_oldDepthSurface->Release();
    }

    s_renderTexture = nullptr;
    s_newRenderSurface = nullptr;
    s_oldDepthSurface = nullptr;
    s_oldRenderSurface = nullptr;
    s_currentShader = ST_INVALID;
    s_currentFilter = FT_NULL_FILTER;

    for (int i = 0; i < ST_MAX; i++) {
        if (g_w3dShaders[i]) {
            g_w3dShaders[i]->Shutdown();
        }
    }

    for (int i = 0; i < FT_MAX; i++) {
        if (g_w3dFilters[i]) {
            g_w3dFilters[i]->Shutdown();
        }
    }
#endif
}

int W3DShaderManager::Get_Shader_Passes(ShaderTypes shader)
{
    return g_w3dShadersPassCount[shader];
}

int W3DShaderManager::Set_Shader(ShaderTypes shader, int pass)
{
    if (shader == s_currentShader && pass == s_currentShaderPass) {
        return 1;
    }

    s_currentShader = shader;
    s_currentShaderPass = pass;

    if (g_w3dShaders[shader]) {
        return g_w3dShaders[shader]->Set(pass);
    } else {
        return 0;
    }
}

void W3DShaderManager::Reset_Shader(ShaderTypes shader)
{
    if (s_currentShader) {
        if (g_w3dShaders[shader]) {
            g_w3dShaders[shader]->Reset();
        }

        s_currentShader = ST_INVALID;
    }
}

bool W3DShaderManager::Filter_Pre_Render(FilterTypes filter, bool &skip, CustomScenePassModes &mode)
{
    if (!g_w3dFilters[filter]) {
        return false;
    }

    bool b = g_w3dFilters[filter]->Pre_Render(skip, mode);

    if (b) {
        s_currentFilter = filter;
    }

    return b;
}

bool W3DShaderManager::Filter_Post_Render(FilterTypes filter, FilterModes mode, Coord2D &delta, bool &b)
{
    if (g_w3dFilters[filter]) {
        return g_w3dFilters[filter]->Post_Render(mode, delta, b);
    }

    s_currentFilter = FT_NULL_FILTER;
    return false;
}

bool W3DShaderManager::Filter_Setup(FilterTypes filter, FilterModes mode)
{
    if (g_w3dFilters[filter]) {
        return g_w3dFilters[filter]->Setup(mode);
    } else {
        return false;
    }
}

void W3DShaderManager::Draw_Viewport(unsigned int color)
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_TEX_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
        float u;
        float v;
    };

    _TRANS_LIT_TEX_VERTEX vertex[4];

    int32_t x;
    int32_t y;
    g_theTacticalView->Get_Origin(&x, &y);
    int32_t w = g_theTacticalView->Get_Width();
    int32_t h = g_theTacticalView->Get_Height();
    int32_t w2 = g_theDisplay->Get_Width();
    int32_t h2 = g_theDisplay->Get_Height();

    vertex[0].p = D3DXVECTOR4((float)(w + x) - 0.5f, (float)(h + y) - 0.5f, 0.0f, 1.0f);
    vertex[0].u = (float)(w + x) / (float)w2;
    vertex[0].v = (float)(h + y) / (float)h2;
    vertex[1].p = D3DXVECTOR4((float)(w + x) - 0.5f, (float)y - 0.5f, 0.0f, 1.0f);
    vertex[1].u = (float)(w + x) / (float)w2;
    vertex[1].v = (float)y / (float)h2;
    vertex[2].p = D3DXVECTOR4((float)x - 0.5f, (float)(h + y) - 0.5f, 0.0f, 1.0f);
    vertex[2].u = (float)x / (float)w2;
    vertex[2].v = (float)(h + y) / (float)h2;
    vertex[3].p = D3DXVECTOR4((float)x - 0.5f, (float)y - 0.5f, 0.0f, 1.0f);
    vertex[3].u = (float)x / (float)w2;
    vertex[3].v = (float)y / (float)h2;
    vertex[0].color = color;
    vertex[1].color = color;
    vertex[2].color = color;
    vertex[3].color = color;

    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
#endif
}

void W3DShaderManager::Start_Render_To_Texture()
{
#ifdef BUILD_WITH_D3D8
    captainslog_dbgassert(!s_renderingToTexture, "Already rendering to texture - cannot nest calls.");

    if (!s_renderingToTexture) {
        if (s_newRenderSurface) {
            if (s_oldDepthSurface) {
                HRESULT r = DX8Wrapper::Get_D3D_Device8()->SetRenderTarget(s_newRenderSurface, s_oldDepthSurface);
                captainslog_dbgassert(SUCCEEDED(r), "Set target failed unexpectedly.");

                if (SUCCEEDED(r)) {
                    s_renderingToTexture = true;

                    if (g_theWriteableGlobalData->m_showSoftWaterEdge) {
                        if (s_currentFilter != FT_VIEW_MOTION_BLUR_FILTER && s_currentFilter != FT_VIEW_CROSS_FADE_FILTER) {
                            DX8Wrapper::Clear(true,
                                false,
                                Vector3(0.0f, 0.0f, 0.0f),
                                g_theWaterTransparency->m_transparentWaterMinOpacity);
                        } else {
                            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 8);
                            ShaderClass s = ShaderClass::s_presetOpaqueShader;
                            s.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
                            s.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
                            DX8Wrapper::Set_Shader(s);
                            VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
                            DX8Wrapper::Set_Material(m);

                            if (m) {
                                m->Release_Ref();
                            }

                            W3DShaderManager::Draw_Viewport(
                                ((int)(g_theWaterTransparency->m_transparentWaterMinOpacity * 255.0f)) << 24 | 0xFFFFFF);
                            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
                        }
                    }
                }
            }
        }
    }
#endif
}

w3dtexture_t W3DShaderManager::End_Render_To_Texture()
{
#ifdef BUILD_WITH_D3D8
    captainslog_dbgassert(s_renderingToTexture, "Not rendering to texture.");

    if (!s_renderingToTexture) {
        return nullptr;
    }

    HRESULT r = DX8Wrapper::Get_D3D_Device8()->SetRenderTarget(s_oldRenderSurface, s_oldDepthSurface);
    captainslog_dbgassert(SUCCEEDED(r), "Set target failed unexpectedly.");

    if (SUCCEEDED(r)) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
        s_renderingToTexture = false;
    }

    return s_renderTexture;
#else
    return 0;
#endif
}

w3dtexture_t W3DShaderManager::Get_Render_Texture()
{
    return s_renderTexture;
}

GPUType W3DShaderManager::Get_Chipset()
{
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_chipsetType) {
        return (GPUType)g_theWriteableGlobalData->m_chipsetType;
    }

    return GPU_PS20;
}

long W3DShaderManager::Load_And_Create_D3D_Shader(
    const char *path, const unsigned long *decleration, unsigned long usage, bool type, unsigned long *handle)
{
#ifdef BUILD_WITH_D3D8
    if (W3DShaderManager::Get_Chipset() < GPU_PS11) {
        return E_FAIL;
    }
    File *file = g_theFileSystem->Open_File(path, File::BINARY | File::READ);

    if (file == nullptr) {
        captainslog_debug("Could not find file \"%s\"", path);
        return E_FAIL;
    }

    FileInfo info;
    g_theFileSystem->Get_File_Info(path, &info);
    DWORD *buf = (DWORD *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, info.file_size_low);

    if (!buf) {
        captainslog_debug("Failed to allocate memory to load shader \"%s\"", path);
        file->Close();
        return E_FAIL;
    }

    file->Read(buf, info.file_size_low);
    file->Close();
    file = nullptr;

    HRESULT res;

    if (type) {
        res = DX8Wrapper::Get_D3D_Device8()->CreateVertexShader(decleration, buf, handle, usage);
    } else if (!type) {
        res = DX8Wrapper::Get_D3D_Device8()->CreatePixelShader(buf, handle);
    }

    HeapFree(GetProcessHeap(), 0, buf);

    if (FAILED(res)) {
        captainslog_debug("Failed to create shader \"%s\"", path);
        return E_FAIL;
    }

    return S_OK;
#else
    return 0;
#endif
}

bool W3DShaderManager::Set_Shroud_Tex(int stage)
{
#ifdef BUILD_WITH_D3D8
    W3DShroud *shroud = g_theTerrainRenderObject->Get_Shroud();

    if (!shroud) {
        return false;
    }

    DX8Wrapper::Set_Texture(stage, shroud->Get_Shroud_Texture());
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    D3DXMATRIX m2;
    D3DXMATRIX m;
    DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
    float f;
    D3DXMatrixInverse(&m2, &f, &m);
    D3DXMATRIX m3;
    D3DXMATRIX m4;
    float x = 0.0f;
    float y = 0.0f;
    float sx = shroud->Get_Cell_Width();
    float sy = shroud->Get_Cell_Height();

    if (g_theTerrainRenderObject->Get_Map()) {
        x = sx - shroud->Get_Draw_Origin_X();
        y = sy - shroud->Get_Draw_Origin_Y();
    }

    D3DXMatrixTranslation(&m4, x, y, 0.0f);
    sx = 1.0f / ((float)shroud->Get_Texture_Width() * sx);
    sy = 1.0f / ((float)shroud->Get_Texture_Height() * sy);
    D3DXMatrixScaling(&m3, sx, sy, 1.0f);
    m = m2 * m4 * m3;
    DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(stage + D3DTS_TEXTURE0), m);
#endif
    return true;
}
