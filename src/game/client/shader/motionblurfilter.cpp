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
#include "gamelogic.h"
#include "shadermanager.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

Coord3D ScreenMotionBlurFilter::s_zoomToPos;
bool ScreenMotionBlurFilter::s_zoomToValid;

ScreenMotionBlurFilter::ScreenMotionBlurFilter() : m_maxCount(0), m_lastFrame(0), m_decrement(0), m_skipRender(false)
{
    // #BUGFIX Initialize all members
    m_additive = false;
    m_doZoomTo = false;
    m_priorDelta = Coord2D{};
    m_panFactor = 0;
}

int ScreenMotionBlurFilter::Init()
{
    if (!W3DShaderManager::Render_Surfaces_Valid()) {
        return 0;
    }

    g_w3dFilters[FT_VIEW_MOTION_BLUR_FILTER] = this;
    return 1;
}

int ScreenMotionBlurFilter::Shutdown()
{
    return 1;
}

bool ScreenMotionBlurFilter::Pre_Render(bool &skip, CustomScenePassModes &mode)
{
    skip = m_skipRender;
    W3DShaderManager::Start_Render_To_Texture();
    return 1;
}

bool ScreenMotionBlurFilter::Post_Render(FilterModes mode, Coord2D &delta, bool &b)
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

    if (m_additive) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ONE);
    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    float f1 = 0.5f;
    float f2 = 0.5f;
    bool b1 = false;
    bool b2 = true;

    if (mode < FM_VIEW_MB_PAN_ALPHA) {
        if (mode == FM_VIEW_MB_END_PAN_ALPHA) {
            float f3 = GameMath::Sqrt(m_priorDelta.x * m_priorDelta.x + m_priorDelta.y * m_priorDelta.y);
            f1 = m_priorDelta.x / f3 * 0.5f + f1;
            f2 = f2 - m_priorDelta.y / f3 * 0.5f;
            m_decrement = false;

            if (--m_maxCount < 2) {
                b2 = false;
            }

            b1 = true;
        }
    } else {
        float f3 = GameMath::Sqrt(delta.x * delta.x + delta.y * delta.y);
        f2 = f2 - 0.5f;
        m_decrement = false;
        m_maxCount = (f3 * 200.0f * m_panFactor / 30.0f);

        if (m_maxCount < m_panFactor / 2) {
            m_maxCount = m_panFactor / 2;
        }

        if (m_maxCount > m_panFactor) {
            m_maxCount = m_panFactor;
        }

        b1 = true;
        m_priorDelta.x = delta.x;
        m_priorDelta.y = delta.y;
    }

    m_skipRender = false;

    if (!b1 && m_lastFrame != g_theGameLogic->Get_Frame()) {
        if (m_decrement) {
            m_maxCount -= 5;

            if (m_maxCount >= 1) {
                m_skipRender = true;
            } else {
                m_decrement = false;
                b2 = false;
            }
        } else {
            m_maxCount += 5;

            if (m_maxCount < 60) {
                m_skipRender = true;
            } else {
                m_decrement = true;

                if (m_doZoomTo && ScreenMotionBlurFilter::s_zoomToValid) {
                    g_theTacticalView->Look_At(&ScreenMotionBlurFilter::s_zoomToPos);
                } else {
                    b2 = false;
                }
            }
        }
    }

    if (!b1) {
        for (int i = 0; i < 4; i++) {
            float f3 = 1.0f - m_maxCount / 60.0f * 0.89999998f;
            f3 = GameMath::Sqrt(f3);
            vertex[i].u = (vertex[i].u - f1) * f3 + f1;
            vertex[i].v = (vertex[i].v - f2) * f3 + f2;
        }
    }

    DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
    DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
    DX8Wrapper::Apply_Render_State_Changes();
    int count = m_maxCount;

    if (m_maxCount > 30) {
        count = 30;
    }

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            float f3 = 0.99000001f;

            if (m_additive) {
                f3 = 0.98000002f;
            }

            int color = 21;

            if (m_additive) {
                color = 9;

                if (m_maxCount > count) {
                    color += (m_maxCount - count) / 5;
                }
                if (m_maxCount == 60) {
                    color += 60;
                }
            }

            vertex[j].color = (color << 24) | 0xFFFFFF;

            if (b1) {
                vertex[j].u = (vertex[j].u - f1) * (f3 + 0.006f) + f1;
            } else {
                vertex[j].u = (vertex[j].u - f1) * f3 + f1;
            }

            vertex[j].v = (vertex[j].v - f2) * f3 + f2;
        }

        DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_TEX_VERTEX));
    }

    m_lastFrame = g_theGameLogic->Get_Frame();

    if (b1) {
        m_skipRender = false;
    }

    Reset();

    if (!b2) {
        s_zoomToValid = false;
    }

    return b2;
#else
    return false;
#endif
}

bool ScreenMotionBlurFilter::Setup(FilterModes mode)
{
    m_additive = false;

    if (mode == FM_VIEW_MB_IN_AND_OUT_STATURATE || mode == FM_VIEW_MB_IN_STATURATE || mode == FM_VIEW_MB_OUT_STATURATE) {
        m_additive = true;
    }

    m_doZoomTo = false;
    if (mode == FM_VIEW_MB_IN_AND_OUT_STATURATE || mode == FM_VIEW_MB_IN_AND_OUT_ALPHA) {
        m_doZoomTo = true;
    }

    if (mode >= FM_VIEW_MB_PAN_ALPHA) {
        m_panFactor = mode - FM_VIEW_MB_PAN_ALPHA;

        if (m_panFactor < 1) {
            m_panFactor = 30;
        }
    }

    m_skipRender = false;

    if (mode != FM_VIEW_MB_END_PAN_ALPHA) {
        m_maxCount = 0;
    }

    m_decrement = false;
    m_skipRender = false;

    if (mode == FM_VIEW_MB_OUT_ALPHA || mode == FM_VIEW_MB_OUT_STATURATE) {
        m_maxCount = 60;
        m_decrement = true;
    }

    return 1;
}

int ScreenMotionBlurFilter::Set(FilterModes mode)
{
#ifdef BUILD_WITH_D3D8
    if (mode > 0) {
        VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(m);

        if (m) {
            m->Release_Ref();
        }

        DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Set_Texture(1, nullptr);
        DX8Wrapper::Apply_Render_State_Changes();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
        DX8Wrapper::Apply_Render_State_Changes();
    }
#endif
    return 1;
}

void ScreenMotionBlurFilter::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}
