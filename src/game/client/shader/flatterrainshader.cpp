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
#include "globaldata.h"
#include "shadermanager.h"
#include "w3dshroud.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int FlatTerrainShader2Stage::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Apply_Render_State_Changes();

    if (g_theWriteableGlobalData
        && (g_theWriteableGlobalData->m_bilinearTerrainTexture || g_theWriteableGlobalData->m_trilinearTerrainTexture)) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
    }

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    }

    if (pass) {
        if (pass == 1) {
            D3DXMATRIX m;
            DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ZERO);
            D3DXMATRIX m2;
            float f;
            D3DXMatrixInverse(&m2, &f, &m);

            if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE12) {
                g_terrainShader2Stage.Update_Noise_1(&m, &m2, true);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Get_D3D_Device8()->SetTexture(
                    0, W3DShaderManager::Get_Shader_Texture(2)->Peek_Platform_Base_Texture());
                g_terrainShader2Stage.Update_Noise_2(&m, &m2, true);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE1, m);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
                DX8Wrapper::Get_D3D_Device8()->SetTexture(
                    1, W3DShaderManager::Get_Shader_Texture(3)->Peek_Platform_Base_Texture());
            } else {
                if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE1) {
                    DX8Wrapper::Get_D3D_Device8()->SetTexture(
                        0, W3DShaderManager::Get_Shader_Texture(2)->Peek_Platform_Base_Texture());
                    g_terrainShader2Stage.Update_Noise_1(&m, &m2, true);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                } else {
                    DX8Wrapper::Get_D3D_Device8()->SetTexture(
                        0, W3DShaderManager::Get_Shader_Texture(1)->Peek_Platform_Base_Texture());
                    g_terrainShader2Stage.Update_Noise_2(&m, &m2, true);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                }

                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m);
            }
        }
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        if (W3DShaderManager::Get_Shader_Texture(0)) {
            DX8Wrapper::Get_D3D_Device8()->SetTexture(
                0, W3DShaderManager::Get_Shader_Texture(0)->Peek_Platform_Base_Texture());
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            W3DShroud *shroud = g_theTerrainRenderObject->Get_Shroud();

            if (shroud) {
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
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m);
            }
        } else {
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
        }

        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
    }
#endif
    return 1;
}

void FlatTerrainShader2Stage::Reset()
{
#ifdef BUILD_WITH_D3D8
    ShaderClass::Invalidate();
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, 0);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
#endif
}

int FlatTerrainShader2Stage::Init()
{
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN] = &g_flatTerrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN] = 1;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE1] = &g_flatTerrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE1] = 2;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE2] = &g_flatTerrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE2] = 2;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE12] = &g_flatTerrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE12] = 2;
    return 1;
}

int FlatTerrainShaderPixelShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    int i = 1;
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(2));
    DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(2));
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

    if (g_theWriteableGlobalData
        && (g_theWriteableGlobalData->m_bilinearTerrainTexture || g_theWriteableGlobalData->m_trilinearTerrainTexture)) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MAGFILTER, D3DTEXF_POINT);
    }

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    }

    i = 0;
    W3DShroud *shroud = g_theTerrainRenderObject->Get_Shroud();

    if (shroud) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
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
        DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(i + D3DTS_TEXTURE0), m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Get_D3D_Device8()->SetTexture(i, shroud->Get_Shroud_Texture()->Peek_Platform_Base_Texture());

        if (++i == 1) {
            i = 2;
        }
    }
    if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE1
        || W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE12) {
        D3DXMATRIX m;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
        D3DXMATRIX m2;
        float f;
        D3DXMatrixInverse(&m2, &f, &m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSU, 1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSV, 1);
        DX8Wrapper::Get_D3D_Device8()->SetTexture(i, W3DShaderManager::Get_Shader_Texture(2)->Peek_Platform_Base_Texture());
        g_terrainShader2Stage.Update_Noise_1(&m, &m2, true);
        DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(i + D3DTS_TEXTURE0), m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i++, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

        if (i == 1) {
            i = 2;
        }
    }

    if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE2
        || W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_FLAT_TERRAIN_NOISE12) {
        D3DXMATRIX m;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
        D3DXMATRIX m2;
        float f;
        D3DXMatrixInverse(&m2, &f, &m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        DX8Wrapper::Get_D3D_Device8()->SetTexture(i, W3DShaderManager::Get_Shader_Texture(3)->Peek_Platform_Base_Texture());
        g_terrainShader2Stage.Update_Noise_2(&m, &m2, true);
        DX8Wrapper::Set_DX8_Transform((D3DTRANSFORMSTATETYPE)(i + D3DTS_TEXTURE0), m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i++, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        if (i == 1) {
            i = 2;
        }
    }

    if (i >= 2) {
        switch (i) {
            case 2:
                DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBase1PixelShader);
                break;
            case 3:
                DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBaseNoise1PixelShader);
                break;
            case 4:
                DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBaseNoise2PixelShader);
                break;
        }
    } else {
        DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBase2PixelShader);
    }

    DX8Wrapper::Get_D3D_Device8()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Get_D3D_Device8()->SetTexture(i, W3DShaderManager::Get_Shader_Texture(3)->Peek_Platform_Base_Texture());
#endif
    return 1;
}

void FlatTerrainShaderPixelShader::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetTexture(2, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(3, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, nullptr);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, 2);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, 3);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}

int FlatTerrainShaderPixelShader::Init()
{
#ifdef BUILD_WITH_D3D8
    if (W3DShaderManager::Get_Chipset() <= GPU_PS11) {
        return 0;
    }

    DWORD decl[] = { D3DVSD_STREAM(0),
        D3DVSD_REG(0, D3DVSDT_FLOAT3),
        D3DVSD_REG(1, D3DVSDT_D3DCOLOR),
        D3DVSD_REG(2, D3DVSDT_FLOAT2),
        D3DVSD_REG(3, D3DVSDT_FLOAT2),
        D3DVSD_END() };

    int i = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\fterrain.pso", decl, 0, false, &m_dwBase1PixelShader);

    if (i < 0) {
        return 0;
    }

    i = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\fterrain0.pso", decl, 0, false, &m_dwBase2PixelShader);

    if (i < 0) {
        return 0;
    }

    i = W3DShaderManager::Load_And_Create_D3D_Shader(
        "shaders\\fterrainnoise.pso", decl, 0, false, &m_dwBaseNoise1PixelShader);

    if (i < 0) {
        return 0;
    }

    i = W3DShaderManager::Load_And_Create_D3D_Shader(
        "shaders\\fterrainnoise2.pso", decl, 0, false, &m_dwBaseNoise2PixelShader);

    if (i < 0) {
        return 0;
    }

    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN] = &g_flatTerrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE1] = &g_flatTerrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE2] = &g_flatTerrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_FLAT_TERRAIN_NOISE12] = &g_flatTerrainShaderPixelShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE1] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE2] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_FLAT_TERRAIN_NOISE12] = 1;
#endif
    return 1;
}

int FlatTerrainShaderPixelShader::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (m_dwBase1PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBase1PixelShader);
    }

    if (m_dwBase2PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBase2PixelShader);
    }

    if (m_dwBaseNoise1PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBaseNoise1PixelShader);
    }

    if (m_dwBaseNoise2PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBaseNoise2PixelShader);
    }

    m_dwBase1PixelShader = 0;
    m_dwBase2PixelShader = 0;
    m_dwBaseNoise1PixelShader = 0;
    m_dwBaseNoise2PixelShader = 0;
#endif
    return 1;
}
