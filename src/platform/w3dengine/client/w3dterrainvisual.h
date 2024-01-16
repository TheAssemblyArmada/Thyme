/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Terrain Visual
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
#include "terrainvisual.h"

class BaseHeightMapRenderObjClass;
class WaterRenderObjClass;

class W3DTerrainVisual : public TerrainVisual
{
public:
    W3DTerrainVisual();
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Load(Utf8String filename) override;
    virtual void Get_Terrain_Color_At(float x, float y, RGBColor *color) override;
    virtual TerrainType *Get_Terrain_Tile(float x, float y) override;
    virtual bool Intersect_Terrain(Coord3D *ray_start, Coord3D *ray_end, Coord3D *result) override;
    virtual void Enable_Water_Grid(bool enable) override;
    virtual void Set_Water_Grid_Height_Clamps(const WaterHandle *handle, float min_z, float max_z) override;
    virtual void Set_Water_Attenuation_Factors(
        const WaterHandle *handle, float att_0, float att_1, float att_2, float range) override;
    virtual void Set_Water_Transform(const WaterHandle *handle, float angle, float x, float y, float z) override;
    virtual void Set_Water_Transform(const Matrix3D *transform) override;
    virtual void Get_Water_Transform(const WaterHandle *handle, Matrix3D *transform) override;
    virtual void Set_Water_Grid_Resolution(
        const WaterHandle *handle, float grid_cells_x, float grid_cells_y, float cell_size) override;
    virtual void Get_Water_Grid_Resolution(
        const WaterHandle *handle, float *grid_cells_x, float *grid_cells_y, float *cell_size) override;
    virtual void Change_Water_Height(float wx, float wy, float delta) override;
    virtual void Add_Water_Velocity(float world_x, float world_y, float z_velocity, float preferred_height) override;
    virtual bool Get_Water_Grid_Height(float x, float y, float *height) override;
    virtual void Set_Terrain_Tracks_Detail() override;
    virtual void Set_Shore_Line_Detail() override;
    virtual void Add_Faction_Bib(Object *object, bool highlighted, float width) override;
    virtual void Remove_Faction_Bib(Object *object) override;
    virtual void Add_Faction_Bib_Drawable(Drawable *drawable, bool highlighted, float width) override;
    virtual void Remove_Faction_Bib_Drawable(Drawable *drawable) override;
    virtual void Remove_All_Bibs() override;
    virtual void Remove_Bib_Highlighting() override;
    virtual void Remove_Trees_And_Props_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle) override;
    virtual void Add_Prop(const ThingTemplate *tmplate, const Coord3D *pos, float angle) override;
    virtual void Set_Raw_Map_Height(const ICoord2D *pos, int height) override;
    virtual int Get_Raw_Map_Height(const ICoord2D *pos) override;
    virtual WorldHeightMap *Get_Logic_Height_Map() override;
    virtual WorldHeightMap *Get_Client_Height_Map() override;
    virtual void Replace_Skybox_Textures(const Utf8String **oldnames, const Utf8String **newnames) override;

    virtual ~W3DTerrainVisual() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

private:
    BaseHeightMapRenderObjClass *m_baseHeightMap;
    WaterRenderObjClass *m_waterRenderObj;
    WorldHeightMap *m_heightMap;
    bool m_waterGridEnabled;
    Utf8String m_currentSkyboxTextures[5];
    Utf8String m_initialSkyboxTextures[5];
};
