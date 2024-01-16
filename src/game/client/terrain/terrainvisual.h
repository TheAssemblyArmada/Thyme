/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Terrain Visual
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
#include "snapshot.h"
#include "subsysteminterface.h"

class RGBColor;
class TerrainType;
class Coord3D;
class WaterHandle;
class Matrix3D;
class Object;
class Drawable;
class GeometryInfo;
class ThingTemplate;
class ICoord2D;
class WorldHeightMap;

class TerrainVisual : public SnapShot, public SubsystemInterface
{
public:
    TerrainVisual();
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Load(Utf8String filename);
    virtual void Get_Terrain_Color_At(float x, float y, RGBColor *color) = 0;
    virtual TerrainType *Get_Terrain_Tile(float x, float y) = 0;
    virtual bool Intersect_Terrain(Coord3D *ray_start, Coord3D *ray_end, Coord3D *result);
    virtual void Enable_Water_Grid(bool enable) = 0;
    virtual void Set_Water_Grid_Height_Clamps(const WaterHandle *handle, float min_z, float max_z) = 0;
    virtual void Set_Water_Attenuation_Factors(
        const WaterHandle *handle, float att_0, float att_1, float att_2, float range) = 0;
    virtual void Set_Water_Transform(const WaterHandle *handle, float angle, float x, float y, float z) = 0;
    virtual void Set_Water_Transform(const Matrix3D *transform) = 0;
    virtual void Get_Water_Transform(const WaterHandle *handle, Matrix3D *transform) = 0;
    virtual void Set_Water_Grid_Resolution(
        const WaterHandle *handle, float grid_cells_x, float grid_cells_y, float cell_size) = 0;
    virtual void Get_Water_Grid_Resolution(
        const WaterHandle *handle, float *grid_cells_x, float *grid_cells_y, float *cell_size) = 0;
    virtual void Change_Water_Height(float wx, float wy, float delta) = 0;
    virtual void Add_Water_Velocity(float world_x, float world_y, float z_velocity, float preferred_height) = 0;
    virtual bool Get_Water_Grid_Height(float x, float y, float *height) = 0;
    virtual void Set_Terrain_Tracks_Detail() = 0;
    virtual void Set_Shore_Line_Detail() = 0;
    virtual void Add_Faction_Bib(Object *object, bool highlighted, float width) = 0;
    virtual void Remove_Faction_Bib(Object *object) = 0;
    virtual void Add_Faction_Bib_Drawable(Drawable *drawable, bool highlighted, float width) = 0;
    virtual void Remove_Faction_Bib_Drawable(Drawable *drawable) = 0;
    virtual void Remove_All_Bibs() = 0;
    virtual void Remove_Bib_Highlighting() = 0;
    virtual void Remove_Trees_And_Props_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle) = 0;
    virtual void Add_Prop(const ThingTemplate *tmplate, const Coord3D *pos, float angle) = 0;
    virtual void Set_Raw_Map_Height(const ICoord2D *pos, int height) = 0;
    virtual int Get_Raw_Map_Height(const ICoord2D *) = 0;
    virtual WorldHeightMap *Get_Logic_Height_Map();
    virtual WorldHeightMap *Get_Client_Height_Map();
    virtual void Replace_Skybox_Textures(const Utf8String **oldnames, const Utf8String **newnames) = 0;

    virtual ~TerrainVisual() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

private:
    Utf8String m_filenameString;
};

#ifdef GAME_DLL
extern TerrainVisual *&g_theTerrainVisual;
#else
extern TerrainVisual *g_theTerrainVisual;
#endif
