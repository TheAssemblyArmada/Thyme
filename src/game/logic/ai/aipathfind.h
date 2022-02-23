/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Pathfinding
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
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"

class PathNode;

struct ClosestPointOnPathInfo
{
    float m_distance;
    Coord3D m_pos;
    PathfindLayerEnum m_layer;
};

class Path : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Path);

public:
    virtual ~Path() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Get_Point_Pos(Coord3D *pos) { *pos = m_closestPoint.m_pos; }

private:
    PathNode *m_path;
    PathNode *m_pathTail;
    bool m_isOptimized;
    bool m_unk1;
    bool m_unk2;
    int m_unk3;
    Coord3D m_unk4;
    ClosestPointOnPathInfo m_closestPoint;
    PathNode *m_unkNode;
};
