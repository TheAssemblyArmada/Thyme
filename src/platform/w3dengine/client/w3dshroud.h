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
#pragma once
#include "always.h"
#include "texture.h"
#include "w3dtypes.h"

class CameraClass;
class WorldHeightMap;

class W3DShroud
{
public:
    W3DShroud();
    ~W3DShroud();

    TextureClass *Get_Shroud_Texture() { return m_pDstTexture; }
    float Get_Cell_Width() { return m_cellWidth; }
    float Get_Cell_Height() { return m_cellHeight; }
    int Get_Texture_Width() { return m_dstTextureWidth; }
    int Get_Texture_Height() { return m_dstTextureHeight; }
    float Get_Draw_Origin_X() { return m_drawOriginX; }
    float Get_Draw_Origin_Y() { return m_drawOriginY; }

    void Init(WorldHeightMap *map, float worldx, float worldy);
    void Reset();
    void Release_Resources();
    void Re_Acquire_Resources();
    unsigned char Get_Shroud_Level(int x, int y);
    void Set_Shroud_Level(int x, int y, unsigned char level, bool b);
    void Fill_Shroud_Data(unsigned char level);
    void Fill_Border_Shroud_Data(unsigned char level, w3dsurface_t surface);
    void Set_Border_Shroud_Level(unsigned char level);
    void Render(CameraClass *cam);
#ifdef BUILD_WITH_D3D8
    void Interpolate_Fog_Levels(RECT *rect);
#endif
    void Set_Shroud_Filter(bool enable);

private:
    int m_numCellsX;
    int m_numCellsY;
    int m_numMaxVisibleCellsX;
    int m_numMaxVisibleCellsY;
    float m_cellWidth;
    float m_cellHeight;
    char *m_shroudData;
    w3dsurface_t m_pSrcTexture;
    void *m_srcTextureData;
    unsigned int m_srcTexturePitch;
    TextureClass *m_pDstTexture;
    int m_dstTextureWidth;
    int m_dstTextureHeight;
    TextureFilterClass::FilterType m_shroudFilter;
    float m_drawOriginX;
    float m_drawOriginY;
    char m_drawFogOfWar;
    char m_fillBorderShroudData;
    char m_shroudAlpha;
    unsigned char *m_fogArray1;
    unsigned char *m_fogArray2;
};
