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

int CloudTextureShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    D3DXMATRIX m;
    DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
    D3DXMATRIX m2;
    float f;
    D3DXMatrixInverse(&m2, &f, &m);
    g_terrainShader2Stage.Update_Noise_1(&m, &m2, false);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(pass + D3DTS_TEXTURE0), m);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_COLORARG2, D3DTA_CURRENT);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(
        pass, W3DShaderManager::Get_Shader_Texture(pass)->Peek_Platform_Base_Texture());
    m_pass = pass;
#endif
    return 1;
}

void CloudTextureShader::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(m_pass, nullptr);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_TEXCOORDINDEX, m_pass);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
#endif
}

int CloudTextureShader::Init()
{
    g_w3dShaders[W3DShaderManager::ST_CLOUD_TEXTURE] = &g_cloudTextureShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_CLOUD_TEXTURE] = 1;
    return 1;
}
