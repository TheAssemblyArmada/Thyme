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
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Load(Utf8String);
    virtual void Get_Terrain_Color_At(float, float, RGBColor *) = 0;
    virtual TerrainType *Get_Terrain_Tile(float, float) = 0;
    virtual bool Intersect_Terrain(Coord3D *, Coord3D *, Coord3D *);
    virtual void Enable_Water_Grid(bool) = 0;
    virtual void Set_Water_Grid_Height_Clamps(const WaterHandle *, float, float) = 0;
    virtual void Set_Water_Attenuation_Factors(const WaterHandle *, float, float, float, float) = 0;
    virtual void Set_Water_Transform(const WaterHandle *, float, float, float, float) = 0;
    virtual void Set_Water_Transform(const Matrix3D *) = 0;
    virtual void Get_Water_Transform(const WaterHandle *, Matrix3D *) = 0;
    virtual void Set_Water_Grid_Resolution(const WaterHandle *, float, float, float) = 0;
    virtual void Get_Water_Grid_Resolution(const WaterHandle *, float *, float *, float *) = 0;
    virtual void Change_Water_Height(float, float, float) = 0;
    virtual void Add_Water_Velocity(float, float, float, float) = 0;
    virtual bool Get_Water_Grid_Height(float, float, float *) = 0;
    virtual void Set_Terrain_Tracks_Detail() = 0;
    virtual void Set_Shore_Line_Detail() = 0;
    virtual void Add_Faction_Bib(Object *, bool, float) = 0;
    virtual void Remove_Faction_Bib(Object *) = 0;
    virtual void Add_Faction_Bib_Drawable(Drawable *, bool, float) = 0;
    virtual void Remove_Faction_Bib_Drawable(Drawable *) = 0;
    virtual void Remove_All_Bibs() = 0;
    virtual void Remove_Bib_Highlighting() = 0;
    virtual void Remove_Trees_And_Props_For_Construction(const Coord3D *, const GeometryInfo &, float) = 0;
    virtual void Add_Prop(const ThingTemplate *, const Coord3D *, float) = 0;
    virtual void Set_Raw_Map_Height(const ICoord2D *, int) = 0;
    virtual unsigned char Get_Raw_Map_Height(const ICoord2D *) = 0;
    virtual WorldHeightMap *Get_Logic_Height_Map();
    virtual WorldHeightMap *Get_Client_Height_Map();
    virtual void Replace_Skybox_Textures(const Utf8String **, const Utf8String **) = 0;

private:
    Utf8String m_filenameString;
};

#ifdef GAME_DLL
extern TerrainVisual *&g_theTerrainVisual;
#else
extern TerrainVisual *g_theTerrainVisual;
#endif
