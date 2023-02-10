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
#include "globaldata.h"
#include "shadermanager.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

int TerrainShader2Stage::Set(int pass)
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
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    }

    switch (pass) {
        case 0:
            DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(0));
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
            break;

        case 1:
            DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(1));
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 1);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            break;

        case 2:
            D3DXMATRIX m2;
            DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m2);
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
            D3DXMATRIX m;
            float f;
            D3DXMatrixInverse(&m, &f, &m2);

            if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_TERRAIN_NOISE12) {
                DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(2));
                Update_Noise_1(&m2, &m, true);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m2);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(3));
                Update_Noise_2(&m2, &m, true);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE1, m2);
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
            } else {
                if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_TERRAIN_NOISE1) {
                    DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(2));
                    Update_Noise_1(&m2, &m, true);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                } else {
                    DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(3));
                    Update_Noise_2(&m2, &m, true);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                }

                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m2);
            }
            break;
    }
#endif
    return 1;
}

void TerrainShader2Stage::Reset()
{
#ifdef BUILD_WITH_D3D8
    ShaderClass::Invalidate();
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, nullptr);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
#endif
}

int TerrainShader2Stage::Init()
{
    m_xSlidePerSecond = -0.02f;
    m_ySlidePerSecond = 1.5f * m_xSlidePerSecond;
    m_curTick = 0;
    m_curTick = W3D::Get_Sync_Time();
    m_xOffset = 0.0f;
    m_yOffset = 0.0f;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN] = &g_terrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN] = 2;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE1] = &g_terrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE1] = 3;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE2] = &g_terrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE2] = 3;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE12] = &g_terrainShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE12] = 3;
    return 1;
}

#ifdef BUILD_WITH_D3D8
void TerrainShader2Stage::Update_Noise_1(D3DXMATRIX *dest_matrix, D3DXMATRIX *cur_view_inverse, bool do_update)
{
    D3DXMATRIX m;
    D3DXMatrixScaling(&m, 0.0031746032f, 0.0031746032f, 1.0f);
    *dest_matrix = *cur_view_inverse * m;
    D3DXMATRIX m2;
    int tick = m_curTick;
    m_curTick = W3D::Get_Sync_Time();
    tick = m_curTick - tick;
    m_xOffset = (float)tick * m_xSlidePerSecond / 1000.0f + m_xOffset;
    m_yOffset = (float)tick * m_ySlidePerSecond / 1000.0f + m_yOffset;

    while (m_xOffset > 1.0f) {
        m_xOffset = m_xOffset - 1.0f;
    }

    while (m_yOffset > 1.0f) {
        m_yOffset = m_yOffset - 1.0f;
    }

    while (m_xOffset < -1.0f) {
        m_xOffset = m_xOffset + 1.0f;
    }

    while (m_yOffset < -1.0f) {
        m_yOffset = m_yOffset + 1.0f;
    }

    D3DXMatrixTranslation(&m2, m_xOffset, m_yOffset, 0.0f);
    *dest_matrix *= m2;
}

void TerrainShader2Stage::Update_Noise_2(D3DXMATRIX *dest_matrix, D3DXMATRIX *cur_view_inverse, bool do_update)
{
    D3DXMATRIX m;
    D3DXMatrixScaling(&m, 0.0031746032f, 0.0031746032f, 1.0f);
    *dest_matrix = *cur_view_inverse * m;
}
#endif

int TerrainShader8Stage::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    if (pass) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Invalidate_Cached_Render_States();
        g_terrainShader2Stage.Set(2);
    } else {
        DX8Wrapper::Apply_Render_State_Changes();
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

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
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        }

        DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(0));
        DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(1));
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_ADD);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_ADD);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_COMPLEMENT | D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture(2, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, 2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture(3, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, 3);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture(4, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_TEXCOORDINDEX, 4);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLORARG1, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture(5, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLOROP, D3DTOP_ADD);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_TEXCOORDINDEX, 5);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAOP, D3DTOP_ADD);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAARG1, D3DTA_COMPLEMENT | D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture(6, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_TEXCOORDINDEX, 6);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture(7, nullptr);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_TEXCOORDINDEX, 7);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }
#endif
    return 1;
}

void TerrainShader8Stage::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, nullptr);
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, nullptr);
    DX8Wrapper::Invalidate_Cached_Render_States();
#endif
}

int TerrainShader8Stage::Init()
{
    if (!g_terrainShader2Stage.Init()) {
        return 0;
    }

    if (W3DShaderManager::Get_Chipset() < GPU_TNT || W3DShaderManager::Get_Chipset() > GPU_GF2) {
        return 0;
    }

    g_w3dShaders[W3DShaderManager::ST_TERRAIN] = &g_terrainShader8Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN] = 1;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE1] = &g_terrainShader8Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE1] = 2;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE2] = &g_terrainShader8Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE2] = 2;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE12] = &g_terrainShader8Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE12] = 2;
    return 1;
}

int TerrainShaderPixelShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Get_D3D_Device8()->SetTexture(0, W3DShaderManager::Get_Shader_Texture(0)->Peek_Platform_Base_Texture());
    DX8Wrapper::Get_D3D_Device8()->SetTexture(1, W3DShaderManager::Get_Shader_Texture(1)->Peek_Platform_Base_Texture());
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);

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
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    }

    if (W3DShaderManager::Get_Current_Shader() < W3DShaderManager::ST_TERRAIN_NOISE1) {
        DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBasePixelShader);
    } else {
        D3DXMATRIX m;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
        D3DXMATRIX m2;
        float f;
        D3DXMatrixInverse(&m2, &f, &m);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

        if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_TERRAIN_NOISE12) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            DX8Wrapper::Get_D3D_Device8()->SetTexture(
                2, W3DShaderManager::Get_Shader_Texture(2)->Peek_Platform_Base_Texture());
            DX8Wrapper::Get_D3D_Device8()->SetTexture(
                3, W3DShaderManager::Get_Shader_Texture(3)->Peek_Platform_Base_Texture());
            DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBaseNoise2PixelShader);
            DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_MINFILTER, D3DTEXF_POINT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            g_terrainShader2Stage.Update_Noise_1(&m, &m2, true);
            DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE2, m);
            g_terrainShader2Stage.Update_Noise_2(&m, &m2, true);
            DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE3, m);
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        } else {
            DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBaseNoise1PixelShader);

            if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_TERRAIN_NOISE1) {
                DX8Wrapper::Get_D3D_Device8()->SetTexture(
                    2, W3DShaderManager::Get_Shader_Texture(2)->Peek_Platform_Base_Texture());
                g_terrainShader2Stage.Update_Noise_1(&m, &m2, true);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            } else {
                DX8Wrapper::Get_D3D_Device8()->SetTexture(
                    2, W3DShaderManager::Get_Shader_Texture(3)->Peek_Platform_Base_Texture());
                g_terrainShader2Stage.Update_Noise_2(&m, &m2, true);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
            }

            DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE2, m);
        }
    }
#endif
    return 1;
}

void TerrainShaderPixelShader::Reset()
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

int TerrainShaderPixelShader::Init()
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

    int i = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\terrain.pso", decl, 0, false, &m_dwBasePixelShader);

    if (i < 0) {
        return 0;
    }

    i = W3DShaderManager::Load_And_Create_D3D_Shader(
        "shaders\\terrainnoise.pso", decl, 0, false, &m_dwBaseNoise1PixelShader);

    if (i < 0) {
        return 0;
    }

    i = W3DShaderManager::Load_And_Create_D3D_Shader(
        "shaders\\terrainnoise2.pso", decl, 0, false, &m_dwBaseNoise2PixelShader);

    if (i < 0) {
        return 0;
    }

    g_w3dShaders[W3DShaderManager::ST_TERRAIN] = &g_terrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE1] = &g_terrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE2] = &g_terrainShaderPixelShader;
    g_w3dShaders[W3DShaderManager::ST_TERRAIN_NOISE12] = &g_terrainShaderPixelShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE1] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE2] = 1;
    g_w3dShadersPassCount[W3DShaderManager::ST_TERRAIN_NOISE12] = 1;
#endif
    return 1;
}

int TerrainShaderPixelShader::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (m_dwBasePixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBasePixelShader);
    }

    if (m_dwBaseNoise1PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBaseNoise1PixelShader);
    }

    if (m_dwBaseNoise2PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBaseNoise2PixelShader);
    }

    m_dwBasePixelShader = 0;
    m_dwBaseNoise1PixelShader = 0;
    m_dwBaseNoise2PixelShader = 0;
#endif
    return 1;
}
