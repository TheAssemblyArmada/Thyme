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
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int MaskTextureShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    float f1 = ScreenCrossFadeFilter::Get_Current_Fade_Value();
    float f2 = 1.0f - f1 + 1.0f - f1;

    if (f2 <= 0.0f) {
        f2 = 0.01f;
    }

    f2 = 0.5f / f2;
    VertexMaterialClass *m = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(m);

    if (m) {
        m->Release_Ref();
    }

    DX8Wrapper::Set_Texture(0, ScreenCrossFadeFilter::Get_Fade_Pattern_Texture());
    ShaderClass s = ShaderClass::s_presetOpaqueShader;
    s.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
    DX8Wrapper::Set_Shader(s);
    DX8Wrapper::Apply_Render_State_Changes();
    D3DXMATRIX m1;
    DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m1);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    D3DXMATRIX m2;
    float f;
    D3DXMatrixInverse(&m2, &f, &m1);
    D3DXMATRIX m3;
    D3DXMATRIX m4;
    D3DXMATRIX m5;
    Coord3D c{}; // #BUGFIX Default initialize

    if (g_theTacticalView) {
        ICoord2D c2;
        c2.x = (int)((float)g_theTacticalView->Get_Width() * 0.5f);
        c2.y = (int)((float)g_theTacticalView->Get_Height() * 0.5f);
        g_theTacticalView->Screen_To_Terrain(&c2, &c);
    }

    D3DXMatrixTranslation(&m4, -c.x, -c.y, 0.0f);
    D3DXMatrixTranslation(&m5, 0.5f, 0.5f, 0.0f);
    float f3 = (1.0f - f1) * 25.0f;
    float f4 = (1.0f - f1) * 25.0f;

    if (f3 == 0.0f || f4 == 0.0f) {
        D3DXMatrixScaling(&m3, 0.0f, 0.0f, 1.0f);
        m1 = m2 * m4 * m3;
    } else {
        float f5 = 1.0f / (f3 * 128.0f);
        float f6 = 1.0f / (f4 * 128.0f);
        D3DXMatrixScaling(&m3, f5, f6, 1.0f);
        m1 = m2 * m4 * m3;
    }

    DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m1);
#endif
    return 1;
}

void MaskTextureShader::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
#endif
}

int MaskTextureShader::Init()
{
    g_w3dShaders[W3DShaderManager::ST_MASK_TEXTURE] = &g_maskTextureShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_MASK_TEXTURE] = 1;
    return 1;
}
