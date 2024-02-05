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
#include "aipathfind.h"
#include "object.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "xfer.h"

PathfindZoneManager::PathfindZoneManager() :
    m_zoneBlocks(nullptr),
    m_zoneBlockPointers(nullptr),
    m_maxZone(0),
    m_updateFrequency(0),
    m_zoneTableSize(0),
    m_zoneTableCliff(nullptr),
    m_zoneTableWater(nullptr),
    m_zoneTableRubble(nullptr),
    m_zoneTableObstacle(nullptr),
    m_zoneTableUnk(nullptr),
    m_zoneTableUnk2(nullptr)
{
    m_zoneBlockExtent.x = 0;
    m_zoneBlockExtent.y = 0;
}

PathfindZoneManager::~PathfindZoneManager()
{
#ifdef GAME_DLL
    Call_Method<void, PathfindZoneManager>(PICK_ADDRESS(0x0055C320, 0x0088BF4E), this);
#endif
}

PathfindLayer::PathfindLayer() :
    m_cells(nullptr),
    m_layerCells(nullptr),
    m_width(0),
    m_height(0),
    m_xOrigin(0),
    m_yOrigin(0),
    m_zone(0),
    m_bridge(nullptr),
    m_destroyed(false)
{
    m_unk1.x = -1;
    m_unk1.y = -1;
    m_unk2.x = -1;
    m_unk2.y = -1;
}

PathfindLayer::~PathfindLayer()
{
    Reset();
}

void PathfindLayer::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, PathfindLayer>(PICK_ADDRESS(0x0055DCA0, 0x0088E85F), this);
#endif
}

#ifndef GAME_DLL
PathfindCellInfo *PathfindCellInfo::m_infoArray;
PathfindCellInfo *PathfindCellInfo::m_firstFree;
#endif

void PathfindCellInfo::Allocate_Cell_Infos()
{
    Release_Cell_Infos();
    m_infoArray = new PathfindCellInfo[MAX_CELL_INFOS];
    m_infoArray[MAX_CELL_INFOS - 1].m_pathParent = 0;
    m_infoArray[MAX_CELL_INFOS - 1].m_isFree = true;
    m_firstFree = m_infoArray;

    for (int i = 0; i < MAX_CELL_INFOS - 1; i++) {
        m_infoArray[i].m_pathParent = &m_infoArray[i + 1];
        m_infoArray[i].m_isFree = true;
    }
}

void PathfindCellInfo::Release_Cell_Infos()
{
    if (m_infoArray != nullptr) {
        int count = 0;

        while (m_firstFree != nullptr) {
            count++;
            captainslog_dbgassert(m_firstFree->m_isFree, "Should be freed.");
            m_firstFree = m_firstFree->m_pathParent;
        }

        captainslog_dbgassert(count == MAX_CELL_INFOS, "Error - Allocated cellinfos.");
        delete[] m_infoArray;
        m_infoArray = nullptr;
        m_firstFree = nullptr;
    }
}

Pathfinder::Pathfinder() : m_map(nullptr), m_debugPath(nullptr)
{
    PathfindCellInfo::Allocate_Cell_Infos();
    Reset();
}

Pathfinder::~Pathfinder()
{
    PathfindCellInfo::Release_Cell_Infos();
}

Path *Pathfinder::Find_Path(Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to)
{
#ifdef GAME_DLL
    return Call_Method<Path *, Pathfinder, Object *, const LocomotorSet &, const Coord3D *, const Coord3D *>(
        PICK_ADDRESS(0x00565090, 0x00895FB4), this, obj, locomotor_set, from, raw_to);
#else
    return nullptr;
#endif
}

Path *Pathfinder::Find_Closest_Path(Object *obj,
    const LocomotorSet &locomotor_set,
    const Coord3D *from,
    Coord3D *raw_to,
    bool blocked,
    float path_cost_multiplier,
    bool move_allies)
{
#ifdef GAME_DLL
    return Call_Method<Path *, Pathfinder, Object *, const LocomotorSet &, const Coord3D *, Coord3D *, bool, float, bool>(
        PICK_ADDRESS(0x0056A3D0, 0x0089B634),
        this,
        obj,
        locomotor_set,
        from,
        raw_to,
        blocked,
        path_cost_multiplier,
        move_allies);
#else
    return nullptr;
#endif
}

Path *Pathfinder::Find_Attack_Path(const Object *obj,
    const LocomotorSet &locomotor_set,
    const Coord3D *from,
    const Object *victim,
    const Coord3D *victim_pos,
    const Weapon *weapon)
{
#ifdef GAME_DLL
    return Call_Method<Path *,
        Pathfinder,
        const Object *,
        const LocomotorSet &,
        const Coord3D *,
        const Object *,
        const Coord3D *,
        const Weapon *>(PICK_ADDRESS(0x0056F370, 0x0089F8DC), this, obj, locomotor_set, from, victim, victim_pos, weapon);
#else
    return nullptr;
#endif
}

Path *Pathfinder::Patch_Path(const Object *obj, const LocomotorSet &locomotor_set, Path *path, bool blocked)
{
#ifdef GAME_DLL
    return Call_Method<Path *, Pathfinder, const Object *, const LocomotorSet &, Path *, bool>(
        PICK_ADDRESS(0x0056E9C0, 0x0089F097), this, obj, locomotor_set, path, blocked);
#else
    return nullptr;
#endif
}

Path *Pathfinder::Find_Safe_Path(const Object *obj,
    const LocomotorSet &locomotor_set,
    const Coord3D *pos1,
    const Coord3D *pos2,
    const Coord3D *pos3,
    float f)
{
#ifdef GAME_DLL
    return Call_Method<Path *,
        Pathfinder,
        const Object *,
        const LocomotorSet &,
        const Coord3D *,
        const Coord3D *,
        const Coord3D *,
        float>(PICK_ADDRESS(0x00570400, 0x008A064A), this, obj, locomotor_set, pos1, pos2, pos3, f);
#else
    return nullptr;
#endif
}

Path *Pathfinder::Internal_Find_Path(
    Object *obj, const LocomotorSet &locomotor_set, const Coord3D *from, const Coord3D *raw_to)
{
#ifdef GAME_DLL
    return Call_Method<Path *, Pathfinder, Object *, const LocomotorSet &, const Coord3D *, const Coord3D *>(
        PICK_ADDRESS(0x005651B0, 0x00896085), this, obj, locomotor_set, from, raw_to);
#else
    return nullptr;
#endif
}

void Pathfinder::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00570950, 0x008A09F0), this, xfer);
#endif
}

void Pathfinder::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

void Pathfinder::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder>(PICK_ADDRESS(0x0055F250, 0x008904EA), this);
#endif
}

void Pathfinder::New_Map()
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder>(PICK_ADDRESS(0x00560970, 0x00891DC6), this);
#endif
}

bool Pathfinder::Valid_Movement_Terrain(PathfindLayerEnum layer, const Locomotor *locomotor, const Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, PathfindLayerEnum, const Locomotor *, const Coord3D *>(
        PICK_ADDRESS(0x00561100, 0x00892877), this, layer, locomotor, pos);
#else
    return false;
#endif
}

bool Pathfinder::Adjust_Target_Destination(const Object *source_obj,
    const Object *target_obj,
    const Coord3D *target_pos,
    const Weapon *weapon,
    Coord3D *destination_pos)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, const Object *, const Object *, const Coord3D *, const Weapon *, Coord3D *>(
        PICK_ADDRESS(0x00562EB0, 0x00893CA1), this, source_obj, target_obj, target_pos, weapon, destination_pos);
#else
    return false;
#endif
}

void Pathfinder::Classify_Object_Footprint(Object *obj, bool insert)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *, bool>(PICK_ADDRESS(0x0055FA70, 0x00890D45), this, obj, insert);
#endif
}

void Pathfinder::Update_Pos(Object *obj, const Coord3D *pos)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *, const Coord3D *>(PICK_ADDRESS(0x0056D510, 0x0089DFE3), this, obj, pos);
#endif
}

void Pathfinder::Remove_Pos(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *>(PICK_ADDRESS(0x0056DA30, 0x0089E389), this, obj);
#endif
}

void Pathfinder::Change_Bridge_State(PathfindLayerEnum layer, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, PathfindLayerEnum, bool>(PICK_ADDRESS(0x0056C970, 0x0089D713), this, layer, b);
#endif
}

PathfindLayerEnum Pathfinder::Add_Bridge(Bridge *bridge)
{
#ifdef GAME_DLL
    return Call_Method<PathfindLayerEnum, Pathfinder, Bridge *>(PICK_ADDRESS(0x0055F580, 0x008908BE), this, bridge);
#else
    return LAYER_INVALID;
#endif
}

bool Pathfinder::Is_Point_On_Wall(const Coord3D *point)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, const Coord3D *>(PICK_ADDRESS(0x0055F450, 0x00890845), this, point);
#else
    return false;
#endif
}

void Pathfinder::Force_Map_Recalculation()
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder>(PICK_ADDRESS(0x00560E90, 0x00892519), this);
#endif
}

bool Pathfinder::Adjust_Destination(Object *obj, const LocomotorSet &locomotor_set, Coord3D *dest, const Coord3D *unk)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, Object *, const LocomotorSet &, Coord3D *, const Coord3D *>(
        PICK_ADDRESS(0x00562AF0, 0x008938DC), this, obj, locomotor_set, dest, unk);
#else
    return false;
#endif
}

void Pathfinder::Update_Goal(Object *obj, const Coord3D *new_goal_pos, PathfindLayerEnum layer)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *, const Coord3D *, PathfindLayerEnum>(
        PICK_ADDRESS(0x0056CAD0, 0x0089D85D), this, obj, new_goal_pos, layer);
#endif
}

void Pathfinder::Remove_Goal(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *>(PICK_ADDRESS(0x0056D280, 0x0089DDE6), this, obj);
#endif
}

bool Pathfinder::Is_Line_Passable(const Object *obj,
    int i,
    PathfindLayerEnum layer,
    const Coord3D *start_world,
    const Coord3D *end_world,
    bool b1,
    bool b2)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        Pathfinder,
        const Object *,
        int,
        PathfindLayerEnum,
        const Coord3D *,
        const Coord3D *,
        bool,
        bool>(PICK_ADDRESS(0x0056C6E0, 0x0089D66B), this, obj, i, layer, start_world, end_world, b1, b2);
#else
    return false;
#endif
}

Path *Pathfinder::Find_Ground_Path(const Coord3D *point, const Coord3D *point2, int i, bool b)
{
#ifdef GAME_DLL
    return Call_Method<Path *, Pathfinder, const Coord3D *, const Coord3D *, int, bool>(
        PICK_ADDRESS(0x00566980, 0x008974D1), this, point, point2, i, b);
#else
    return nullptr;
#endif
}

void Pathfinder::Add_Wall_Piece(Object *obj)
{
    if (m_numWallPieces < 127) {
        m_wallPieces[m_numWallPieces++] = obj->Get_ID();
    }
}

void Pathfinder::Remove_Wall_Piece(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Object *>(PICK_ADDRESS(0x0055F400, 0x008907BC), this, obj);
#endif
}

PathNode *PathNode::Get_Next_Optimized(Coord2D *pos, float *length)
{
    if (pos != nullptr) {
        *pos = m_optimizedPos;
    }

    if (length != nullptr) {
        *length = m_optimizedLength;
    }

    return m_nextOpti;
}

void Pathfinder::Set_Debug_Path(Path *new_debug_path)
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder, Path *>(PICK_ADDRESS(0x0056B600, 0x0089C822), this, new_debug_path);
#endif
}

void Pathfinder::Process_Pathfind_Queue()
{
#ifdef GAME_DLL
    Call_Method<void, Pathfinder>(PICK_ADDRESS(0x00563F90, 0x00894D94), this);
#endif
}
