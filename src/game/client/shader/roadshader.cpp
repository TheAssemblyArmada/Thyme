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

int RoadShaderPixelShader::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(0));
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    D3DXMATRIX m;
    DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
    D3DXMATRIX m2;
    float f;
    D3DXMatrixInverse(&m2, &f, &m);

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    }

    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(1));
    DX8Wrapper::Set_Texture(2, W3DShaderManager::Get_Shader_Texture(2));
    DX8Wrapper::Get_D3D_Device8()->SetPixelShader(m_dwBaseNoise2PixelShader);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_terrainShader2Stage.Update_Noise_1(&m, &m2, false);
    DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE1, m);
    g_terrainShader2Stage.Update_Noise_2(&m, &m2, false);
    DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE2, m);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
    return 1;
}

void RoadShaderPixelShader::Reset()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_D3D_Device8()->SetPixelShader(0);
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

int RoadShaderPixelShader::Init()
{
#ifdef BUILD_WITH_D3D8
    if (!g_roadShader2Stage.Init()) {
        return 0;
    }

    if (W3DShaderManager::Get_Chipset() <= GPU_PS11) {
        return 0;
    }

    DWORD decl[] = { D3DVSD_STREAM(0),
        D3DVSD_REG(0, D3DVSDT_FLOAT3),
        D3DVSD_REG(1, D3DVSDT_D3DCOLOR),
        D3DVSD_REG(2, D3DVSDT_FLOAT2),
        D3DVSD_END() };

    int i =
        W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\roadnoise2.pso", decl, 0, false, &m_dwBaseNoise2PixelShader);

    if (i < 0) {
        return 0;
    }

    g_w3dShaders[W3DShaderManager::ST_ROAD_NOISE12] = &g_roadShaderPixelShader;
    g_w3dShadersPassCount[W3DShaderManager::ST_ROAD_NOISE12] = 1;
#endif
    return 1;
}

int RoadShaderPixelShader::Shutdown()
{
#ifdef BUILD_WITH_D3D8
    if (m_dwBaseNoise2PixelShader) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwBaseNoise2PixelShader);
    }

    m_dwBaseNoise2PixelShader = 0;
#endif
    return 1;
}

int RoadShader2Stage::Set(int pass)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Texture(0, W3DShaderManager::Get_Shader_Texture(0));
    DX8Wrapper::Apply_Render_State_Changes();
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);

    if (pass) {
        D3DXMATRIX m;
        DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
        D3DXMATRIX m2;
        float f;
        D3DXMatrixInverse(&m2, &f, &m);

        if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        } else {
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        }

        DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(2));
        g_terrainShader2Stage.Update_Noise_2(&m, &m2, false);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_ALPHAREPLICATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE0, m);
    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        if (W3DShaderManager::Get_Current_Shader() < W3DShaderManager::ST_ROAD_NOISE1) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        } else {
            D3DXMATRIX m;
            DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, m);
            D3DXMATRIX m2;
            float f;
            D3DXMatrixInverse(&m2, &f, &m);

            if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
            } else {
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
            }

            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

            if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_ROAD_NOISE12) {
                DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(1));
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                g_terrainShader2Stage.Update_Noise_1(&m, &m2, false);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE1, m);
            } else {
                if (W3DShaderManager::Get_Current_Shader() == W3DShaderManager::ST_ROAD_NOISE1) {
                    DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(1));
                    g_terrainShader2Stage.Update_Noise_1(&m, &m2, false);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                } else {
                    DX8Wrapper::Set_Texture(1, W3DShaderManager::Get_Shader_Texture(2));
                    g_terrainShader2Stage.Update_Noise_2(&m, &m2, false);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
                }

                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                DX8Wrapper::Set_DX8_Transform(D3DTS_TEXTURE1, m);
            }
        }
    }
#endif
    return 1;
}

void RoadShader2Stage::Reset()
{
#ifdef BUILD_WITH_D3D8
    ShaderClass::Invalidate();
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
#endif
}

int RoadShader2Stage::Init()
{
    g_w3dShaders[W3DShaderManager::ST_ROAD] = &g_roadShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_ROAD] = 1;
    g_w3dShaders[W3DShaderManager::ST_ROAD_NOISE1] = &g_roadShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_ROAD_NOISE1] = 1;
    g_w3dShaders[W3DShaderManager::ST_ROAD_NOISE2] = &g_roadShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_ROAD_NOISE2] = 1;
    g_w3dShaders[W3DShaderManager::ST_ROAD_NOISE12] = &g_roadShader2Stage;
    g_w3dShadersPassCount[W3DShaderManager::ST_ROAD_NOISE12] = 2;
    return 1;
}
