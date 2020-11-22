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
#include "asciistring.h"
#include "texture.h"
class WorldHeightMap;

class TerrainTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(TerrainTextureClass)
public:
    TerrainTextureClass(int height);
    TerrainTextureClass(int width, int height);
#ifdef GAME_DLL
    TerrainTextureClass *Hook_Ctor1(int height) { return new (this) TerrainTextureClass(height); }
    TerrainTextureClass *Hook_Ctor2(int width, int height) { return new (this) TerrainTextureClass(width, height); }
#endif
    int Update(WorldHeightMap *ht_map);
    bool Update_Flat(WorldHeightMap *ht_map, int x, int y, int pixels_per_cell, unsigned int cell_width);
    void Set_LOD(int lod);
    void Apply(unsigned stage) override;
};

class AlphaTerrainTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(AlphaTerrainTextureClass)
public:
    AlphaTerrainTextureClass(TextureClass *base);
#ifdef GAME_DLL
    AlphaTerrainTextureClass *Hook_Ctor(TextureClass *base) { return new (this) AlphaTerrainTextureClass(base); }
#endif
    void Apply(unsigned stage) override;
};

class LightMapTerrainTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(LightMapTerrainTextureClass)
public:
    LightMapTerrainTextureClass(Utf8String name, MipCountType mips);
#ifdef GAME_DLL
    LightMapTerrainTextureClass *Hook_Ctor(Utf8String name, MipCountType mips)
    {
        return new (this) LightMapTerrainTextureClass(name, mips);
    }
#endif
    void Apply(unsigned stage) override;
};

class AlphaEdgeTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(AlphaEdgeTextureClass)
public:
    AlphaEdgeTextureClass(int height, MipCountType mips);
#ifdef GAME_DLL
    AlphaEdgeTextureClass *Hook_Ctor(int height, MipCountType mips)
    {
        return new (this) AlphaEdgeTextureClass(height, mips);
    }
#endif
    void Apply(unsigned stage) override;
    int Update(WorldHeightMap *ht_map);
};

class CloudMapTerrainTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(CloudMapTerrainTextureClass)
public:
    CloudMapTerrainTextureClass(MipCountType mips);
#ifdef GAME_DLL
    CloudMapTerrainTextureClass *Hook_Ctor(MipCountType mips) { return new (this) CloudMapTerrainTextureClass(mips); }
#endif
    void Apply(unsigned stage) override;
    void Restore();

protected:
    float m_xSlidePerSecond;
    float m_ySlidePerSecond;
    int m_curTick;
    float m_xOffset;
    float m_yOffset;
};

class ScorchTextureClass : public TextureClass
{
    IMPLEMENT_W3D_POOL(ScorchTextureClass)
public:
    ScorchTextureClass(MipCountType mips);
#ifdef GAME_DLL
    ScorchTextureClass *Hook_Ctor(MipCountType mips) { return new (this) ScorchTextureClass(mips); }
#endif
    void Apply(unsigned stage) override;
};
