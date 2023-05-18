/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "terraintex.h"
#include "dx8wrapper.h"
#include "globaldata.h"
#include "worldheightmap.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

enum
{
    TERRAIN_TEXTURE_WIDTH = 2048
};

TerrainTextureClass::TerrainTextureClass(int height) :
    TextureClass(TERRAIN_TEXTURE_WIDTH, height, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_3, POOL_MANAGED, false, true)
{
}

TerrainTextureClass::TerrainTextureClass(int width, int height) :
    TextureClass(width, height, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_ALL, POOL_MANAGED, false, true)
{
}

int TerrainTextureClass::Update(WorldHeightMap *ht_map)
{
#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));

    if (desc.Width < TERRAIN_TEXTURE_WIDTH) {
        return 0;
    }

    D3DLOCKED_RECT rect;
    DX8Wrapper::Handle_DX8_ErrorCode(surf->LockRect(&rect, nullptr, 0));
    int width = 64;

    if (desc.Format == D3DFMT_A1R5G5B5) {
        for (int i = 0; i < ht_map->m_numBitmapTiles; i++) {
            TileData *tile = ht_map->Get_Source_Tile(i);

            if (tile) {
                int32_t x = tile->m_tileLocationInTexture.x;
                int32_t y = tile->m_tileLocationInTexture.y;

                if (x > 0) {
                    for (int j = 0; j < width; j++) {
                        unsigned char *data = tile->Get_RGB_Data_For_Width(width);
                        unsigned char *data2 = &data[width * 4 * (width - 1 - j)];
                        unsigned short *data3 = (unsigned short *)((char *)rect.pBits + 2 * desc.Width * (j + y) + 2 * x);

                        for (int k = 0; k < width; k++) {
                            *data3 = ((int)*data2 >> 3) + ((int)data2[2] >> 3 << 10) + 0x20 * ((int)data2[1] >> 3) + 0x8000;
                            data3++;
                            data2 += 4;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < ht_map->m_numTextureClasses; i++) {
            int x = ht_map->m_textureClasses[i].position_in_texture.x;
            int y = ht_map->m_textureClasses[i].position_in_texture.y;

            if (x > 0) {
                int w = ht_map->m_textureClasses[i].width << 6;

                for (int j = 0; j < w; j++) {
                    unsigned char *data = (unsigned char *)rect.pBits + 2 * desc.Width * (j + y) + 2 * x;
                    memcpy(data - 8, &data[2 * w - 8], 8);
                    memcpy(&data[2 * w], data, 8);
                }

                for (int j = 0; j < 4; j++) {
                    memcpy((char *)rect.pBits + 2 * desc.Width * (y - j - 1) + 2 * x - 8,
                        (char *)rect.pBits + 2 * desc.Width * (y - j - 1) + 2 * x + 2 * desc.Width * w - 8,
                        2 * (w + 8));
                    memcpy((char *)rect.pBits + 2 * desc.Width * (j + y) + 2 * x + 2 * desc.Width * w - 8,
                        (char *)rect.pBits + 2 * desc.Width * (j + y) + 2 * x - 8,
                        2 * (w + 8));
                }
            }
        }
    }

    surf->UnlockRect();
    surf->Release();
    DX8Wrapper::Handle_DX8_ErrorCode(
        D3DXFilterTexture(reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture()), nullptr, 0, D3DX_FILTER_BOX));

    if (g_theWriteableGlobalData->m_textureReductionFactor) {
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())
            ->SetLOD(g_theWriteableGlobalData->m_textureReductionFactor);
    }

    return desc.Height;
#else
    return 0;
#endif
}

bool TerrainTextureClass::Update_Flat(WorldHeightMap *ht_map, int x, int y, int pixels_per_cell, unsigned int cell_width)
{
#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));

    captainslog_dbgassert(desc.Width != cell_width * pixels_per_cell, "Bitmap too small.");
    captainslog_dbgassert(desc.Height != cell_width * pixels_per_cell, "Bitmap too small.");

    if (desc.Width == cell_width * pixels_per_cell) {
        D3DLOCKED_RECT rect;
        DX8Wrapper::Handle_DX8_ErrorCode(surf->LockRect(&rect, nullptr, 0));

        if (desc.Format == D3DFMT_A1R5G5B5) {
            for (int i = 0; i < pixels_per_cell; i++) {
                for (int j = 0; j < pixels_per_cell; j++) {
                    unsigned char *data = ht_map->Get_Pointer_To_Tile_Data(i + x, j + y, cell_width);

                    if (data) {
                        for (int k = cell_width - 1; k >= 0; k--) {
                            unsigned char *data3 = (unsigned char *)rect.pBits;
                            short *data2 = (short *)&data3[2 * desc.Width * (k + (pixels_per_cell - j - 1) * cell_width)
                                + 2 * cell_width * i];

                            for (int l = 0; l < (int)cell_width; l++) {
                                *data2 =
                                    ((int)*data >> 3) + ((int)data[2] >> 3 << 0xA) + 0x20 * ((int)data[1] >> 3) + 0x8000;
                                data2++;
                                data += 4;
                            }
                        }
                    }
                }
            }
        }

        surf->UnlockRect();
        surf->Release();
        DX8Wrapper::Handle_DX8_ErrorCode(
            D3DXFilterTexture(reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture()), nullptr, 0, D3DX_FILTER_BOX));
        return desc.Height != 0;
    }
#endif
    return false;
}

void TerrainTextureClass::Set_LOD(int lod)
{
#ifdef BUILD_WITH_D3D8
    if (Peek_Platform_Base_Texture()) {
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->SetLOD(lod);
    }
#endif
}

void TerrainTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);
}

AlphaTerrainTextureClass::AlphaTerrainTextureClass(TextureClass *base) :
    TextureClass(8, 8, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_1, POOL_MANAGED, false, true)
{
    Set_Platform_Base_Texture(base->Peek_Platform_Base_Texture());
}

void AlphaTerrainTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);

#ifdef BUILD_WITH_D3D8
    if (g_theWriteableGlobalData
        && (g_theWriteableGlobalData->m_bilinearTerrainTexture || g_theWriteableGlobalData->m_trilinearTerrainTexture)) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
    }

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    }

    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

    if (!stage) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 1);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1u, D3DTSS_COLOROP, D3DTOP_DISABLE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1u, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    } else {
        if (stage == 1) {
            if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_multiPassTerrain) {
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
                DX8Wrapper::Set_Texture(2, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, 2);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_COLORARG2, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_Texture(3, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, 3);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_Texture(4, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLOROP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_TEXCOORDINDEX, 4);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLORARG1, D3DTA_CURRENT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(4, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_Texture(5, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLOROP, D3DTOP_ADD);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_TEXCOORDINDEX, 5);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAOP, D3DTOP_ADD);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAARG1, D3DTA_COMPLEMENT | D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(5, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_Texture(6, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLOROP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_TEXCOORDINDEX, 6);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLORARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(6, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_Texture(7, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_TEXCOORDINDEX, 7);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLORARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(7, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
            } else {
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            }
        }
    }
#endif
}

LightMapTerrainTextureClass::LightMapTerrainTextureClass(Utf8String name, MipCountType mips) :
    TextureClass(name.Is_Empty() ? "TSNoiseUrb.tga" : name.Str(),
        name.Is_Empty() ? "TSNoiseUrb.tga" : name.Str(),
        mips,
        WW3D_FORMAT_UNKNOWN,
        true,
        true)
{
    Get_Texture_Filter()->Set_Min_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    Get_Texture_Filter()->Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
    Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
}

void LightMapTerrainTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);
}

AlphaEdgeTextureClass::AlphaEdgeTextureClass(int height, MipCountType mips) :
    TextureClass(TERRAIN_TEXTURE_WIDTH, height, WW3D_FORMAT_A8R8G8B8, mips, POOL_MANAGED, false, true)
{
}

void AlphaEdgeTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);
}

int AlphaEdgeTextureClass::Update(WorldHeightMap *ht_map)
{
#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));
    D3DLOCKED_RECT rect;
    DX8Wrapper::Handle_DX8_ErrorCode(surf->LockRect(&rect, nullptr, 0));
    int width = 64;

    if (desc.Format == D3DFMT_A8R8G8B8) {
        for (unsigned int i = 0; i < desc.Width; i++) {
            for (unsigned int j = 0; j < desc.Height; j++) {
                unsigned char *data = (unsigned char *)rect.pBits + 4 * (i + desc.Width * j);
                *((unsigned char *)rect.pBits + 4 * (i + desc.Width * j) + 2) = 0xFF - (int)j / 2;
                *data = i / 2;
                data[3] = i / 2;
                data[3] = 128;
            }

            for (int j = 0; j < ht_map->m_numEdgeTiles; j++) {
                TileData *tile = ht_map->Get_Edge_Tile(j);

                if (tile) {
                    int x = tile->m_tileLocationInTexture.x;
                    int y = tile->m_tileLocationInTexture.y;

                    if (x > 0) {
                        for (int k = 0; k < width; k++) {
                            TileData *tile2 = ht_map->Get_Edge_Tile(k);
                            unsigned char *data = tile2->Get_RGB_Data_For_Width(width);
                            unsigned char *data2 = &data[width * 4 * (width - 1 - k)];
                            unsigned char *data3 = (unsigned char *)rect.pBits + 4 * desc.Width * (k + y) + 4 * x;

                            for (int l = 0; l < width; l++) {
                                *data3 = *data2;
                                data3[1] = data2[1];
                                data3[2] = data2[2];

                                if (*data2 || data2[1] || data2[2]) {
                                    if (*data2 == 255 && data2[1] == 255 && data2[2] == 255) {
                                        data3[3] = 0;
                                    } else {
                                        data3[3] = 255;
                                    }
                                } else {
                                    data3[3] = 128;
                                }

                                data3 += 4;
                                data2 += 4;
                            }
                        }
                    }
                }
            }
        }
    }

    surf->UnlockRect();
    surf->Release();
    DX8Wrapper::Handle_DX8_ErrorCode(
        D3DXFilterTexture(reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture()), nullptr, 0, D3DX_FILTER_BOX));
    return desc.Height;
#else
    return 0;
#endif
}

CloudMapTerrainTextureClass::CloudMapTerrainTextureClass(MipCountType mips) :
    TextureClass("TSCloudMed.tga", "TSCloudMed.tga", mips, WW3D_FORMAT_UNKNOWN, true, true)
{
    Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_FAST);
    m_xSlidePerSecond = -0.02f;
    m_ySlidePerSecond = 1.5f * m_xSlidePerSecond;
    m_curTick = 0;
    m_xOffset = 0.0f;
    m_yOffset = 0.0f;
}

void CloudMapTerrainTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);
}

void CloudMapTerrainTextureClass::Restore()
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    if (g_theWriteableGlobalData) {
        if (!g_theWriteableGlobalData->m_multiPassTerrain) {
            for (int i = 0; i < MAX_TEXTURE_STAGES; i++) {
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, i);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                DX8Wrapper::Set_Texture(i, nullptr);
            }
        }
    }
#endif
}

ScorchTextureClass::ScorchTextureClass(MipCountType mips) :
    TextureClass("EXScorch01.tga", "EXScorch01.tga", mips, WW3D_FORMAT_UNKNOWN, true, true)
{
}

void ScorchTextureClass::Apply(unsigned stage)
{
    TextureClass::Apply(stage);

#ifdef BUILD_WITH_D3D8
    if (g_theWriteableGlobalData
        && (g_theWriteableGlobalData->m_bilinearTerrainTexture || g_theWriteableGlobalData->m_trilinearTerrainTexture)) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, D3DTEXF_POINT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
    }

    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_trilinearTerrainTexture) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    }

    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, TRUE);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
#endif
}
