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
#include "display.h"
#include "shadermanager.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int ScreenBWFilter::s_fadeFrames;
int ScreenBWFilter::s_fadeDirection;
int ScreenBWFilter::s_curFadeFrame;
float ScreenBWFilter::s_curFadeValue;

int ScreenBWFilter::Init()
{
#ifdef BUILD_WITH_D3D8
    m_dwBWPixelShader = 0;
    s_curFadeFrame = 0;

    if (!W3DShaderManager::Render_Surfaces_Valid()) {
        return 0;
    }

    if (W3DShaderManager::Get_Chipset() < GPU_PS11) {
        return 0;
    }

    DWORD decl[] = { D3DVSD_STREAM(0),
        D3DVSD_REG(0, D3DVSDT_FLOAT3),
        D3DVSD_REG(1, D3DVSDT_D3DCOLOR),
        D3DVSD_REG(2, D3DVSDT_FLOAT2),
        D3DVSD_END() };

    int i = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\monochrome.pso", decl, 0, false, &m_dwBWPixelShader);

    if (i < 0) {
        return 0;
    }

    g_w3dFilters[FT_VIEW_BW_FILTER] = &g_screenBWFilter;
#endif
    return 1;
}

int ScreenBWFilter::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (m_dwBWPixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBWPixelShader);
    }

    m_dwBWPixelShader = 0;
#endif
    return 1;
}

bool ScreenBWFilter::Pre_Render(bool &skip, CustomScenePassModes &mode)
{
    skip = false;
    W3DShaderManager::Start_Render_To_Texture();
    return true;
}

bool ScreenBWFilter::Post_Render(FilterModes mode, Coord2D &delta, bool &b)
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_TEX_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
        float u;
        float v;
    };

    w3dtexture_t tex = W3DShaderManager::End_Render_To_Texture();
    captainslog_dbgassert(tex, "Require rendered texture.");

    if (!tex) {
        return false;
    }

    if (!Set(mode)) {
        return false;
    }

    _TRANS_LIT_TEX_VERTEX vertex[4];
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, tex);
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
    vertex[0].color = 0xFFFFFFFF;
    vertex[1].color = 0xFFFFFFFF;
    vertex[2].color = 0xFFFFFFFF;
    vertex[3].color = 0xFFFFFFFF;
    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    Reset();
    return true;
#else
    return false;
#endif
}

int ScreenBWFilter::Set(FilterModes mode)
{
#ifdef BUILD_WITH_D3D8
    if (mode <= 0) {
        return 0;
    }

    if (s_fadeDirection <= 0) {
        if (s_fadeDirection < 0) {
            int i = ++s_curFadeFrame;

            if (s_curFadeFrame >= s_fadeFrames) {
                s_curFadeValue = 0.0f;
                g_theTacticalView->Set_View_Filter_Mode(FM_NULL_MODE);
                g_theTacticalView->Set_View_Filter(FT_NULL_FILTER);
                s_curFadeFrame = 0;
                s_fadeDirection = 0;
            } else {
                s_curFadeValue = 1.0f - (float)i / (float)s_fadeFrames;
            }
        }
    } else {
        int i = ++s_curFadeFrame;

        if (s_curFadeFrame >= s_fadeFrames) {
            s_curFadeFrame = 0;
            s_curFadeValue = 1.0f;
            s_fadeDirection = 0;
        } else {
            s_curFadeValue = (float)i / (float)s_fadeFrames;
        }
    }

    VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(m);

    if (m) {
        m->Release_Ref();
    }

    DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBWPixelShader);
    D3DXVECTOR4 v(0.30000001f, 0.58999997f, 0.11f, 1.0f);
    DX8Wrapper::Get_D3D_Device8()->SetPixelShaderConstant(0, static_cast<CONST FLOAT *>(v), 1);
    D3DXVECTOR4 v2(1.0f, 1.0f, 1.0f, 1.0f);

    if (mode == FM_VIEW_BW_BLACK_AND_WHITE) {
        v2.x = 1.0f;
        v2.y = 1.0f;
        v2.z = 1.0f;
    }

    if (mode == FM_VIEW_BW_RED_AND_WHITE) {
        v2.x = 1.0f;
        v2.y = 0.0f;
        v2.z = 0.0f;
    }

    if (mode == FM_VIEW_BW_GREEN_AND_WHITE) {
        v2.x = 0.0f;
        v2.y = 1.0f;
        v2.z = 0.0f;
    }

    DX8Wrapper::Get_D3D_Device8()->SetPixelShaderConstant(1, static_cast<CONST FLOAT *>(v2), 1);
    D3DXVECTOR4 v3(s_curFadeValue, s_curFadeValue, s_curFadeValue, 1.0f);
    DX8Wrapper::Get_D3D_Device8()->SetPixelShaderConstant(2, static_cast<CONST FLOAT *>(v3), 1);
#endif
    return 1;
}

void ScreenBWFilter::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}

int ScreenBWFilterDOT3::Init()
{
    ScreenBWFilter::s_curFadeFrame = 0;

    if (!W3DShaderManager::Render_Surfaces_Valid()) {
        return 0;
    }

    if (W3DShaderManager::Get_Chipset() == GPU_UNKNOWN) {
        return 0;
    }

    g_w3dFilters[FT_VIEW_BW_FILTER] = &g_screenBWFilterDOT3;
    return 1;
}

int ScreenBWFilterDOT3::Shutdown()
{
    return 1;
}

bool ScreenBWFilterDOT3::Pre_Render(bool &skip, CustomScenePassModes &mode)
{
    skip = false;
    W3DShaderManager::Start_Render_To_Texture();
    return true;
}

bool ScreenBWFilterDOT3::Post_Render(FilterModes mode, Coord2D &delta, bool &b)
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_TEX_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
        float u;
        float v;
    };

    w3dtexture_t tex = W3DShaderManager::End_Render_To_Texture();
    captainslog_dbgassert(tex, "Require rendered texture.");

    if (!tex) {
        return false;
    }

    if (!Set(mode)) {
        return false;
    }

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
    unsigned int color = (unsigned int)((1.0f - ScreenBWFilter::s_curFadeValue) * 255.0f);
    vertex[0].color = (color << 24) | 0xFFFFFF;
    vertex[1].color = (color << 24) | 0xFFFFFF;
    vertex[2].color = (color << 24) | 0xFFFFFF;
    vertex[3].color = (color << 24) | 0xFFFFFF;
    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);

    if (DX8Wrapper::Get_Current_Caps()->Supports_Dot3_Blend()) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(128, 165, 202, 142));
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG0, D3DTA_ALPHAREPLICATE | D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_ALPHAREPLICATE | D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(96, 96, 96, 96));
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }

    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, tex);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    ShaderClass::Invalidate();
    ShaderClass s = ShaderClass::s_presetAlphaShader;
    s.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    DX8Wrapper::Set_Shader(s);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    Reset();
    return true;
#else
    return false;
#endif
}

int ScreenBWFilterDOT3::Set(FilterModes mode)
{
#ifdef BUILD_WITH_D3D8
    if (mode <= 0) {
        return 0;
    }

    if (ScreenBWFilter::s_fadeDirection <= 0) {
        if (ScreenBWFilter::s_fadeDirection < 0) {
            if (++ScreenBWFilter::s_curFadeFrame >= ScreenBWFilter::s_fadeFrames) {
                ScreenBWFilter::s_curFadeValue = 0.0f;
                g_theTacticalView->Set_View_Filter_Mode(FM_NULL_MODE);
                g_theTacticalView->Set_View_Filter(FT_NULL_FILTER);
                ScreenBWFilter::s_curFadeFrame = 0;
                ScreenBWFilter::s_fadeDirection = 0;
            } else {
                ScreenBWFilter::s_curFadeValue =
                    1.0f - (float)ScreenBWFilter::s_curFadeFrame / (float)ScreenBWFilter::s_fadeFrames;
            }
        }
    } else if (++ScreenBWFilter::s_curFadeFrame >= ScreenBWFilter::s_fadeFrames) {
        ScreenBWFilter::s_curFadeFrame = 0;
        ScreenBWFilter::s_curFadeValue = 1.0f;
        ScreenBWFilter::s_fadeDirection = 0;
    } else {
        ScreenBWFilter::s_curFadeValue = (float)ScreenBWFilter::s_curFadeFrame / (float)ScreenBWFilter::s_fadeFrames;
    }

    VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(m);

    if (m) {
        m->Release_Ref();
    }

    DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
    DX8Wrapper::Apply_Render_State_Changes();
#endif
    return 1;
}

void ScreenBWFilterDOT3::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}
