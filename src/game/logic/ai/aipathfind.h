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

class Bridge;
class Locomotor;
class LocomotorSet;
class Object;
class PathfindCell;
class Weapon;

class ZoneBlock
{
private:
    ICoord2D m_pos; // not 100% confirmed
    unsigned short m_firstZone; // confirmed
    unsigned short m_maxZone; // confirmed
    unsigned short m_zoneInfoSize; // confirmed
    unsigned short *m_pathfindZoneInfoCliff; // not 100% confirmed
    unsigned short *m_pathfindZoneInfoWater; // not 100% confirmed
    unsigned short *m_pathfindZoneInfoRubble; // not 100% confirmed
    unsigned short *m_pathfindZoneInfoUnk; // not 100% confirmed
    bool m_bridge; // not 100% confirmed
    bool m_passable; // not 100% confirmed
};

class PathfindZoneManager
{
public:
    PathfindZoneManager();
    ~PathfindZoneManager();

private:
    ZoneBlock *m_zoneBlocks; // not 100% confirmed
    ZoneBlock **m_zoneBlockPointers; // not 100% confirmed
    ICoord2D m_zoneBlockExtent; // confirmed
    unsigned short m_maxZone; // confirmed
    unsigned int m_updateFrequency; // not 100% confirmed
    unsigned short m_zoneTableSize; // not 100% confirmed
    unsigned short *m_zoneTableCliff; // confirmed
    unsigned short *m_zoneTableWater; // confirmed
    unsigned short *m_zoneTableRubble; // confirmed
    unsigned short *m_zoneTableObstacle; // confirmed
    unsigned short *m_zoneTableUnk; // not 100% confirmed
    unsigned short *m_zoneTableUnk2; // not 100% confirmed
};

// confirmed
struct ClosestPointOnPathInfo
{
    float m_distance;
    Coord3D m_pos;
    PathfindLayerEnum m_layer;
};

class PathNode : public MemoryPoolObject
{
    IMPLEMENT_NAMED_POOL(PathNode, PathNodePool)

public:
    virtual ~PathNode() override;

    const Coord3D *Get_Position() const { return &m_pos; }

    PathNode *Get_Next_Optimized(Coord2D *pos, float *length);

private:
    int m_optimizedLink; // not 100% confirmed
    PathNode *m_nextOpti; // confirmed
    PathNode *m_next; // confirmed
    PathNode *m_prev; // confirmed
    Coord3D m_pos; // confirmed
    PathfindLayerEnum m_layer; // not 100% confirmed
    bool m_unkBool; // not 100% confirmed
    float m_optimizedLength; // not 100% confirmed
    Coord2D m_optimizedPos; // not 100% confirmed
};

class Path : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(Path, PathPool);

protected:
    virtual ~Path() override;

public:
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Get_Point_Pos(Coord3D *pos) const { *pos = m_closestPoint.m_pos; }
    PathNode *Get_First_Node() const { return m_path; }
    PathNode *Get_Last_Node() const { return m_pathTail; }

private:
    PathNode *m_path; // confirmed
    PathNode *m_pathTail; // confirmed
    bool m_isOptimized; // confirmed
    bool m_blockedByAlly; // confirmed
    bool m_unk2; // not 100% confirmed
    int m_unk3; // not 100% confirmed
    Coord3D m_unk4; // not 100% confirmed
    ClosestPointOnPathInfo m_closestPoint; // not 100% confirmed
    PathNode *m_unkNode; // not 100% confirmed
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
        MAX_CELL_INFOS = 30000,
    };

    PathfindCellInfo *m_next; // confirmed
    PathfindCellInfo *m_prev; // confirmed
    PathfindCellInfo *m_pathParent; // confirmed
    PathfindCell *m_cell; // confirmed
    unsigned short m_totalCost; // confirmed
    unsigned short m_costSoFar; // confirmed
    ICoord2D m_pos; // confirmed
    ObjectID m_goalUnitID; // confirmed
    ObjectID m_posUnitID; // confirmed
    ObjectID m_goalAircraftID; // confirmed
    ObjectID m_obstacleID; // confirmed
    bool m_isFree : 1; // confirmed
    bool m_blockedByAlly : 1; // confirmed
    bool m_unk2 : 1; // not 100% confirmed
    bool m_unk3 : 1; // not 100% confirmed
    bool m_open : 1; // confirmed
    bool m_closed : 1; // confirmed

#ifdef GAME_DLL
    static PathfindCellInfo *&m_infoArray;
    static PathfindCellInfo *&m_firstFree;
#else
    static PathfindCellInfo *m_infoArray;
    static PathfindCellInfo *m_firstFree;
#endif
};

class PathfindCell
{
    enum CellType
    {
        CELL_CLEAR, // confirmed
        CELL_WATER, // confirmed
        CELL_CLIFF, // confirmed
        CELL_RUBBLE, // confirmed
        CELL_OBSTACLE, // confirmed
        CELL_TYPE_5,
        CELL_TYPE_6,
    };

    enum CellFlags
    {
        NO_UNITS = 0,
        UNIT_GOAL = 1,
        UNIT_PRESENT_MOVING = 2,
        UNIT_PRESENT_FIXED = 3,
        UNIT_GOAL_OTHER_MOVING = 5,
    };

private:
    PathfindCellInfo *m_info; // confirmed
    unsigned short m_zone : 14; // confirmed
    unsigned char m_aircraftGoal : 1; // confirmed
    unsigned char m_unk2 : 1; // not 100% confirmed
    unsigned char m_type : 4; // confirmed
    unsigned char m_flags : 4; // not 100% confirmed
    unsigned char m_connectLayer : 4; // not 100% confirmed
    unsigned char m_layer2 : 4; // not 100% confirmed
};

class PathfindLayer
{
public:
    PathfindLayer();
    ~PathfindLayer();
    void Reset();

private:
    PathfindCell *m_cells; // confirmed
    PathfindCell **m_layerCells; // confirmed
    int m_width; // confirmed
    int m_height; // confirmed
    int m_xOrigin; // confirmed
    int m_yOrigin; // confirmed
    ICoord2D m_unk1; // not 100% confirmed, BFME2 shows its two ICoord2D
    ICoord2D m_unk2; // not 100% confirmed
    PathfindLayerEnum m_layer; // confirmed
    int m_zone; // confirmed
    Bridge *m_bridge; // confirmed
    bool m_destroyed; // confirmed
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
    bool Adjust_Destination(Object *obj, const LocomotorSet &locomotor_set, Coord3D *dest, const Coord3D *unk);
    void Update_Goal(Object *obj, const Coord3D *new_goal_pos, PathfindLayerEnum layer);
    void Remove_Goal(Object *obj);
    bool Is_Line_Passable(const Object *obj,
        int i,
        PathfindLayerEnum layer,
        const Coord3D *start_world,
        const Coord3D *end_world,
        bool b1,
        bool b2);

    void Classify_Object_Footprint(Object *obj, bool insert);
    void Update_Pos(Object *obj, const Coord3D *pos);
    void Remove_Pos(Object *obj);
    void Change_Bridge_State(PathfindLayerEnum layer, bool b);
    PathfindLayerEnum Add_Bridge(Bridge *bridge);
    void Add_Wall_Piece(Object *obj);
    void Remove_Wall_Piece(Object *obj);
    bool Is_Point_On_Wall(const Coord3D *point);
    void Force_Map_Recalculation();
    Path *Find_Ground_Path(const Coord3D *point, const Coord3D *point2, int i, bool b);
    void New_Map();
    void Set_Debug_Path(Path *new_debug_path);
    void Process_Pathfind_Queue();

    void Remove_Object_From_Pathfind_Map(Object *obj) { Classify_Object_Footprint(obj, false); }
    void Add_Object_To_Pathfind_Map(Object *obj) { Classify_Object_Footprint(obj, true); }
    float Get_Wall_Height() const { return m_wallHeight; }

private:
    PathfindCell *m_mapPointer; // not 100% confirmed
    PathfindCell **m_map; // confirmed
    IRegion2D m_extent; // confirmed
    IRegion2D m_logicalExtent; // confirmed
    PathfindCell *m_openList; // confirmed
    PathfindCell *m_closedList; // confirmed
    bool m_isMapReady; // confirmed
    bool m_isTunneling; // confirmed
    int m_frameToShowObstacles; // confirmed
    Coord3D m_debugPathPos; // confirmed
    Path *m_debugPath; // confirmed
    ObjectID m_ignoreObstacleID; // confirmed
    PathfindZoneManager m_zoneManager; // confirmed
    PathfindLayer m_layers[LAYER_COUNT]; // confirmed
    ObjectID m_wallPieces[128]; // confirmed
    int m_numWallPieces; // confirmed
    float m_wallHeight; // confirmed
    int m_unk; // not 100% confirmed
    ObjectID m_queuedPathfindRequests[512]; //  confirmed
    int m_queuePRHead; // confirmed
    int m_queuePRTail; // confirmed
    int m_cumulativeCellsAllocated; // confirmed
};
