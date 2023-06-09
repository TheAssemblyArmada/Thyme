/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shroud code
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dshroud.h"
#include "baseheightmap.h"
#include "dx8wrapper.h"
#include "globaldata.h"
#include "partitionmanager.h"
#include "shadermanager.h"
#include "texture.h"
#include "textureloader.h"
#include "worldheightmap.h"
#include <cstring>

using std::memset;

W3DShroud::W3DShroud() :
    // BUGFIX init all members
    m_numCellsX(0),
    m_numCellsY(0),
    m_numMaxVisibleCellsX(0),
    m_numMaxVisibleCellsY(0),
    m_cellWidth(10.0f),
    m_cellHeight(10.0f),
    m_shroudData(nullptr),
#ifdef BUILD_WITH_D3D8
    m_pSrcTexture(nullptr),
#endif
    m_srcTextureData(nullptr),
    m_srcTexturePitch(0),
    m_pDstTexture(nullptr),
    m_dstTextureWidth(0),
    m_dstTextureHeight(0),
    m_shroudFilter(TextureFilterClass::FILTER_TYPE_DEFAULT),
    m_drawOriginX(0),
    m_drawOriginY(0),
    m_drawFogOfWar(false),
    m_fillBorderShroudData(true),
    m_shroudAlpha(g_theWriteableGlobalData->m_shroudAlpha),
    m_srcFogArray(nullptr),
    m_destFogArray(nullptr)
{
}

W3DShroud ::~W3DShroud()
{
    Release_Resources();

#ifdef BUILD_WITH_D3D8
    if (m_pSrcTexture) {
        m_pSrcTexture->Release();
    }

    m_pSrcTexture = nullptr;
#endif

    if (m_srcFogArray != nullptr) {
        delete[] m_srcFogArray;
    }

    if (m_destFogArray != nullptr) {
        delete[] m_destFogArray;
    }

    m_drawFogOfWar = false;
}

void W3DShroud::Init(WorldHeightMap *map, float world_cell_size_x, float world_cell_size_y)
{
#ifdef BUILD_WITH_D3D8
    captainslog_dbgassert(!m_pSrcTexture, "ReAcquire of existing shroud textures");
    captainslog_dbgassert(map, "Shroud init with NULL WorldHeightMap");
    unsigned int width = 0;
    unsigned int height = 0;
    m_cellWidth = world_cell_size_x;
    m_cellHeight = world_cell_size_y;

    if (map != nullptr) {
        m_numCellsX = GameMath::Fast_To_Int_Ceil(
            (float)(int)((map->Get_X_Extent() - 1) - 2 * map->Border_Size()) * 10.0f / m_cellWidth);
        m_numCellsY = GameMath::Fast_To_Int_Ceil(
            (float)(int)((map->Get_Y_Extent() - 1) - 2 * map->Border_Size()) * 10.0f / m_cellHeight);
        m_numMaxVisibleCellsX = GameMath::Fast_To_Int_Floor((float)(map->Get_Draw_Width() - 1) * 10.0f / m_cellWidth) + 1;
        width = m_numMaxVisibleCellsX;
        m_numMaxVisibleCellsY = GameMath::Fast_To_Int_Floor((float)(map->Get_Draw_Height() - 1) * 10.0f / m_cellHeight) + 1;
        height = m_numMaxVisibleCellsY;
        width = m_numCellsX;
        height = m_numCellsY;
        width += 2;
        unsigned int volume = 1;
        height += 2;
        TextureLoader::Validate_Texture_Size(width, height, volume);
    }

    int x = m_numCellsX;
    int y = m_numCellsY;
    y++;

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
        m_pSrcTexture = DX8Wrapper::Create_Surface(x, y, WW3D_FORMAT_A4R4G4B4);
    } else {
        m_pSrcTexture = DX8Wrapper::Create_Surface(x, y, WW3D_FORMAT_R5G6B5);
    }
#else
    m_pSrcTexture = DX8Wrapper::Create_Surface(x, y, WW3D_FORMAT_R5G6B5);
#endif

    captainslog_dbgassert(m_pSrcTexture, "Failed to Allocate Shroud Src Surface");
    D3DLOCKED_RECT r;
    HRESULT res = m_pSrcTexture->LockRect(&r, nullptr, D3DLOCK_NO_DIRTY_UPDATE);
    m_pSrcTexture->UnlockRect();
    captainslog_dbgassert(SUCCEEDED(res), "Failed to lock shroud src surface");
    m_srcTextureData = static_cast<unsigned char *>(r.pBits);
    m_srcTexturePitch = r.Pitch;
    memset(m_srcTextureData, 0, y * m_srcTexturePitch);

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
        Fill_Shroud_Data(g_theWriteableGlobalData->m_shroudAlpha);
    }
#endif

    if (width != m_dstTextureWidth || height != m_dstTextureHeight) {
        Release_Resources();
    }

    if (m_pDstTexture == nullptr) {
        m_dstTextureWidth = width;
        m_dstTextureHeight = height;
        Re_Acquire_Resources();
    }

    if (g_thePartitionManager) {
        g_thePartitionManager->Refresh_Shroud_For_Local_Player();
    }
#endif
}

void W3DShroud::Reset()
{
#ifdef BUILD_WITH_D3D8
    if (m_pSrcTexture) {
        m_pSrcTexture->Release();
    }

    m_pSrcTexture = nullptr;
#endif

    if (m_srcFogArray != nullptr) {
        delete[] m_srcFogArray;
    }

    m_srcFogArray = nullptr;

    if (m_destFogArray != nullptr) {
        delete[] m_destFogArray;
    }

    m_destFogArray = nullptr;
    m_fillBorderShroudData = true;
}

void W3DShroud::Release_Resources()
{
    Ref_Ptr_Release(m_pDstTexture);
}

bool W3DShroud::Re_Acquire_Resources()
{
    if (m_dstTextureWidth == 0) {
        return true;
    }

    captainslog_dbgassert(!m_pDstTexture, "ReAcquire of existing shroud texture");

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
        m_pDstTexture = new TextureClass(
            m_dstTextureWidth, m_dstTextureHeight, WW3D_FORMAT_A4R4G4B4, MIP_LEVELS_1, POOL_DEFAULT, false, true);
    } else {
        m_pDstTexture = new TextureClass(
            m_dstTextureWidth, m_dstTextureHeight, WW3D_FORMAT_R5G6B5, MIP_LEVELS_1, POOL_DEFAULT, false, true);
    }
#else
    m_pDstTexture =
        new TextureClass(m_dstTextureWidth, m_dstTextureHeight, WW3D_FORMAT_R5G6B5, MIP_LEVELS_1, POOL_DEFAULT, false, true);
#endif
    captainslog_dbgassert(m_pDstTexture, "Failed ReAcquire of shroud texture");

    if (m_pDstTexture != nullptr) {
        m_pDstTexture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        m_pDstTexture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        m_pDstTexture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
        m_fillBorderShroudData = true;
        return true;
    }

    m_dstTextureWidth = 0;
    m_dstTextureHeight = 0;
    return false;
}

unsigned char W3DShroud::Get_Shroud_Level(int x, int y)
{
    captainslog_dbgassert(m_pSrcTexture, "Reading empty shroud");

    if (x < m_numCellsX && y < m_numCellsY) {
        unsigned short s = *reinterpret_cast<unsigned short *>(&m_srcTextureData[2 * x + m_srcTexturePitch * y]);
#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
            return (int)((1.0f - (float)((int)s >> 12) / 15.0f) * 255.0f);
        } else {
            return (int)((float)(((int)s >> 5) & 0x3F) / 63.0f * 255.0f);
        }
#else
        return (int)((float)(((int)s >> 5) & 0x3F) / 63.0f * 255.0f);
#endif
    }

    return 0;
}

void W3DShroud::Set_Shroud_Level(int x, int y, unsigned char level, bool unk)
{
    captainslog_dbgassert(m_pSrcTexture, "Writing empty shroud.  Usually means that map failed to load.");

    if (m_pSrcTexture) {
        if (x < m_numCellsX && y < m_numCellsY) {

            if (level < (int)g_theWriteableGlobalData->m_shroudAlpha) {
                level = g_theWriteableGlobalData->m_shroudAlpha;
            }

#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
                *reinterpret_cast<unsigned short *>(&m_srcTextureData[2 * x + m_srcTexturePitch * y]) =
                    ((((0xFF - level) >> 4) & 0xF) << 12)
                    | ((((int)g_theWriteableGlobalData->m_shroudColor.red >> 4) & 0xF) << 8)
                    | (16 * (((int)g_theWriteableGlobalData->m_shroudColor.green >> 4) & 0xF))
                    | ((int)g_theWriteableGlobalData->m_shroudColor.blue >> 4) & 0xF;
            } else {
                float c = (float)level;
                int r = (int)((float)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
                int g = (int)((float)((g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
                int b =
                    (int)((float)((g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

                if (level == 0xFF) {
                    b = 0xFF;
                    g = 0xFF;
                    r = 0xFF;
                }

                *reinterpret_cast<unsigned short *>(&m_srcTextureData[2 * x + m_srcTexturePitch * y]) =
                    ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
            }
#else
            float c = (float)level;
            int r = (int)((float)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
            int g = (int)((float)((g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
            int b = (int)((float)((g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

            if (level == 0xFF) {
                b = 0xFF;
                g = 0xFF;
                r = 0xFF;
            }

            *reinterpret_cast<unsigned short *>(&m_srcTextureData[2 * x + m_srcTexturePitch * y]) =
                ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
#endif
        }
    }
}

void W3DShroud::Fill_Shroud_Data(unsigned char level)
{
    if (level < (int)g_theWriteableGlobalData->m_shroudAlpha) {
        level = g_theWriteableGlobalData->m_shroudAlpha;
    }

    unsigned short data;

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
        data = ((((0xFF - level) >> 4) & 0xF) << 12) | ((((int)g_theWriteableGlobalData->m_shroudColor.red >> 4) & 0xF) << 8)
            | (0x10 * (((int)g_theWriteableGlobalData->m_shroudColor.green >> 4) & 0xF))
            | ((int)g_theWriteableGlobalData->m_shroudColor.blue >> 4) & 0xF;
    } else {
        float c = (float)level;
        int r = (int)((double)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
        int g = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
        int b = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

        if (level == 0xFF) {
            b = 0xFF;
            g = 0xFF;
            r = 0xFF;
        }

        data = ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
    }
#else
    float c = (float)level;
    int r = (int)((double)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
    int g = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
    int b = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

    if (level == 0xFF) {
        b = 0xFF;
        g = 0xFF;
        r = 0xFF;
    }

    data = ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
#endif

    unsigned char *src = m_srcTextureData;
    int pitch = m_srcTexturePitch >> 1;

    for (int i = 0; i < m_numCellsY; i++) {
        for (int j = 0; j < m_numCellsX; j++) {
            *reinterpret_cast<unsigned short *>(&src[2 * j]) = data;
        }

        src += 2 * pitch;
    }
}

void W3DShroud::Fill_Border_Shroud_Data(unsigned char level, SurfaceClass *surface)
{
#ifdef BUILD_WITH_D3D8
    if (level < (int)g_theWriteableGlobalData->m_shroudAlpha) {
        level = g_theWriteableGlobalData->m_shroudAlpha;
    }

    unsigned short data;

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn) {
        data = ((((0xFF - level) >> 4) & 0xF) << 12) | ((((int)g_theWriteableGlobalData->m_shroudColor.red >> 4) & 0xF) << 8)
            | (0x10 * (((int)g_theWriteableGlobalData->m_shroudColor.green >> 4) & 0xF))
            | ((int)g_theWriteableGlobalData->m_shroudColor.blue >> 4) & 0xF;
    } else {
        float c = (float)level;
        int r = (int)((double)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
        int g = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
        int b = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

        if (level == 0xFF) {
            b = 0xFF;
            g = 0xFF;
            r = 0xFF;
        }

        data = ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
    }
#else
    float c = (float)level;
    int r = (int)((double)(unsigned char)g_theWriteableGlobalData->m_shroudColor.Get_As_Int() / 255.0f * c);
    int g = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF00) >> 8) / 255.0f * c);
    int b = (int)((double)((int)(g_theWriteableGlobalData->m_shroudColor.Get_As_Int() & 0xFF0000) >> 0x10) / 255.0f * c);

    if (level == 0xFF) {
        b = 0xFF;
        g = 0xFF;
        r = 0xFF;
    }

    data = ((b & 0xF8) << 8) | (8 * (g & 0xFC)) | ((unsigned char)(r & 0xF8) >> 3);
#endif
    unsigned char *src = &m_srcTextureData[2 * (m_srcTexturePitch >> 1) * m_numCellsY];

    for (int i = 0; i < m_numCellsX; i++) {
        *reinterpret_cast<unsigned short *>(&src[2 * i]) = data;
    }

    RECT src_rect;
    src_rect.left = 0;
    src_rect.top = m_numCellsY;
    src_rect.right = m_numCellsX;
    src_rect.bottom = m_numCellsY + 1;

    POINT point;
    point.x = 0;
    point.y = 0;
    unsigned int i1 = m_dstTextureWidth / src_rect.right;
    unsigned int i2 = m_dstTextureWidth % src_rect.right;

    for (unsigned int j = 0; j < m_dstTextureHeight; j++) {
        point.y = j;
        point.x = 0;

        for (int i = 0; i < (int)i1; i++) {
            point.x = src_rect.right * i;
            DX8Wrapper::Copy_DX8_Rects(m_pSrcTexture, &src_rect, 1, surface->Peek_D3D_Surface(), &point);
        }

        if (i2) {
            unsigned int right = src_rect.right;
            point.x = src_rect.right * i1;
            src_rect.right = i2;
            DX8Wrapper::Copy_DX8_Rects(m_pSrcTexture, &src_rect, 1, surface->Peek_D3D_Surface(), &point);
            src_rect.right = right;
        }
    }
#endif
}

void W3DShroud::Set_Border_Shroud_Level(unsigned char level)
{
    m_shroudAlpha = level;
    m_fillBorderShroudData = true;
}

void W3DShroud::Render(CameraClass *cam)
{
#ifdef BUILD_WITH_D3D8
    if (m_pSrcTexture != nullptr) {
        if (DX8Wrapper::Get_D3D_Device8() == nullptr || SUCCEEDED(DX8Wrapper::Get_D3D_Device8()->TestCooperativeLevel())) {
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_fogOfWarOn != m_drawFogOfWar) {
                Reset();
                Release_Resources();
                Init(g_theTerrainRenderObject->Get_Map(), m_cellHeight, m_cellWidth);
                g_thePartitionManager->Refresh_Shroud_For_Local_Player();
                m_drawFogOfWar = g_theWriteableGlobalData->m_fogOfWarOn;
                m_fillBorderShroudData = true;
            }
#endif
            captainslog_dbgassert(m_pSrcTexture, "Updating unallocated shroud texture");

            // TODO investigate dead code

            int visible_start_x = 0;
            int visible_start_y = 0;

            // TODO investigate dead code

            int visible_end_x = m_numCellsX;
            int visible_end_y = m_numCellsY;
            m_drawOriginX = (float)visible_start_x * m_cellWidth;
            m_drawOriginY = (float)visible_start_y * m_cellHeight;

            if (m_pDstTexture->Get_Texture_Filter()->Get_Mag_Filter() != m_shroudFilter) {
                m_pDstTexture->Get_Texture_Filter()->Set_Mag_Filter(m_shroudFilter);
                m_pDstTexture->Get_Texture_Filter()->Set_Min_Filter(m_shroudFilter);
            }

            SurfaceClass *surface = m_pDstTexture->Get_Surface_Level(0);

            POINT point;
            point.x = 1;
            point.y = 1;

            RECT src_rect;
            src_rect.left = visible_start_x;
            src_rect.top = visible_start_y;
            src_rect.right = visible_end_x;
            src_rect.bottom = visible_end_y;

            if (m_fillBorderShroudData) {
                m_fillBorderShroudData = false;
                Fill_Border_Shroud_Data(m_shroudAlpha, surface);
            }

            DX8Wrapper::Copy_DX8_Rects(m_pSrcTexture, &src_rect, 1, surface->Peek_D3D_Surface(), &point);
            Ref_Ptr_Release(surface);
        }
    }
#endif
}

void W3DShroud::Set_Shroud_Filter(bool enable)
{
    if (enable) {
        m_shroudFilter = TextureFilterClass::FILTER_TYPE_DEFAULT;
    } else {
        m_shroudFilter = TextureFilterClass::FILTER_TYPE_NONE;
    }
}

void W3DShroudMaterialPassClass::Install_Materials()
{
    if (g_theTerrainRenderObject->Get_Shroud()) {
        W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
        W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
    }
}

void W3DShroudMaterialPassClass::UnInstall_Materials()
{
    W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
}

void W3DMaskMaterialPassClass::Install_Materials()
{
    W3DShaderManager::Set_Shader(W3DShaderManager::ST_MASK_TEXTURE, 0);
}

void W3DMaskMaterialPassClass::UnInstall_Materials()
{
    if (m_resetShader) {
        W3DShaderManager::Reset_Shader(W3DShaderManager::ST_MASK_TEXTURE);
    }
}
