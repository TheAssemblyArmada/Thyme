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
#include "w3dsmudge.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int ScreenDefaultFilter::Init()
{
    if (!W3DShaderManager::Render_Surfaces_Valid()) {
        return 0;
    }

    g_w3dFilters[FT_VIEW_SCREEN_DEFAULT_FILTER] = &g_screenDefaultFilter;
    return 1;
}

bool ScreenDefaultFilter::Pre_Render(bool &skip, CustomScenePassModes &mode)
{
    if (g_theSmudgeManager && !g_theSmudgeManager->Get_Heat_Haze_Count()) {
        return false;
    }

    W3DShaderManager::Start_Render_To_Texture();
    return true;
}

bool ScreenDefaultFilter::Post_Render(FilterModes mode, Coord2D &delta, bool &b)
{
#ifdef BUILD_WITH_D3D8
    w3dtexture_t tex = W3DShaderManager::End_Render_To_Texture();
    captainslog_dbgassert(tex, "Require rendered texture.");

    if (!tex) {
        return false;
    }

    if (!Set(mode)) {
        return false;
    }

    VertexFormatXYZWDUV1 vertex[4];
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, tex);
    int32_t x;
    int32_t y;
    g_theTacticalView->Get_Origin(&x, &y);
    int32_t w = g_theTacticalView->Get_Width();
    int32_t h = g_theTacticalView->Get_Height();
    int32_t w2 = g_theDisplay->Get_Width();
    int32_t h2 = g_theDisplay->Get_Height();

    vertex[0].x = (float)(w + x) - 0.5f;
    vertex[0].y = (float)(h + y) - 0.5f;
    vertex[0].z = 0.0f;
    vertex[0].w = 1.0f;
    vertex[0].u = (float)(w + x) / (float)w2;
    vertex[0].v = (float)(h + y) / (float)h2;

    vertex[1].x = (float)(w + x) - 0.5f;
    vertex[1].y = (float)y - 0.5f;
    vertex[1].z = 0.0f;
    vertex[1].w = 1.0f;
    vertex[1].u = (float)(w + x) / (float)w2;
    vertex[1].v = (float)y / (float)h2;

    vertex[2].x = (float)x - 0.5f;
    vertex[2].y = (float)(h + y) - 0.5f;
    vertex[2].z = 0.0f;
    vertex[2].w = 1.0f;
    vertex[2].u = (float)x / (float)w2;
    vertex[2].v = (float)(h + y) / (float)h2;

    vertex[3].x = (float)x - 0.5f;
    vertex[3].y = (float)y - 0.5f;
    vertex[3].z = 0.0f;
    vertex[3].w = 1.0f;
    vertex[3].u = (float)x / (float)w2;
    vertex[3].v = (float)y / (float)h2;

    vertex[0].color = 0xFFFFFFFF;
    vertex[1].color = 0xFFFFFFFF;
    vertex[2].color = 0xFFFFFFFF;
    vertex[3].color = 0xFFFFFFFF;

    DX8Wrapper::Get_D3D_Device8()->SetVertexShader(VertexFormatXYZWDUV1::DX8FVF);
    DX8Wrapper::Get_D3D_Device8()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(VertexFormatXYZWDUV1));
    Reset();
    return true;
#else
    return false;
#endif
}

int ScreenDefaultFilter::Set(FilterModes mode)
{
#ifdef BUILD_WITH_D3D8
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

void ScreenDefaultFilter::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}
