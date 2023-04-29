/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Ghost Object
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
#include "geometry.h"
#include "snapshot.h"

class GhostObject;
class Object;
class PartitionData;

class GhostObject : public SnapShot
{
public:
    GhostObject();
    virtual ~GhostObject() {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    virtual void SnapShot(int player) = 0;
    virtual void Update_Parent_Object(Object *obj, PartitionData *data) = 0;
    virtual void Free_SnapShot(int player) = 0;

protected:
    Object *m_parentObject;
    GeometryType m_geoType;
    bool m_geoIsSmall;
    float m_geoMajorRadius;
    float m_geoMinorRadius;
    float m_cachedAngle;
    Coord3D m_cachedPos;
    PartitionData *m_parentPartitionData;
};

class GhostObjectManager : public SnapShot
{
public:
    GhostObjectManager();
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    virtual ~GhostObjectManager() {}
    virtual void Reset() {}
    virtual GhostObject *Add_Ghost_Object(Object *obj, PartitionData *data) { return nullptr; }
    virtual void Remove_Ghost_Object(GhostObject *obj) {}
    virtual void Set_Local_Player_Index(int index) { m_localPlayerIndex = index; }
    virtual void Update_Orphaned_Objects(int *unk, int unk2) {}
    virtual void Release_Partition_Data() {}
    virtual void Restore_Partition_Data() {}

    void Set_Updating_Map_Boundary(bool update) { m_isUpdatingMapBoundary = update; }
    void Set_Is_Loading(bool loading) { m_isLoading = loading; }

protected:
    int m_localPlayerIndex;
    bool m_isUpdatingMapBoundary;
    bool m_isLoading;
    friend class W3DGhostObject;
};

#ifdef GAME_DLL
extern GhostObjectManager *&g_theGhostObjectManager;
#else
extern GhostObjectManager *g_theGhostObjectManager;
#endif
