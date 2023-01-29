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
    m_zoneTable1(nullptr),
    m_zoneTable2(nullptr),
    m_zoneTable3(nullptr),
    m_zoneTable4(nullptr),
    m_zoneTable5(nullptr),
    m_zoneTable6(nullptr)
{
    m_blockSize.x = 0;
    m_blockSize.y = 0;
}

PathfindZoneManager::~PathfindZoneManager()
{
#ifdef GAME_DLL
    Call_Method<void, PathfindZoneManager>(PICK_ADDRESS(0x0055C320, 0x0088BF4E), this);
#endif
}

PathfindLayer::PathfindLayer() :
    m_map(nullptr), m_mapPointers(nullptr), m_layerNum(LAYER_INVALID), m_unk(0), m_bridge(nullptr), m_destroyed(false)
{
    m_extent.lo.x = 0;
    m_extent.lo.y = 0;
    m_extent.hi.x = 0;
    m_extent.hi.y = 0;
    m_unkRegion.lo.x = -1;
    m_unkRegion.lo.y = -1;
    m_unkRegion.hi.x = -1;
    m_unkRegion.hi.y = -1;
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

Pathfinder::Pathfinder() : m_mapPointers(nullptr), m_debugPath(nullptr)
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
