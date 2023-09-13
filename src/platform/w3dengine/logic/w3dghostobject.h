/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Ghost Object
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
#include "drawable.h"
#include "ghostobject.h"

class RenderObjClass;

class W3DRenderObjectSnapshot : public SnapShot
{
public:
    W3DRenderObjectSnapshot(RenderObjClass *robj, DrawableInfo *info, bool copy_render_obj);
    ~W3DRenderObjectSnapshot();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Update(RenderObjClass *robj, DrawableInfo *info, bool copy_render_obj);

private:
    RenderObjClass *m_renderObj;
    W3DRenderObjectSnapshot *m_next;
    friend class W3DGhostObject;
};

class W3DGhostObject : public GhostObject
{
public:
    W3DGhostObject();
    ~W3DGhostObject();
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void SnapShot(int player) override;
    virtual void Update_Parent_Object(Object *obj, PartitionData *data) override;
    virtual void Free_SnapShot(int player) override;

    void Remove_Parent_Object();
    void Restore_Parent_Object();
    void Remove_From_Scene(int player);
    void Free_All_SnapShots();
    void Add_To_Scene(int player);
    ObjectShroudStatus Get_Shroud_Status(int player) const;

private:
    W3DRenderObjectSnapshot *m_parentShapshots[MAX_PLAYER_COUNT];
    DrawableInfo m_info;
    W3DGhostObject *m_next;
    W3DGhostObject *m_prev;
    friend class W3DGhostObjectManager;
};

class W3DGhostObjectManager : public GhostObjectManager
{
public:
    W3DGhostObjectManager();
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual ~W3DGhostObjectManager() override;
    virtual void Reset() override;
    virtual GhostObject *Add_Ghost_Object(Object *obj, PartitionData *data) override;
    virtual void Remove_Ghost_Object(GhostObject *obj) override;
    virtual void Set_Local_Player_Index(int index) override;
    virtual void Update_Orphaned_Objects(int *unk, int unk2) override;
    virtual void Release_Partition_Data() override;
    virtual void Restore_Partition_Data() override;

private:
    W3DGhostObject *m_freeObjectList;
    W3DGhostObject *m_objectList;
    friend class W3DGhostObject;
};
