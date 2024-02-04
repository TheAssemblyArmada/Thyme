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
#include "partitionmanager.h"
#include "object.h"
#include "simpleobjectiterator.h"
#ifdef GAME_DLL
#include "hooker.h"
#else
PartitionManager *g_thePartitionManager = nullptr;
#endif

// zh: 0x0053B550 wb: 0x0081DE80
PartitionManager::PartitionManager() :
    m_moduleList(nullptr),
    m_cellSize(0.0f),
    m_cellSizeInv(0.0f),
    m_cellCountX(0),
    m_cellCountY(0),
    m_totalCellCount(0),
    m_cells(nullptr),
    m_dirtyModules(nullptr),
    m_updatedSinceLastReset(false),
    m_maxGcoRadius(0)
{
    m_worldExtents.hi.Zero();
    m_worldExtents.lo.Zero();
}

// zh: 0x0053B930 wb: 0x0081E045
void PartitionManager::Init()
{
#ifdef GAME_DLL
    Call_Method<void, SubsystemInterface>(PICK_ADDRESS(0x0053B930, 0x0081E045), this);
#endif
}

// zh: 0x0053BC60 wb: 0x0081E4CE
void PartitionManager::Reset()
{
    // Original WB had profile reset code

    Reset_Pending_Undo_Shroud_Reveal_Queue();
    Shutdown();
}

// zh: 0x0053BDD0 wb: 0x0081E5E1
void PartitionManager::Update()
{
#ifdef GAME_DLL
    Call_Method<void, SubsystemInterface>(PICK_ADDRESS(0x0053BDD0, 0x0081E5E1), this);
#endif
}

// zh: 0x0053FCB0 wb: 0x00821E60
void PartitionManager::CRC_Snapshot(Xfer *xfer)
{
    for (int32_t i = 0; i < m_totalCellCount; ++i) {
        m_cells[i].CRC_Snapshot(xfer);
    }
}

// zh: 0x0053FCF0 wb: 0x00821EB9
void PartitionManager::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x0053FCF0, 0x00821EB9), this, xfer);
#endif
}

// wb: 0x0081E52F
void PartitionManager::Shutdown()
{
    m_updatedSinceLastReset = false;

    g_thePartitionManager->Remove_All_Dirty_Modules();

    m_radiusVec.clear();

    Reset_Pending_Undo_Shroud_Reveal_Queue();

    delete[] m_cells;
    m_cells = nullptr;

    m_cellSize = 0.0f;
    m_cellSizeInv = 0.0f;
    m_cellCountX = 0;
    m_cellCountY = 0;
    m_totalCellCount = 0;
    m_worldExtents.hi.Zero();
    m_worldExtents.lo.Zero();
}

// wb: 0x00820C6D
void PartitionManager::Reset_Pending_Undo_Shroud_Reveal_Queue()
{
    while (!m_sightingInfos.empty()) {
        m_sightingInfos.front()->Delete_Instance();
        m_sightingInfos.pop();
    }
}

// wb: 0x008255E0
void PartitionManager::Remove_All_Dirty_Modules()
{
    while (m_dirtyModules != nullptr) {
        Remove_From_Dirty_Modules(m_dirtyModules);
    }
}

// wb: 0x00824D6A
void PartitionManager::Remove_From_Dirty_Modules(PartitionData *data)
{
    data->Remove_From_Dirty_Modules(&m_dirtyModules);
}

// zh: 0x0053A820 wb: 0x0081CE95
bool PartitionManager::Geom_Collides_With_Geom(Coord3D const *position,
    GeometryInfo const &geometry,
    float angle,
    Coord3D const *position2,
    GeometryInfo const &geometry2,
    float angle2) const
{
#ifdef GAME_DLL
    return Call_Method<bool,
        const PartitionManager,
        Coord3D const *,
        GeometryInfo const &,
        float,
        Coord3D const *,
        GeometryInfo const &,
        float>(PICK_ADDRESS(0x0053A820, 0x0081CE95), this, position, geometry, angle, position2, geometry2, angle2);
#else
    return false;
#endif
}

// zh: 0x0053CA50 wb: 0x0081F364
int32_t PartitionManager::Calc_Min_Radius(const ICoord2D &cur)
{
#ifdef GAME_DLL
    return Call_Method<int32_t, PartitionManager, const ICoord2D &>(PICK_ADDRESS(0x0053CA50, 0x0081F364), this, cur);
#else
    return 0;
#endif
}

// zh: 0x0053CB80 wb: 0x0081F57B
void PartitionManager::Calc_Radius_Vec()
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager>(PICK_ADDRESS(0x0053CB80, 0x0081F57B), this);
#endif
}

// zh: 0x0053E430 wb: 0x00820E73
void PartitionManager::Do_Shroud_Cover(float centerX, float centerY, float radius, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint16_t>(
        PICK_ADDRESS(0x0053E430, 0x00820E73), this, centerX, centerY, radius, playerIndex);
#endif
}

// zh: 0x0053DEB0 wb: 0x00820ABA
void PartitionManager::Do_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint16_t>(
        PICK_ADDRESS(0x0053DEB0, 0x00820ABA), this, centerX, centerY, radius, playerIndex);
#endif
}

// zh: 0x0053D0C0 wb: 0x0081FC05
Object *PartitionManager::Get_Closest_Object(Coord3D *pos,
    float maxDist,
    DistanceCalculationType dc,
    PartitionFilter **filters,
    float *closestDist,
    Coord3D *closestDistVec)
{
    return Get_Closest_Objects(nullptr, pos, maxDist, dc, filters, nullptr, closestDist, closestDistVec);
}

// zh: 0x0053C050 wb: 0x0081EC24
void PartitionManager::Register_Object(Object *object)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, Object *>(PICK_ADDRESS(0x0053C050, 0x0081EC24), this, object);
#endif
}

// zh: 0x0053C0E0 wb: 0x0081ED24
void PartitionManager::Unregister_Object(Object *object)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, Object *>(PICK_ADDRESS(0x0053C0E0, 0x0081ED24), this, object);
#endif
}

// zh: 0x0053C200 wb: 0x0081EE06
void PartitionManager::Register_Ghost_Object(GhostObject *object)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, GhostObject *>(PICK_ADDRESS(0x0053C200, 0x0081EE06), this, object);
#endif
}

// zh: 0x0053C290 wb: 0x0081EEEF
void PartitionManager::Unregister_Ghost_Object(GhostObject *object)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, GhostObject *>(PICK_ADDRESS(0x0053C290, 0x0081EEEF), this, object);
#endif
}

// zh: 0x0053C320 wb: 0x0081EF6E
void PartitionManager::Reveal_Map_For_Player(int playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int>(PICK_ADDRESS(0x0053C320, 0x0081EF6E), this, playerIndex);
#endif
}

// zh: 0x0053C360 wb: 0x0081EFD0
void PartitionManager::Reveal_Map_For_Player_Permanently(int playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int>(PICK_ADDRESS(0x0053C360, 0x0081EFD0), this, playerIndex);
#endif
}

// zh: 0x0053C3A0 wb: 0x0081F018
void PartitionManager::Undo_Reveal_Map_For_Player_Permanently(int playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int>(PICK_ADDRESS(0x0053C3A0, 0x0081F018), this, playerIndex);
#endif
}

// zh: 0x0053C490 wb: 0x0081F068
void PartitionManager::Shroud_Map_For_Player(int playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int>(PICK_ADDRESS(0x0053C490, 0x0081F068), this, playerIndex);
#endif
}

// zh: 0x0053C580 wb: 0x0081F0D2
void PartitionManager::Refresh_Shroud_For_Local_Player()
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager>(PICK_ADDRESS(0x0053C580, 0x0081F0D2), this);
#endif
}

// zh: 0x0053C670 wb: 0x0081F1E2
CellShroudStatus PartitionManager::Get_Shroud_Status_For_Player(int playerIndex, int x, int y) const
{
#ifdef GAME_DLL
    return Call_Method<CellShroudStatus, const PartitionManager, int, int, int>(
        PICK_ADDRESS(0x0053C670, 0x0081F1E2), this, playerIndex, x, y);
#else
    return CellShroudStatus(0);
#endif
}

// zh: 0x0053C6E0 wb: 0x0081F232
CellShroudStatus PartitionManager::Get_Shroud_Status_For_Player(int playerIndex, Coord3D const *loc) const
{
    int32_t x = 0;
    int32_t y = 0;
    World_To_Cell(loc->x, loc->y, &x, &y);
    return Get_Shroud_Status_For_Player(playerIndex, x, y);
}

// zh: 0x0053D090 wb: 0x0081FC05
Object *PartitionManager::Get_Closest_Object(Object const *obj,
    float maxDist,
    DistanceCalculationType dc,
    PartitionFilter **filters,
    float *closestDist,
    Coord3D *closestDistVec)
{
    return Get_Closest_Objects(obj, nullptr, maxDist, dc, filters, nullptr, closestDist, closestDistVec);
}

// zh: 0x0053D0F0 wb: 0x0081FC36
void PartitionManager::Get_Vector_To(Object const *obj, Object const *otherObj, DistanceCalculationType dc, Coord3D &vec)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, Object const *, Object const *, DistanceCalculationType, Coord3D &>(
        PICK_ADDRESS(0x0053D0F0, 0x0081FC36), this, obj, otherObj, dc, vec);
#endif
}

// zh: 0x0053D130 wb: 0x0081FC81
void PartitionManager::Get_Vector_To(Object const *obj, Coord3D const *pos, DistanceCalculationType dc, Coord3D &vec)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, Object const *, Coord3D const *, DistanceCalculationType, Coord3D &>(
        PICK_ADDRESS(0x0053D130, 0x0081FC81), this, obj, pos, dc, vec);
#endif
}

// zh: 0x0053D170 wb: 0x0081FCC5
float PartitionManager::Get_Distance_Squared(
    Object const *obj, Object const *otherObj, DistanceCalculationType dc, Coord3D *vec)
{
#ifdef GAME_DLL
    return Call_Method<float, PartitionManager, Object const *, Object const *, DistanceCalculationType, Coord3D *>(
        PICK_ADDRESS(0x0053D170, 0x0081FCC5), this, obj, otherObj, dc, vec);
#else
    return 0.0f;
#endif
}

// zh: 0x0053D1D0 wb: 0x0081FD2D
float PartitionManager::Get_Distance_Squared(Object const *obj, Coord3D const *pos, DistanceCalculationType dc, Coord3D *vec)
{
#ifdef GAME_DLL
    return Call_Method<float, PartitionManager, Object const *, Coord3D const *, DistanceCalculationType, Coord3D *>(
        PICK_ADDRESS(0x0053D1D0, 0x0081FD2D), this, obj, pos, dc, vec);
#else
    return 0.0f;
#endif
}

// zh: 0x0053D230 wb: 0x0081FD8E
float PartitionManager::Get_Goal_Distance_Squared(
    Object const *obj, Coord3D const *goalPos, Object const *otherObj, DistanceCalculationType dc, Coord3D *vec)
{
#ifdef GAME_DLL
    return Call_Method<float,
        PartitionManager,
        Object const *,
        Coord3D const *,
        Object const *,
        DistanceCalculationType,
        Coord3D *>(PICK_ADDRESS(0x0053D230, 0x0081FD8E), this, obj, goalPos, otherObj, dc, vec);
#else
    return 0.0f;
#endif
}

// zh: 0x0053D290 wb: 0x0081FDF1
float PartitionManager::Get_Goal_Distance_Squared(
    Object const *obj, Coord3D const *goalPos, Coord3D const *otherPos, DistanceCalculationType dc, Coord3D *vec)
{
#ifdef GAME_DLL
    return Call_Method<float,
        PartitionManager,
        Object const *,
        Coord3D const *,
        Coord3D const *,
        DistanceCalculationType,
        Coord3D *>(PICK_ADDRESS(0x0053D290, 0x0081FDF1), this, obj, goalPos, otherPos, dc, vec);
#else
    return 0.0f;
#endif
}

// zh: 0x0053D2F0 wb: 0x0081FE4D
float PartitionManager::Get_Relative_Angle_2D(Object const *obj, Object const *otherObj)
{
    return Get_Relative_Angle_2D(obj, otherObj->Get_Position());
}

// zh: 0x0053D310 wb: 0x0081FE6F
float PartitionManager::Get_Relative_Angle_2D(Object const *obj, Coord3D const *pos)
{
#ifdef GAME_DLL
    return Call_Method<float, PartitionManager, Object const *, Coord3D const *>(
        PICK_ADDRESS(0x0053D310, 0x0081FE6F), this, obj, pos);
#else
    return 0.0f;
#endif
}

// zh: 0x0053D410 wb: 0x0081FFB5
SimpleObjectIterator *PartitionManager::Iterate_Objects_In_Range(
    Object const *obj, float unk, DistanceCalculationType dc, PartitionFilter **filters, IterOrderType order)
{
#ifdef GAME_DLL
    return Call_Method<SimpleObjectIterator *,
        PartitionManager,
        Object const *,
        float,
        DistanceCalculationType,
        PartitionFilter **,
        IterOrderType>(PICK_ADDRESS(0x0053D410, 0x0081FFB5), this, obj, unk, dc, filters, order);
#else
    return nullptr;
#endif
}

// zh: 0x0053D520 wb: 0x0082008B
SimpleObjectIterator *PartitionManager::Iterate_Objects_In_Range(
    Coord3D const *pos, float unk, DistanceCalculationType dc, PartitionFilter **filters, IterOrderType order)
{
#ifdef GAME_DLL
    return Call_Method<SimpleObjectIterator *,
        PartitionManager,
        Coord3D const *,
        float,
        DistanceCalculationType,
        PartitionFilter **,
        IterOrderType>(PICK_ADDRESS(0x0053D520, 0x0082008B), this, pos, unk, dc, filters, order);
#else
    return nullptr;
#endif
}

// zh: 0x0053D5F0 wb: 0x00820161
SimpleObjectIterator *PartitionManager::Iterate_Potential_Collisions(
    Coord3D const *pos, GeometryInfo const &geom, float angle, bool unk)
{
#ifdef GAME_DLL
    return Call_Method<SimpleObjectIterator *, PartitionManager, Coord3D const *, GeometryInfo const &, float, bool>(
        PICK_ADDRESS(0x0053D5F0, 0x00820161), this, pos, geom, angle, unk);
#else
    return nullptr;
#endif
}

// zh: 0x0053D750 wb: 0x008202D1
SimpleObjectIterator *PartitionManager::Iterate_All_Objects(PartitionFilter **filters)
{
#ifdef GAME_DLL
    return Call_Method<SimpleObjectIterator *, PartitionManager, PartitionFilter **>(
        PICK_ADDRESS(0x0053D750, 0x008202D1), this, filters);
#else
    return nullptr;
#endif
}

// zh: 0x0053DCC0 wb: 0x008208A2
bool PartitionManager::Find_Position_Around(Coord3D const *center, FindPositionOptions const *options, Coord3D *result)
{
#ifdef GAME_DLL
    return Call_Method<bool, PartitionManager, Coord3D const *, FindPositionOptions const *, Coord3D *>(
        PICK_ADDRESS(0x0053DCC0, 0x008208A2), this, center, options, result);
#else
    return false;
#endif
}

// zh: 0x0053E030 wb: 0x00820BB1
void PartitionManager::Process_Entire_Pending_Undo_Shroud_Reveal_Queue()
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager>(PICK_ADDRESS(0x0053E030, 0x00820BB1), this);
#endif
}

// zh: 0x0053E0E0 wb: 0x00820CB3
void PartitionManager::Undo_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint16_t>(
        PICK_ADDRESS(0x0053E0E0, 0x00820CB3), this, centerX, centerY, radius, playerIndex);
#endif
}

// zh: 0x0053E260 wb: 0x00820DAA
void PartitionManager::Queue_Undo_Shroud_Reveal(float centerX, float centerY, float radius, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint16_t>(
        PICK_ADDRESS(0x0053E260, 0x00820DAA), this, centerX, centerY, radius, playerIndex);
#endif
}

// zh: 0x0053E5B0 wb: 0x00820F6A
void PartitionManager::Undo_Shroud_Cover(float centerX, float centerY, float radius, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint16_t>(
        PICK_ADDRESS(0x0053E5B0, 0x00820F6A), this, centerX, centerY, radius, playerIndex);
#endif
}

// zh: 0x0053E730 wb: 0x00821061
void PartitionManager::Do_Threat_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint32_t, uint16_t>(
        PICK_ADDRESS(0x0053E730, 0x00821061), this, centerX, centerY, radius, unk, playerIndex);
#endif
}

// zh: 0x0053E8E0 wb: 0x00821191
void PartitionManager::Undo_Threat_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint32_t, uint16_t>(
        PICK_ADDRESS(0x0053E8E0, 0x00821191), this, centerX, centerY, radius, unk, playerIndex);
#endif
}

// zh: 0x0053EA90 wb: 0x008212C1
void PartitionManager::Do_Value_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint32_t, uint16_t>(
        PICK_ADDRESS(0x0053EA90, 0x008212C1), this, centerX, centerY, radius, unk, playerIndex);
#endif
}

// zh: 0x0053EC40 wb: 0x008213F1
void PartitionManager::Undo_Value_Affect(float centerX, float centerY, float radius, uint32_t unk, uint16_t playerIndex)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, float, float, float, uint32_t, uint16_t>(
        PICK_ADDRESS(0x0053EC40, 0x008213F1), this, centerX, centerY, radius, unk, playerIndex);
#endif
}

// zh: 0x0053EDF0 wb: 0x008215B1
bool PartitionManager::Estimate_Terrain_Extremes_Along_Line(
    Coord3D const &unk1, Coord3D const &unk2, float *unk3, float *unk4, Coord2D *unk5, Coord2D *unk6)
{
#ifdef GAME_DLL
    return Call_Method<bool, PartitionManager, Coord3D const &, Coord3D const &, float *, float *, Coord2D *, Coord2D *>(
        PICK_ADDRESS(0x0053EDF0, 0x008215B1), this, unk1, unk2, unk3, unk4, unk5, unk6);
#else
    return false;
#endif
}

// zh: 0x0053FBF0 wb: 0x00821DA5
bool PartitionManager::Is_Clear_Line_Of_Sight_Terrain(
    Object const *unk1, Coord3D const &unk2, Object const *unk3, Coord3D const &unk4)
{
#ifdef GAME_DLL
    return Call_Method<bool, PartitionManager, Object const *, Coord3D const &, Object const *, Coord3D const &>(
        PICK_ADDRESS(0x0053FBF0, 0x00821DA5), this, unk1, unk2, unk3, unk4);
#else
    return false;
#endif
}

// zh: 0x0053FFD0 wb: 0x008221A0
float PartitionManager::Get_Ground_Or_Structure_Height(float x, float y)
{
#ifdef GAME_DLL
    return Call_Method<float, PartitionManager, float, float>(PICK_ADDRESS(0x0053FFD0, 0x008221A0), this, x, y);
#else
    return 0.0f;
#endif
}

// zh: 0x005401E0 wb: 0x008222C7
void PartitionManager::Get_Most_Valuable_Location(int unk1, unsigned int unk2, ValueOrThreat unk3, Coord3D *loc)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int, unsigned int, ValueOrThreat, Coord3D *>(
        PICK_ADDRESS(0x005401E0, 0x008222C7), this, unk1, unk2, unk3, loc);
#endif
}

// zh: 0x00540380 wb: 0x008224F1
void PartitionManager::Get_Nearest_Group_With_Value(
    int unk1, unsigned int unk2, ValueOrThreat unk3, Coord3D const *unk4, int unk5, bool unk6, Coord3D *unk7)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, int, unsigned int, ValueOrThreat, Coord3D const *, int, bool, Coord3D *>(
        PICK_ADDRESS(0x00540380, 0x008224F1), this, unk1, unk2, unk3, unk4, unk5, unk6, unk7);
#endif
}

// zh: 0x005404C0 wb: 0x00822671
void PartitionManager::Store_Fogged_Cells(ShroudStatusStoreRestore &unk1, bool unk2) const
{
#ifdef GAME_DLL
    Call_Method<void, const PartitionManager, ShroudStatusStoreRestore &, bool>(
        PICK_ADDRESS(0x005404C0, 0x00822671), this, unk1, unk2);
#endif
}

// zh: 0x00540600 wb: 0x00822853
void PartitionManager::Restore_Fogged_Cells(ShroudStatusStoreRestore const &unk1, bool unk2)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionManager, ShroudStatusStoreRestore const &, bool>(
        PICK_ADDRESS(0x00540600, 0x00822853), this, unk1, unk2);
#endif
}

// wb: 0x00825910
int32_t PartitionManager::Get_Cell_CountX() const
{
    captainslog_dbgassert(m_cellCountX != 0, "Partition not inited!");
    return m_cellCountX;
}

// wb: 0x00825970
int32_t PartitionManager::Get_Cell_CountY() const
{
    captainslog_dbgassert(m_cellCountY != 0, "Partition not inited!");
    return m_cellCountY;
}

// wb: 0x00824E60
PartitionCell *PartitionManager::Get_Cell_At(int32_t x, int32_t y)
{
    if (x < 0 || x >= m_cellCountX) {
        return nullptr;
    }
    if (y < 0 || y >= m_cellCountY) {
        return nullptr;
    }
    return &m_cells[x + y * m_cellCountX];
}

// wb: 0x008258B0
const PartitionCell *PartitionManager::Get_Cell_At(int32_t x, int32_t y) const
{
    if (x < 0 || x >= m_cellCountX) {
        return nullptr;
    }
    if (y < 0 || y >= m_cellCountY) {
        return nullptr;
    }
    return &m_cells[x + y * m_cellCountX];
}

// wb: 0x00821521
void PartitionManager::Get_Cell_Center_Pos(int32_t x, int32_t y, float &xx, float &yy) const
{
    captainslog_dbgassert(x >= 0 && y >= 0, "Hmm, Invalid Cell!");
    const auto half_size = m_cellSize * 0.5f;

    xx = x * m_cellSize + m_worldExtents.lo.x + half_size;
    yy = y * m_cellSize + m_worldExtents.lo.y + half_size;
}

// zh: 0x0053CDA0 wb: 0x0081F822
Object *PartitionManager::Get_Closest_Objects(Object const *obj,
    Coord3D const *pos,
    float maxDist,
    DistanceCalculationType dc,
    PartitionFilter **filters,
    SimpleObjectIterator *iterArg,
    float *closestDistArg,
    Coord3D *closestDistVecArg)
{
#ifdef GAME_DLL
    return Call_Method<Object *,
        PartitionManager,
        Object const *,
        Coord3D const *,
        float,
        DistanceCalculationType,
        PartitionFilter **,
        SimpleObjectIterator *,
        float *,
        Coord3D *>(PICK_ADDRESS(0x0053CDA0, 0x0081F822),
        this,
        obj,
        pos,
        maxDist,
        dc,
        filters,
        iterArg,
        closestDistArg,
        closestDistVecArg);
#else
    return nullptr;
#endif
}

// wb: 0x008146F0
void PartitionManager::World_To_Cell(float x, float y, int *xx, int *yy) const
{
    *xx = GameMath::Fast_To_Int_Floor((x - m_worldExtents.lo.x) * m_cellSizeInv);
    *yy = GameMath::Fast_To_Int_Floor((y - m_worldExtents.lo.y) * m_cellSizeInv);
}

// wb: 0x00824EC0
int32_t PartitionManager::World_To_Cell_Dist(float dist) const
{
    return GameMath::Fast_To_Int_Ceil(dist * m_cellSizeInv);
}

// zh: 0x0053D850 wb: 0x008203D5
bool PartitionManager::Try_Position(
    Coord3D const *center, float dist, float angle, FindPositionOptions const *options, Coord3D *result)
{
#ifdef GAME_DLL
    return Call_Method<bool, PartitionManager, Coord3D const *, float, float, FindPositionOptions const *, Coord3D *>(
        PICK_ADDRESS(0x0053D850, 0x008203D5), this, center, dist, angle, options, result);
#else
    return false;
#endif
}

// zh: 0x0053EF40 wb: 0x008216D8
int PartitionManager::Iterate_Cells_Along_Line(
    Coord3D const &pos, Coord3D const &posOther, int (*proc)(PartitionCell *, void *), void *userData)
{
#ifdef GAME_DLL
    return Call_Method<int, PartitionManager, Coord3D const &, Coord3D const &, int (*)(PartitionCell *, void *), void *>(
        PICK_ADDRESS(0x0053EF40, 0x008216D8), this, pos, posOther, proc, userData);
#else
    return 0;
#endif
}

// zh: 0x0053F190 wb: 0x008218DE
int PartitionManager::Iterate_Cells_Breadth_First(Coord3D const *pos, int (*proc)(PartitionCell *, void *), void *userData)
{
#ifdef GAME_DLL
    return Call_Method<int, PartitionManager, Coord3D const *, int (*)(PartitionCell *, void *), void *>(
        PICK_ADDRESS(0x0053F190, 0x008218DE), this, pos, proc, userData);
#else
    return 0;
#endif
}

// zh: 0x0053B840 wb: 0x00824D80
void PartitionData::Remove_From_Dirty_Modules(PartitionData **dirtyModules)
{
    m_dirtyFlag = 0;
    if (m_nextDirty != nullptr) {
        m_nextDirty->m_prevDirty = m_prevDirty;
    }

    if (m_prevDirty == nullptr) {
        *dirtyModules = m_nextDirty;
    } else {
        m_prevDirty->m_nextDirty = m_nextDirty;
    }

    m_prevDirty = nullptr;
    m_nextDirty = nullptr;
}

void PartitionData::Make_Dirty(bool b)
{
#ifdef GAME_DLL
    Call_Method<void, PartitionData, bool>(PICK_ADDRESS(0x0053ACA0, 0x0081D3CE), this, b);
#endif
}

ObjectShroudStatus PartitionData::Get_Shrouded_Status(int index)
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, PartitionData, int>(PICK_ADDRESS(0x00539C50, 0x0081C0C8), this, index);
#else
    return SHROUDED_INVALID;
#endif
}

// wb: 0x0081B76E
PartitionCell::~PartitionCell()
{
    captainslog_dbgassert(m_firstCoilInCell == nullptr && m_coiCount == 0, "Destroying a nonempty PartitionCell!");
}

// zh: 0x005399A0 wb: 0x0081DB8C
void PartitionCell::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x005399A0, 0x0081DB8C), this, xfer);
#endif
}

// zh: 0x005399F0 wb: 0x0081BDE2
void PartitionCell::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x005399F0, 0x0081BDE2), this, xfer);
#endif
}

// zh: 0x00541A40 wb: 0x008242E7
void SightingInfo::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferCoord3D(&m_where);
    xfer->xferReal(&m_radius);
    xfer->xferUser(&m_playerIndex, sizeof(m_playerIndex));
    xfer->xferUnsignedInt(&m_frame);
}

// zh: 0x00541A00 wb: 0x00824274
void SightingInfo::Reset()
{
    m_where.Zero();
    m_radius = 0.0f;
    m_playerIndex = 0;
    m_frame = 0;
}

bool PartitionFilterRelationship::Allow(Object *obj)
{
    return ((1 << m_object->Get_Relationship(obj)) & m_unk) != 0;
}

bool PartitionFilterAlive::Allow(Object *obj)
{
    return !obj->Is_Effectively_Dead();
}

bool PartitionFilterSameMapStatus::Allow(Object *obj)
{
    return m_object->Is_Outside_Map() == obj->Is_Outside_Map();
}

ObjectShroudStatus PartitionManager::Get_Prop_Shroud_Status_For_Player(int id, const Coord3D *position) const
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, const PartitionManager, int, const Coord3D *>(
        PICK_ADDRESS(0x0053C7F0, 0x0081F275), this, id, position);
#else
    return SHROUDED_INVALID;
#endif
}

bool PartitionFilterAcceptByKindOf::Allow(Object *obj)
{
    return obj->Is_KindOf_Multi(m_mustBeSet, m_mustBeClear);
}

void PartitionData::Friend_Set_Previous_Shrouded_Status(int index, ObjectShroudStatus status)
{
    m_previousShroudedness[index] = status;
    m_everSeen[index] = status != SHROUDED_NEVERSEEN;
}
