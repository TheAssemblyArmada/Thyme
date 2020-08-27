/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief World Height Map
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "refcount.h"
#include "coord.h"
#include <vector>

class TileData;
class TerrainTextureClass;
class AlphaTerrainTextureClass;
class AlphaEdgeTextureClass;

class WorldHeightMapInterfaceClass
{
public:
    virtual int Get_Border_Size() = 0;
    virtual float Get_Seismic_Z_Velocity(int x, int y) = 0;
    virtual void Set_Seismic_Z_Velocity(int x, int y, float velocity) = 0;
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y) = 0;
};

struct TBlendTileInfo
{
    int blendNdx;
    char horiz;
    char vert;
    char rightDiagonal;
    char leftDiagonal;
    char inverted;
    char longDiagonal;
    int customBlendEdgeClass;
};

struct TCliffInfo
{
    float u0;
    float v0;
    float u1;
    float v1;
    float u2;
    float v2;
    float u3;
    float v3;
    int flip;
    int mutant;
    short tileIndex;
};

struct TXTextureClass
{
    int globalTextureClass;
    int firstTile;
    int numTiles;
    int width;
    int isBlendEdgeTile;
    Utf8String name;
    ICoord2D positionInTexture;
};

class WorldHeightMap : public RefCountClass, public WorldHeightMapInterfaceClass
{
public:
    virtual ~WorldHeightMap();
    virtual int Get_Border_Size();
    virtual float Get_Seismic_Z_Velocity(int x, int y);
    virtual void Set_Seismic_Z_Velocity(int x, int y, float velocity);
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y);
    int Get_Border_Size_Unk() { return m_borderSize; }

private:
    int m_width;
    int m_height;
    int m_borderSize;
    std::vector<ICoord2D> m_boundaries;
    int m_dataSize;
    char *m_data;
    unsigned char *m_siesmicUpdateFlags;
    int m_siesmicUpdateWidth;
    float *m_siesmicZVelocitie;
    char *m_cellFlipState;
    int m_flipStateWidth;
    unsigned char *m_cliffState;
    short *m_tileNdxes;
    short *m_blendTileNdxes;
    short *m_cliffInfoNdxes;
    short *m_extraBlendTileNdxes;
    int m_numBitmapTiles;
    int m_numEdgeTiles;
    int m_numBlendedTiles;
    TileData *m_sourceTiles[1024];
    TileData *m_edgeTiles[1024];
    TBlendTileInfo m_blendedTiles[16192];
    TBlendTileInfo m_extraBlendedTiles[16192];
    TCliffInfo m_cliffInfo[32384];
    int m_numCliffInfo;
    int m_numTextureClasses;
    TXTextureClass m_textureClasses[256];
    int m_numEdgeTextureClasses;
    TXTextureClass m_edgeTextureClasses[256];
    TerrainTextureClass *m_terrainTex;
    int m_terrainTexHeight;
    AlphaTerrainTextureClass *m_alphaTerrainTex;
    int m_alphaTexHeight;
    AlphaEdgeTextureClass *m_alphaEdgeTex;
    int m_alphaEdgeHeight;
    int m_drawOriginX;
    int m_drawOriginY;
    int m_drawWidthX;
    int m_drawHeightY;
};
