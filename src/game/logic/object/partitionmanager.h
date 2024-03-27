/**
 * @file PartitionManager
 *
 * @author Duncans_Pumpkin
 *
 * @brief Stores information on current generals rank and purchased "sciences".
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
#include "bitflags.h"
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <queue>

struct ShroudStatusStoreRestore;
class Object;
class GhostObject;
class SimpleObjectIterator;
class GeometryInfo;
class CellAndObjectIntersection;

enum FindPositionFlags
{
    FPF_NONE = 0,
    FPF_IGNORE_WATER = 1,
    FPF_WATER_ONLY = 2,
    FPF_IGNORE_ALL_OBJECTS = 4,
    FPF_IGNORE_ALLY_OR_NEUTRAL_UNITS = 8,
    FPF_IGNORE_ALLY_OR_NEUTRAL_STRUCTURES = 0x10,
    FPF_IGNORE_ENEMY_UNITS = 0x20,
    FPF_IGNORE_ENEMY_STRUCTURES = 0x40,
    FPF_USE_LAYER_HEIGHT = 0x80,
    FPF_CLEAR_CELLS_ONLY = 0x100,
};

struct FindPositionOptions
{
    FindPositionFlags flags;
    float min_radius;
    float max_radius;
    float start_angle;
    float unk;
    Object *ignore_object;
    Object *source_to_path_to_dest;
    Object *relationship_object;
    FindPositionOptions() :
        flags(FPF_NONE),
        min_radius(0.0f),
        max_radius(0.0f),
        start_angle(-99999.898f),
        unk(10000000000.0f),
        ignore_object(nullptr),
        source_to_path_to_dest(nullptr),
        relationship_object(nullptr)
    {
    }
};

class PartitionFilter
{
public:
    virtual bool Allow(Object *obj) = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual const char *Get_Name() = 0;
#endif
};

class PartitionFilterRelationship : public PartitionFilter
{
public:
    PartitionFilterRelationship(Object *obj, int unk) : m_object(obj), m_unk(unk) {}
    virtual bool Allow(Object *obj) override;
#ifdef GAME_DEBUG_STRUCTS
    virtual const char *Get_Name() override { return "PartitionFilterRelationship"; }
#endif
private:
    Object *m_object;
    int m_unk;
};

class PartitionFilterAlive : public PartitionFilter
{
public:
    virtual bool Allow(Object *obj) override;
#ifdef GAME_DEBUG_STRUCTS
    virtual const char *Get_Name() override { return "PartitionFilterAlive"; }
#endif
};

class PartitionFilterSameMapStatus : public PartitionFilter
{
public:
    PartitionFilterSameMapStatus(Object *obj) : m_object(obj) {}
    virtual bool Allow(Object *obj) override;
#ifdef GAME_DEBUG_STRUCTS
    virtual const char *Get_Name() override { return "PartitionFilterSameMapStatus"; }
#endif
private:
    Object *m_object;
};

class PartitionFilterAcceptByKindOf : public PartitionFilter
{
public:
    PartitionFilterAcceptByKindOf(BitFlags<KINDOF_COUNT> &must_be_set, BitFlags<KINDOF_COUNT> &must_be_clear) :
        m_mustBeSet(must_be_set), m_mustBeClear(must_be_clear)
    {
    }

    virtual bool Allow(Object *obj) override;
#ifdef GAME_DEBUG_STRUCTS
    virtual const char *Get_Name() override { return "PartitionFilterAcceptByKindOf"; }
#endif

private:
    BitFlags<KINDOF_COUNT> m_mustBeSet;
    BitFlags<KINDOF_COUNT> m_mustBeClear;
};

enum DistanceCalculationType : int32_t
{
    FROM_CENTER_2D,
    FROM_CENTER_3D,
    FROM_BOUNDINGSPHERE_2D,
    FROM_BOUNDINGSPHERE_3D,
};

enum IterOrderType : int32_t
{
    ITER_FASTEST,
    ITER_SORTED_NEAR_TO_FAR,
    ITER_SORTED_FAR_TO_NEAR,
    ITER_SORTED_CHEAP_TO_EXPENSIVE,
    ITER_SORTED_EXPENSIVE_TO_CHEAP,
};

enum ValueOrThreat : int32_t
{
    VOT_THREAT,
    VOT_VALUE,
};

enum ObjectShroudStatus : int32_t
{
    SHROUDED_INVALID,
    SHROUDED_NONE,
    SHROUDED_TRANSITION,
    SHROUDED_SEEN,
    SHROUDED_NEVERSEEN,
};

struct LookerShrouder
{
    int16_t looker;
    int16_t shrouder;
};

class PartitionCell : public SnapShot
{
public:
    ~PartitionCell();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override{};

private:
    CellAndObjectIntersection *m_firstCoilInCell;
    LookerShrouder m_shroudStatus[16];
    float m_maxDist;
    float m_minDist;
    uint32_t m_threat[16];
    uint32_t m_value[16];
    int16_t m_coiCount;
    int16_t m_cellX;
    int16_t m_cellY;
};

class PartitionData : public MemoryPoolObject
{
    IMPLEMENT_NAMED_POOL(PartitionData, PartitionDataPool);

public:
    virtual ~PartitionData() override;

    void Remove_From_Dirty_Modules(PartitionData **dirtyModules);
    void Make_Dirty(bool b);
    ObjectShroudStatus Get_Shrouded_Status(int index);
    ObjectShroudStatus Get_Previous_Shrouded_Status(int index) { return m_previousShroudedness[index]; }
    void Friend_Set_Previous_Shrouded_Status(int index, ObjectShroudStatus status);
    void Set_Ghost_Object(GhostObject *obj) { m_ghostObject = obj; }

private:
    Object *m_object;
    GhostObject *m_ghostObject;
    PartitionData *m_next;
    PartitionData *m_prev;
    PartitionData *m_nextDirty;
    PartitionData *m_prevDirty;
    int32_t m_coiArrayCount;
    int32_t m_coiInUseCount;
    CellAndObjectIntersection *m_coiArray;
    int32_t unk1;
    int32_t m_dirtyFlag;
    ObjectShroudStatus m_shroudedness[16];
    ObjectShroudStatus m_previousShroudedness[16];
    bool m_everSeen[16];
    PartitionCell *m_lastCell;
};

class SightingInfo : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(SightingInfo);

public:
    virtual ~SightingInfo() override{};
    // zh: 0x00541930 wb: 0x00824207
    SightingInfo() { Reset(); }

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override{};

    void Reset();
    // zh: 0x00541A20 wb: 0x008242C1
    bool Is_Invalid() const { return m_radius == 0.0f; }

private:
    // Data is likely public or users are friends
    Coord3D m_where;
    float m_radius;
    uint16_t m_playerIndex;
    uint32_t m_frame;
    friend class Object;
};

class PartitionManager : public SubsystemInterface, public SnapShot
{
public:
    virtual ~PartitionManager() override { Shutdown(); };
    PartitionManager();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override{};

    bool Geom_Collides_With_Geom(Coord3D const *position,
        GeometryInfo const &geometry,
        float angle,
        Coord3D const *position2,
        GeometryInfo const &geometry2,
        float angle2) const;
    int32_t Calc_Min_Radius(const ICoord2D &cur);
    void Calc_Radius_Vec();
    void Do_Shroud_Cover(float centerX, float centerY, float radius, uint16_t playerIndex);
    void Do_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex);
    Object *Get_Closest_Object(Coord3D *pos,
        float maxDist,
        DistanceCalculationType dc,
        PartitionFilter **filters,
        float *closestDist,
        Coord3D *closestDistVec);

    void Register_Object(Object *object);
    void Unregister_Object(Object *object);
    void Register_Ghost_Object(GhostObject *object);
    void Unregister_Ghost_Object(GhostObject *object);
    void Reveal_Map_For_Player(int playerIndex);
    void Reveal_Map_For_Player_Permanently(int playerIndex);
    void Undo_Reveal_Map_For_Player_Permanently(int playerIndex);
    void Shroud_Map_For_Player(int playerIndex);
    void Refresh_Shroud_For_Local_Player();
    CellShroudStatus Get_Shroud_Status_For_Player(int playerIndex, int x, int y) const;
    CellShroudStatus Get_Shroud_Status_For_Player(int playerIndex, Coord3D const *loc) const;

    Object *Get_Closest_Object(Object const *obj,
        float maxDist,
        DistanceCalculationType dc,
        PartitionFilter **filters,
        float *closestDist,
        Coord3D *closestDistVec);

    void Get_Vector_To(Object const *obj, Object const *otherObj, DistanceCalculationType dc, Coord3D &vec);
    void Get_Vector_To(Object const *obj, Coord3D const *pos, DistanceCalculationType dc, Coord3D &vec);
    float Get_Distance_Squared(Object const *obj, Object const *otherObj, DistanceCalculationType dc, Coord3D *vec);
    float Get_Distance_Squared(Object const *obj, Coord3D const *pos, DistanceCalculationType dc, Coord3D *vec);
    float Get_Goal_Distance_Squared(
        Object const *obj, Coord3D const *goalPos, Object const *otherObj, DistanceCalculationType dc, Coord3D *vec);
    float Get_Goal_Distance_Squared(
        Object const *obj, Coord3D const *goalPos, Coord3D const *otherPos, DistanceCalculationType dc, Coord3D *vec);
    float Get_Relative_Angle_2D(Object const *obj, Object const *otherObj);
    float Get_Relative_Angle_2D(Object const *obj, Coord3D const *pos);
    SimpleObjectIterator *Iterate_Objects_In_Range(
        Object const *obj, float unk, DistanceCalculationType dc, PartitionFilter **filters, IterOrderType order);
    SimpleObjectIterator *Iterate_Objects_In_Range(
        Coord3D const *pos, float unk, DistanceCalculationType dc, PartitionFilter **filters, IterOrderType order);
    SimpleObjectIterator *Iterate_Potential_Collisions(Coord3D const *pos, GeometryInfo const &geom, float angle, bool unk);
    SimpleObjectIterator *Iterate_All_Objects(PartitionFilter **filters);
    ObjectShroudStatus Get_Prop_Shroud_Status_For_Player(int id, const Coord3D *position) const;

    bool Find_Position_Around(Coord3D const *center, FindPositionOptions const *options, Coord3D *result);

    void Process_Entire_Pending_Undo_Shroud_Reveal_Queue();
    void Undo_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex);
    void Queue_Undo_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex);
    void Undo_Shroud_Cover(float centerX, float centerY, float radius, uint16_t playerIndex);
    void Do_Threat_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex);
    void Undo_Threat_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex);
    void Do_Value_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex);
    void Undo_Value_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex);
    bool Estimate_Terrain_Extremes_Along_Line(
        Coord3D const &unk1, Coord3D const &unk2, float *unk3, float *unk4, Coord2D *unk5, Coord2D *unk6);
    bool Is_Clear_Line_Of_Sight_Terrain(Object const *unk1, Coord3D const &unk2, Object const *unk3, Coord3D const &unk4);
    float Get_Ground_Or_Structure_Height(float x, float y);
    void Get_Most_Valuable_Location(int unk1, unsigned int unk2, ValueOrThreat unk3, Coord3D *loc);
    void Get_Nearest_Group_With_Value(
        int unk1, unsigned int unk2, ValueOrThreat unk3, Coord3D const *unk4, int unk5, bool unk6, Coord3D *unk7);
    void Store_Fogged_Cells(ShroudStatusStoreRestore &unk1, bool unk2) const;
    void Restore_Fogged_Cells(ShroudStatusStoreRestore const &unk1, bool unk2);

    void World_To_Cell(float x, float y, int *xx, int *yy) const;
    int32_t World_To_Cell_Dist(float dist) const;
    void Get_Cell_Center_Pos(int32_t x, int32_t y, float &xx, float &yy) const;
    float Get_Cell_Size() const { return m_cellSize; }
    int32_t Get_Cell_CountX() const;
    int32_t Get_Cell_CountY() const;
    PartitionCell *Get_Cell_At(int32_t x, int32_t y);
    const PartitionCell *Get_Cell_At(int32_t x, int32_t y) const;
    bool Get_Updated_Since_Last_Reset() const { return m_updatedSinceLastReset; }

    PartitionManager *Hook_Ctor() { return new (this) PartitionManager; }

protected:
    Object *Get_Closest_Objects(Object const *obj,
        Coord3D const *pos,
        float maxDist,
        DistanceCalculationType dc,
        PartitionFilter **filters,
        SimpleObjectIterator *iterArg,
        float *closestDistArg,
        Coord3D *closestDistVecArg);
    bool Try_Position(Coord3D const *center, float dist, float angle, FindPositionOptions const *options, Coord3D *result);
    int Iterate_Cells_Along_Line(
        Coord3D const &pos, Coord3D const &posOther, int (*proc)(PartitionCell *, void *), void *userData);
    int Iterate_Cells_Breadth_First(Coord3D const *pos, int (*proc)(PartitionCell *, void *), void *userData);
    void Remove_All_Dirty_Modules();
    void Remove_From_Dirty_Modules(PartitionData *data);
    void Shutdown();
    void Reset_Pending_Undo_Shroud_Reveal_Queue();

private:
    PartitionData *m_moduleList;
    Region3D m_worldExtents;
    float m_cellSize;
    float m_cellSizeInv;
    int32_t m_cellCountX;
    int32_t m_cellCountY;
    int32_t m_totalCellCount;
    PartitionCell *m_cells;
    PartitionData *m_dirtyModules;
    bool m_updatedSinceLastReset;
    std::queue<SightingInfo *> m_sightingInfos;
    int32_t m_maxGcoRadius;
    std::vector<std::vector<ICoord2D>> m_radiusVec;
};

#ifdef GAME_DLL
extern PartitionManager *&g_thePartitionManager;
#else
extern PartitionManager *g_thePartitionManager;
#endif
