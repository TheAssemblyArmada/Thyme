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
#include "assetmgr.h"
#include "display.h"
#include "shadermanager.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int ScreenCrossFadeFilter::s_fadeFrames;
int ScreenCrossFadeFilter::s_curFadeFrame;
int ScreenCrossFadeFilter::s_fadeDirection;
float ScreenCrossFadeFilter::s_curFadeValue;
TextureClass *ScreenCrossFadeFilter::s_fadePatternTexture;
bool ScreenCrossFadeFilter::s_skipRender;

int ScreenCrossFadeFilter::Init()
{
    if (!g_theDisplay) {
        return 0;
    }

    s_curFadeFrame = 0;

    if (!W3DShaderManager::Render_Surfaces_Valid()) {
        return 0;
    }

    s_fadePatternTexture = W3DAssetManager::Get_Instance()->Get_Texture("exmask_g.tga");

    if (!s_fadePatternTexture) {
        return 0;
    }

    s_fadePatternTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    s_fadePatternTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    s_fadePatternTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
    g_w3dFilters[FT_VIEW_CROSS_FADE_FILTER] = &g_screenCrossFadeFilter;
    return 1;
}

int ScreenCrossFadeFilter::Shutdown()
{
    Ref_Ptr_Release(s_fadePatternTexture);
    return 1;
}

bool ScreenCrossFadeFilter::Pre_Render(bool &skip, CustomScenePassModes &mode)
{
    if (Update_Fade_Level()) {
        W3DShaderManager::Start_Render_To_Texture();
        mode = MODE_MASK;
        skip = false;
        s_skipRender = true;
    }
    return true;
}

bool ScreenCrossFadeFilter::Post_Render(FilterModes mode, Coord2D &delta, bool &b)
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_TEX_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
        float u1;
        float v1;
        float u2;
        float v2;
    };

    if (s_skipRender) {
        s_skipRender = false;
        b = true;
        W3DShaderManager::End_Render_To_Texture();
        return true;
    }

    w3dtexture_t tex = W3DShaderManager::Get_Render_Texture();
    captainslog_dbgassert(tex, "Require last rendered texture.");

    if (!tex) {
        return false;
    }

    if (!Set(mode)) {
        return false;
    }

    _TRANS_LIT_TEX_VERTEX vertex[4];
    float f1 = 0.0f;
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, tex);

    if (mode == FM_4) {
        DX8Wrapper::Get_D3D_Device8()->SetTexture(1, s_fadePatternTexture->Peek_Platform_Base_Texture());
        f1 = 1.0f - s_curFadeValue + 1.0f - s_curFadeValue;

        if (f1 <= 0.0f) {
            f1 = 0.01f;
        }

        f1 = 0.5f / f1;
    }

    int32_t x;
    int32_t y;
    g_theTacticalView->Get_Origin(&x, &y);
    int32_t w = g_theTacticalView->Get_Width();
    int32_t h = g_theTacticalView->Get_Height();
    int32_t w2 = g_theDisplay->Get_Width();
    int32_t h2 = g_theDisplay->Get_Height();
    vertex[0].p = D3DXVECTOR4((float)(w + x) - 0.5f, (float)(h + y) - 0.5f, 0.0f, 1.0f);
    vertex[0].u1 = (float)(w + x) / (float)w2;
    vertex[0].v1 = (float)(h + y) / (float)h2;
    vertex[0].u2 = f1 + 0.5f;
    vertex[0].v2 = f1 + 0.5f;
    vertex[1].p = D3DXVECTOR4((float)(w + x) - 0.5f, (float)y - 0.5f, 0.0f, 1.0f);
    vertex[1].u1 = (float)(w + x) / (float)w2;
    vertex[1].v1 = (float)y / (float)h2;
    vertex[1].u2 = f1 + 0.5f;
    vertex[1].v2 = 0.5f - f1;
    vertex[2].p = D3DXVECTOR4((float)x - 0.5f, (float)(h + y) - 0.5f, 0.0f, 1.0f);
    vertex[2].u1 = (float)x / (float)w2;
    vertex[2].v1 = (float)(h + y) / (float)h2;
    vertex[2].u2 = 0.5f - f1;
    vertex[2].v2 = f1 + 0.5f;
    vertex[3].p = D3DXVECTOR4((float)x - 0.5f, (float)y - 0.5f, 0.0f, 1.0f);
    vertex[3].u1 = (float)x / (float)w2;
    vertex[3].v1 = (float)y / (float)h2;
    vertex[3].u2 = 0.5f - f1;
    vertex[3].v2 = 0.5f - f1;
    vertex[0].color = 0xFFFFFFFF;
    vertex[1].color = 0xFFFFFFFF;
    vertex[2].color = 0xFFFFFFFF;
    vertex[3].color = 0xFFFFFFFF;
    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(D3DFVF_TEX2 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    Reset();
    return true;
#else
    return false;
#endif
}

int ScreenCrossFadeFilter::Set(FilterModes mode)
{
#ifdef BUILD_WITH_D3D8
    if (mode <= 0) {
        return 0;
    }

    VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(m);

    if (m) {
        m->Release_Ref();
    }

    DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

    if (mode == FM_4) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
#endif
    return 1;
}

void ScreenCrossFadeFilter::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}

bool ScreenCrossFadeFilter::Update_Fade_Level()
{
    if (s_fadeDirection <= 0) {
        if (s_fadeDirection < 0) {
            if (s_curFadeFrame >= s_fadeFrames) {
                s_curFadeValue = 0.0f;
                g_theTacticalView->Set_View_Filter_Mode(FM_NULL_MODE);
                g_theTacticalView->Set_View_Filter(FT_NULL_FILTER);
                s_curFadeFrame = 0;
                s_fadeDirection = 0;
                return false;
            }

            s_curFadeValue = 1.0f - (float)s_curFadeFrame++ / (float)s_fadeFrames;
        }
    } else {
        if (++s_curFadeFrame >= s_fadeFrames) {
            s_curFadeFrame = 0;
            s_curFadeValue = 1.0f;
            s_fadeDirection = 0;
            return false;
        }

        s_curFadeValue = (float)s_curFadeFrame / (float)s_fadeFrames;
    }
    return true;
}
