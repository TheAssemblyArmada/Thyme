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

class Locomotor;
class PathNode;
class Object;
class Weapon;
class ZoneBlock;
class LocomotorSet;
class PathfindCell;
class Bridge;

class PathfindZoneManager
{
public:
    PathfindZoneManager();
    ~PathfindZoneManager();

private:
    ZoneBlock *m_zoneBlocks; // not 100% confirmed
    ZoneBlock **m_zoneBlockPointers; // not 100% confirmed
    ICoord2D m_blockSize; // not 100% confirmed
    unsigned short m_maxZone; // not 100% confirmed
    unsigned int m_updateFrequency; // not 100% confirmed
    unsigned short m_zoneTableSize; // not 100% confirmed
    unsigned short *m_zoneTable1; // not 100% confirmed
    unsigned short *m_zoneTable2; // not 100% confirmed
    unsigned short *m_zoneTable3; // not 100% confirmed
    unsigned short *m_zoneTable4; // not 100% confirmed
    unsigned short *m_zoneTable5; // not 100% confirmed
    unsigned short *m_zoneTable6; // not 100% confirmed
};

struct ClosestPointOnPathInfo
{
    float m_distance;
    Coord3D m_pos;
    PathfindLayerEnum m_layer;
};

class Path : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Path);

protected:
    virtual ~Path() override;

public:
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Get_Point_Pos(Coord3D *pos) const { *pos = m_closestPoint.m_pos; }

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

class PathfindLayer
{
public:
    PathfindLayer();
    ~PathfindLayer();
    void Reset();

private:
    PathfindCell *m_map; // not 100% confirmed
    PathfindCell **m_mapPointers; // not 100% confirmed
    IRegion2D m_extent; // not 100% confirmed
    IRegion2D m_unkRegion; // not 100% confirmed
    PathfindLayerEnum m_layerNum; // not 100% confirmed
    int m_unk; // not 100% confirmed
    Bridge *m_bridge; // not 100% confirmed
    bool m_destroyed; // not 100% confirmed
};

class PathfindServicesInterface
{
public:
    virtual Path *Find_Path(Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to) = 0;
    virtual Path *Find_Closest_Path(Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        Coord3D *raw_to,
        bool blocked,
        float path_cost_multiplier,
        bool move_allies) = 0;
    virtual Path *Find_Attack_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        const Object *victim,
        const Coord3D *victim_pos,
        const Weapon *weapon) = 0;
    virtual Path *Patch_Path(const Object *obj, const LocomotorSet &locomotor_set, Path *path, bool blocked) = 0;
    virtual Path *Find_Safe_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *pos1,
        const Coord3D *pos2,
        const Coord3D *pos3,
        float f) = 0;
};

class PathfindCellInfo
{
public:
    static void Allocate_Cell_Infos();
    static void Release_Cell_Infos();

private:
    enum
    {
        MAX_CELL_INFOS
    };

    PathfindCellInfo *m_nextOpen; // not 100% confirmed
    PathfindCellInfo *m_nextClosed; // not 100% confirmed
    PathfindCellInfo *m_pathParent; // not 100% confirmed
    PathfindCell *m_cell; // not 100% confirmed
    unsigned short m_costRemaining; // not 100% confirmed
    unsigned short m_costSoFar; // not 100% confirmed
    ICoord2D m_pos; // not 100% confirmed
    ObjectID m_goalUnitID; // not 100% confirmed
    ObjectID m_posUnitID; // not 100% confirmed
    ObjectID m_goalAircraftID; // not 100% confirmed
    ObjectID m_obstacleID; // not 100% confirmed
    bool m_isFree : 1; // not 100% confirmed
    bool m_unk1 : 1; // not 100% confirmed
    bool m_unk2 : 1; // not 100% confirmed
    bool m_unk3 : 1; // not 100% confirmed
    bool m_unk4 : 1; // not 100% confirmed
    bool m_unk5 : 1; // not 100% confirmed

#ifdef GAME_DLL
    static PathfindCellInfo *&m_infoArray;
    static PathfindCellInfo *&m_firstFree;
#else
    static PathfindCellInfo *m_infoArray;
    static PathfindCellInfo *m_firstFree;
#endif
};

class Pathfinder : public PathfindServicesInterface, public SnapShot
{
public:
#ifdef GAME_DLL
    Pathfinder *Hook_Ctor() { return new (this) Pathfinder(); }
    void Hook_Dtor() { Pathfinder::~Pathfinder(); }
#endif

    Pathfinder();
    ~Pathfinder();

    virtual Path *Find_Path(
        Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to) override;
    virtual Path *Find_Closest_Path(Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        Coord3D *raw_to,
        bool blocked,
        float path_cost_multiplier,
        bool move_allies) override;
    virtual Path *Find_Attack_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *from,
        const Object *victim,
        const Coord3D *victim_pos,
        const Weapon *weapon) override;
    virtual Path *Patch_Path(const Object *obj, const LocomotorSet &locomotor_set, Path *path, bool blocked) override;
    virtual Path *Find_Safe_Path(const Object *obj,
        const LocomotorSet &locomotor_set,
        const Coord3D *pos1,
        const Coord3D *pos2,
        const Coord3D *pos3,
        float f) override;
    virtual Path *Internal_Find_Path(
        Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to);

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset();

    bool Valid_Movement_Terrain(PathfindLayerEnum layer, const Locomotor *locomotor, const Coord3D *pos);
    bool Adjust_Target_Destination(const Object *source_obj,
        const Object *target_obj,
        const Coord3D *target_pos,
        const Weapon *weapon,
        Coord3D *destination_pos);

private:
    PathfindCell *m_map; // not 100% confirmed
    PathfindCell **m_mapPointers; // not 100% confirmed
    IRegion2D m_extent; // not 100% confirmed
    IRegion2D m_unkRegion; // not 100% confirmed
    PathfindCell *m_openList; // not 100% confirmed
    PathfindCell *m_closedList; // not 100% confirmed
    bool m_isMapReady; // not 100% confirmed
    bool m_isTunneling; // not 100% confirmed
    int m_frameToShowObstacles; // not 100% confirmed
    Coord3D m_debugPathPos; // not 100% confirmed
    Path *m_debugPath; // not 100% confirmed
    ObjectID m_ignoreObstacleID; // not 100% confirmed
    PathfindZoneManager m_zoneManager; // not 100% confirmed
    PathfindLayer m_layers[LAYER_COUNT]; // not 100% confirmed
    ObjectID m_wallPieces[128]; // not 100% confirmed
    int m_wallPieceCount; // not 100% confirmed
    float m_wallHeight; // not 100% confirmed
    int m_unk; // not 100% confirmed
    ObjectID m_queuedPathfindRequests[512]; // not 100% confirmed
    int m_queuePRHead; // not 100% confirmed
    int m_queuePRTail; // not 100% confirmed
    int m_cumulativeCellsAllocated; // not 100% confirmed
};
