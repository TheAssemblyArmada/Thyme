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
#include "baseheightmap.h"
#include "shadermanager.h"
#include "w3dshroud.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif
#ifdef GAME_DEBUG_STRUCTS
#include "globaldata.h"
#endif

int ShroudTextureShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    VertexMaterialClass *mat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(mat);

    if (mat) {
        mat->Release_Ref();
    }

    DX8Wrapper::Set_Texture(pass, W3DShaderManager::Get_Shader_Texture(0));

    if (!pass) {
#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
            DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaSpriteShader);
        } else {
            DX8Wrapper::Set_Shader(ShaderClass::s_presetMultiplicativeSpriteShader);
        }
#else
        DX8Wrapper::Set_Shader(ShaderClass::s_presetMultiplicativeSpriteShader);
#endif
    }

    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(pass, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_EQUAL);
    W3DShroud *shroud = g_theTerrainRenderObject->Get_Shroud();

    if (shroud) {
        D3DXMATRIX m;
        D3DXMATRIX m2;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m2);
        float f;
        D3DXMatrixInverse(&m, &f, &m2);
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
        m2 = m * m4 * m3;
        DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(pass + D3DTS_TEXTURE0), m2);
    }

    m_pass = pass;
#endif
    return 1;
}

void ShroudTextureShader::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Texture(m_pass, nullptr);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_TEXCOORDINDEX, m_pass);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_pass, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
#endif
}

int ShroudTextureShader::Init()
{
    g_w3dShaders[W3DShaderManager::ST_SHROUD_TEXTURE] = &g_shroudTextureShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_SHROUD_TEXTURE] = 1;
    return 1;
}
