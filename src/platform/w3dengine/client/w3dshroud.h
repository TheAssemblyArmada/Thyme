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
#pragma once
#include "always.h"
#include "matpass.h"
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
    bool Re_Acquire_Resources();
    unsigned char Get_Shroud_Level(int x, int y);
    void Set_Shroud_Level(int x, int y, unsigned char level, bool unk);
    void Fill_Shroud_Data(unsigned char level);
    void Fill_Border_Shroud_Data(unsigned char level, SurfaceClass *surface);
    void Set_Border_Shroud_Level(unsigned char level);
    void Render(CameraClass *cam);
    void Set_Shroud_Filter(bool enable);

#ifdef GAME_DLL
    W3DShroud *Hook_Ctor() { return new (this) W3DShroud(); }
    void Hook_Dtor() { W3DShroud::~W3DShroud(); }
#endif

private:
    int m_numCellsX;
    int m_numCellsY;
    int m_numMaxVisibleCellsX;
    int m_numMaxVisibleCellsY;
    float m_cellWidth;
    float m_cellHeight;
    char *m_shroudData;
    w3dsurface_t m_pSrcTexture;
    unsigned char *m_srcTextureData;
    unsigned int m_srcTexturePitch;
    TextureClass *m_pDstTexture;
    unsigned int m_dstTextureWidth;
    unsigned int m_dstTextureHeight;
    TextureFilterClass::FilterType m_shroudFilter;
    float m_drawOriginX;
    float m_drawOriginY;
    bool m_drawFogOfWar;
    bool m_fillBorderShroudData;
    unsigned char m_shroudAlpha;
    unsigned char *m_srcFogArray;
    unsigned char *m_destFogArray;
};

class W3DShroudMaterialPassClass : public MaterialPassClass
{
public:
    W3DShroudMaterialPassClass() : m_unk(false) {}
    virtual ~W3DShroudMaterialPassClass() override {}
    virtual void Install_Materials() override;
    virtual void UnInstall_Materials() override;

private:
    bool m_unk;
};

class W3DMaskMaterialPassClass : public MaterialPassClass
{
public:
    W3DMaskMaterialPassClass() : m_unk(0), m_resetShader(true) {}
    virtual ~W3DMaskMaterialPassClass() override {}
    virtual void Install_Materials() override;
    virtual void UnInstall_Materials() override;
    void Reset_Shader(bool b) { m_resetShader = b; }

private:
    int m_unk;
    bool m_resetShader;
};