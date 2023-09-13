/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Terrain Logic
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
#include "terrainlogic.h"

class W3DTerrainLogic : public TerrainLogic
{
public:
    W3DTerrainLogic();
    virtual ~W3DTerrainLogic() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Load_Map(Utf8String filename, bool query) override;
    virtual void New_Map(bool b) override;
    virtual float Get_Ground_Height(float x, float y, Coord3D *n) const override;
    virtual float Get_Layer_Height(float x, float y, PathfindLayerEnum layer, Coord3D *n, bool b) const override;
    virtual void Get_Extent(Region3D *extent) const override;
    virtual void Get_Extent_Including_Border(Region3D *extent) const override;
    virtual void Get_Maximum_Pathfind_Extent(Region3D *extent) const override;
    virtual bool Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const override;
    virtual bool Is_Cliff_Cell(float x, float y) const override;

private:
    float m_mapMinZ;
    float m_mapMaxZ;
};
