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
#include "w3dghostobject.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "hlod.h"
#include "matinfo.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dmodeldraw.h"
#include "w3dscene.h"

RenderObjClass::Material_Override s_animationDisableOverride;

void Disable_UV_Animations(RenderObjClass *robj)
{
    if (robj != nullptr && robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        for (int sub_obj_idx = 0; sub_obj_idx < robj->Get_Num_Sub_Objects(); sub_obj_idx++) {
            RenderObjClass *subobj = robj->Get_Sub_Object(sub_obj_idx);

            if (subobj != nullptr && subobj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                MaterialInfoClass *mat_info = subobj->Get_Material_Info();

                if (mat_info != nullptr) {
                    for (int mat_idx = 0; mat_idx < mat_info->Vertex_Material_Count(); mat_idx++) {
                        TextureMapperClass *mapper = mat_info->Get_Vertex_Material(mat_idx)->Get_Mapper();

                        if (mapper != nullptr && mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET) {
                            subobj->Set_User_Data(&s_animationDisableOverride);
                        }
                    }

                    mat_info->Release_Ref();
                }

                if (subobj->Get_Name() != nullptr) {
                    if (strstr(subobj->Get_Name(), "MUZZLEFX")) {
                        subobj->Set_Hidden(true);
                    }
                }
            }

            Ref_Ptr_Release(subobj);
        }
    }
}

W3DRenderObjectSnapshot::W3DRenderObjectSnapshot(RenderObjClass *robj, DrawableInfo *info, bool copy_render_obj) :
    m_renderObj(nullptr), m_next(nullptr)
{
    Update(robj, info, copy_render_obj);
}

void W3DRenderObjectSnapshot::Update(RenderObjClass *robj, DrawableInfo *info, bool copy_render_obj)
{
    Ref_Ptr_Release(m_renderObj);

    if (copy_render_obj) {
        m_renderObj = robj->Clone();
        m_renderObj->Set_House_Color(robj->Get_House_Color());
        m_renderObj->Set_Transform(robj->Get_Transform());

        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            float frame;
            int frames;
            int mode;
            float multiplier;
            HLodClass *lod = static_cast<HLodClass *>(robj);
            HAnimClass *anim = lod->Peek_Animation_And_Info(frame, frames, mode, multiplier);
            m_renderObj->Set_Animation(anim, frame);
            Disable_UV_Animations(m_renderObj);
        }
    } else {
        m_renderObj = robj;
    }

    m_renderObj->Set_User_Data(info);
}

W3DRenderObjectSnapshot::~W3DRenderObjectSnapshot()
{
    Ref_Ptr_Release(m_renderObj);
}

void W3DRenderObjectSnapshot::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    Matrix3D tm = m_renderObj->Get_Transform();
    xfer->xferUser(&tm, sizeof(tm));

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_renderObj->Set_Transform(tm);
    }

    int num_sub_objs = m_renderObj->Get_Num_Sub_Objects();
    xfer->xferInt(&num_sub_objs);
    Utf8String name;

    for (int sub_obj_idx = 0; sub_obj_idx < num_sub_objs; sub_obj_idx++) {
        RenderObjClass *subobj;

        if (xfer->Get_Mode() == XFER_SAVE) {
            subobj = m_renderObj->Get_Sub_Object(sub_obj_idx);
            name = subobj->Get_Name();
            xfer->xferAsciiString(&name);
        } else {
            xfer->xferAsciiString(&name);
            subobj = m_renderObj->Get_Sub_Object_By_Name(name.Str(), nullptr);
        }

        bool visible;

        if (subobj != nullptr) {
            visible = static_cast<bool>(subobj->Is_Not_Hidden_At_All());
        }

        xfer->xferBool(&visible);

        if (subobj != nullptr && xfer->Get_Mode() == XFER_LOAD) {
            subobj->Set_Hidden(!visible);
        }

        if (subobj != nullptr) {
            tm = subobj->Get_Transform();
        }

        xfer->xferUser(&tm, sizeof(tm));

        if (subobj != nullptr && xfer->Get_Mode() == XFER_LOAD) {
            subobj->Set_Transform(tm);
        }

        if (subobj != nullptr && subobj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            HLodClass *lod = static_cast<HLodClass *>(subobj);
            lod->Set_Hierarchy_Valid(true);
        }

        Ref_Ptr_Release(subobj);
    }

    m_renderObj->Set_Sub_Object_Transforms_Dirty(false);
}

W3DGhostObject::W3DGhostObject()
{
    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        m_parentShapshots[i] = nullptr;
    }

    m_info.drawable = nullptr;
    m_info.flags = 0;
    m_info.ghost_object = nullptr;
    m_info.object_id = INVALID_OBJECT_ID;
    m_next = nullptr;
    m_prev = nullptr;
}

W3DGhostObject::~W3DGhostObject() {}

void W3DGhostObject::CRC_Snapshot(Xfer *xfer)
{
    GhostObject::CRC_Snapshot(xfer);
}

void W3DGhostObject::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    GhostObject::Xfer_Snapshot(xfer);
    xfer->xferObjectID(&m_info.object_id);
    xfer->xferInt(&m_info.flags);
    DrawableID drawable_id;

    if (m_info.drawable != nullptr) {
        drawable_id = m_info.drawable->Get_ID();
    } else {
        drawable_id = INVALID_DRAWABLE_ID;
    }

    xfer->xferDrawableID(&drawable_id);

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_info.drawable = g_theGameClient->Find_Drawable_By_ID(drawable_id);
    }

    for (int player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
        unsigned char snapshot_count = 0;

        for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player_idx]; snap != nullptr; snap = snap->m_next) {
            snapshot_count++;
        }

        xfer->xferUnsignedByte(&snapshot_count);
        captainslog_relassert(snapshot_count != 0 || m_parentShapshots[player_idx] == nullptr,
            CODE_06,
            "W3DGhostObject::Xfer_Snapshot - m_parentShapshots[ %d ] has data present but the count from the xfer stream is "
            "empty",
            player_idx);
        Utf8String name;
        float scale;
        unsigned int color;

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player_idx]; snap != nullptr; snap = snap->m_next) {
                name = snap->m_renderObj->Get_Name();
                xfer->xferAsciiString(&name);
                scale = snap->m_renderObj->Get_ObjectScale();
                xfer->xferReal(&scale);
                color = snap->m_renderObj->Get_House_Color();
                xfer->xferUnsignedInt(&color);
                xfer->xferSnapshot(snap);
            }
        } else {
            W3DRenderObjectSnapshot *snap = nullptr;

            for (unsigned int snapshot_idx = 0; snapshot_idx < snapshot_count; snapshot_idx++) {
                xfer->xferAsciiString(&name);
                xfer->xferReal(&scale);
                xfer->xferUnsignedInt(&color);
                RenderObjClass *robj =
                    W3DDisplay::s_assetManager->Create_Render_Obj(name.Str(), scale, color, nullptr, nullptr);
                Disable_UV_Animations(robj);
                W3DRenderObjectSnapshot *new_snap = new W3DRenderObjectSnapshot(robj, &m_info, false);

                if (snap != nullptr) {
                    snap->m_next = new_snap;
                } else {
                    m_parentShapshots[player_idx] = new_snap;
                }

                snap = new_snap;
                xfer->xferSnapshot(new_snap);
                W3DDisplay::s_3DScene->Add_Render_Object(new_snap->m_renderObj);
            }
        }
    }

    if (m_parentObject != nullptr && m_parentShapshots[g_thePlayerList->Get_Local_Player()->Get_Player_Index()]
        && xfer->Get_Mode() == XFER_LOAD) {
        Remove_Parent_Object();
    }

    unsigned char snapshot_count = 0;

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        if (m_parentShapshots[i] != nullptr) {
            snapshot_count++;
        }
    }

    xfer->xferUnsignedByte(&snapshot_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (unsigned char player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
            if (m_parentShapshots[player_idx] != nullptr) {
                xfer->xferUnsignedByte(&player_idx);
                ObjectShroudStatus status = m_parentPartitionData->Get_Shrouded_Status(player_idx);
                xfer->xferUser(&status, sizeof(status));
            }
        }
    } else {
        for (unsigned char snapshot_idx = 0; snapshot_idx < snapshot_count; snapshot_idx++) {
            unsigned char player_idx;
            xfer->xferUnsignedByte(&player_idx);
            ObjectShroudStatus status;
            xfer->xferUser(&status, sizeof(status));
            m_parentPartitionData->Friend_Set_Previous_Shrouded_Status(player_idx, status);
        }
    }
}

void W3DGhostObject::Load_Post_Process()
{
    GhostObject::Load_Post_Process();
}

void W3DGhostObject::SnapShot(int player)
{
    if (player == g_theGhostObjectManager->m_localPlayerIndex) {
        bool hidden = false;
        Drawable *drawable = m_parentObject->Get_Drawable();

        if (drawable->Is_Hidden()) {
            hidden = true;
        }

        if (!hidden) {
            W3DRenderObjectSnapshot *snap = nullptr;
            W3DRenderObjectSnapshot *parent_snaps = m_parentShapshots[player];

            for (DrawModule **modules = drawable->Get_Draw_Modules(); *modules != nullptr; modules++) {
                ObjectDrawInterface *draw = (*modules)->Get_Object_Draw_Interface();

                if (draw != nullptr) {
                    RenderObjClass *robj = static_cast<W3DModelDraw *>(draw)->Get_Render_Object();

                    if (robj != nullptr) {
                        if (parent_snaps != nullptr) {
                            parent_snaps->Update(robj, &m_info, true);
                        } else {
                            parent_snaps = new W3DRenderObjectSnapshot(robj, &m_info, true);

                            if (snap != nullptr) {
                                snap->m_next = parent_snaps;
                            } else {
                                m_parentShapshots[player] = parent_snaps;
                            }
                        }

                        if (player == static_cast<W3DGhostObjectManager *>(g_theGhostObjectManager)->m_localPlayerIndex) {
                            robj->Remove();
                            W3DDisplay::s_3DScene->Add_Render_Object(parent_snaps->m_renderObj);
                        }

                        snap = parent_snaps;
                        parent_snaps = snap->m_next;
                    }
                }
            }

            if (parent_snaps != m_parentShapshots[player]) {
                m_geoType = m_parentObject->Get_Geometry_Info().Get_Type();
                m_geoIsSmall = m_parentObject->Get_Geometry_Info().Is_Small();
                m_geoMajorRadius = m_parentObject->Get_Geometry_Info().Get_Major_Radius();
                m_geoMinorRadius = m_parentObject->Get_Geometry_Info().Get_Minor_Radius();
                m_cachedPos = *m_parentObject->Get_Position();
                m_cachedAngle = m_parentObject->Get_Orientation();
            }
        }
    }
}

void W3DGhostObject::Update_Parent_Object(Object *obj, PartitionData *data)
{
    m_parentObject = obj;
    m_parentPartitionData = data;
}

void W3DGhostObject::Free_SnapShot(int player)
{
    if (player == g_theGhostObjectManager->m_localPlayerIndex) {
        if (m_parentShapshots[player] != nullptr) {
            if (player == g_theGhostObjectManager->m_localPlayerIndex) {
                Remove_From_Scene(player);

                if (m_parentObject != nullptr) {
                    Restore_Parent_Object();
                }
            }

            W3DRenderObjectSnapshot *next_snap;

            for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player]; snap != nullptr; snap = next_snap) {
                next_snap = snap->m_next;
                delete snap;
            }

            m_parentShapshots[player] = nullptr;
        }
    }
}

void W3DGhostObject::Remove_Parent_Object()
{
    if (m_parentObject != nullptr) {
        Drawable *drawable = m_parentObject->Get_Drawable();

        if (drawable != nullptr) {
            drawable->Set_Fully_Obscured_By_Shroud(true);

            for (DrawModule **modules = drawable->Get_Draw_Modules(); *modules != nullptr; modules++) {
                ObjectDrawInterface *draw = (*modules)->Get_Object_Draw_Interface();

                if (draw != nullptr) {
                    RenderObjClass *robj = static_cast<W3DModelDraw *>(draw)->Get_Render_Object();

                    if (robj != nullptr) {
                        robj->Remove();
                    }
                }
            }
        }
    }
}

void W3DGhostObject::Restore_Parent_Object()
{
    Drawable *drawable = m_parentObject->Get_Drawable();

    if (drawable != nullptr) {
        drawable->Set_Fully_Obscured_By_Shroud(false);

        for (DrawModule **modules = drawable->Get_Draw_Modules(); *modules != nullptr; modules++) {
            ObjectDrawInterface *draw = (*modules)->Get_Object_Draw_Interface();

            if (draw != nullptr) {
                RenderObjClass *robj = static_cast<W3DModelDraw *>(draw)->Get_Render_Object();

                if (robj != nullptr) {
                    if (robj->Peek_Scene() == nullptr) {
                        W3DDisplay::s_3DScene->Add_Render_Object(robj);
                    }
                }
            }
        }
    }
}

void W3DGhostObject::Remove_From_Scene(int player)
{
    for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player]; snap != nullptr; snap = snap->m_next) {
        snap->m_renderObj->Remove();
    }
}

void W3DGhostObject::Add_To_Scene(int player)
{
    for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player]; snap != nullptr; snap = snap->m_next) {
        W3DDisplay::s_3DScene->Add_Render_Object(snap->m_renderObj);
    }
}

ObjectShroudStatus W3DGhostObject::Get_Shroud_Status(int player) const
{
    return m_parentPartitionData->Get_Shrouded_Status(player);
}

void W3DGhostObject::Free_All_SnapShots()
{
    int player_idx = g_theGhostObjectManager->m_localPlayerIndex;
    if (m_parentShapshots[player_idx] != nullptr) {
        Remove_From_Scene(player_idx);

        if (m_parentObject != nullptr) {
            Restore_Parent_Object();
        }

        W3DRenderObjectSnapshot *next_snap;

        for (W3DRenderObjectSnapshot *snap = m_parentShapshots[player_idx]; snap != nullptr; snap = next_snap) {
            next_snap = snap->m_next;
            delete snap;
        }

        m_parentShapshots[player_idx] = nullptr;
    }
}

W3DGhostObjectManager::W3DGhostObjectManager() : m_freeObjectList(nullptr), m_objectList(nullptr) {}

W3DGhostObjectManager::~W3DGhostObjectManager()
{
    Reset();
    W3DGhostObject *next_obj;

    for (W3DGhostObject *obj = m_freeObjectList; obj != nullptr; obj = next_obj) {
        next_obj = obj->m_next;
        delete obj;
    }
}

void W3DGhostObjectManager::Reset()
{
    W3DGhostObject *next_obj;

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = next_obj) {
        next_obj = obj->m_next;

        if (obj->m_parentObject == nullptr) {
            g_thePartitionManager->Unregister_Ghost_Object(obj);
            Remove_Ghost_Object(obj);
        }
    }

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = next_obj) {
        next_obj = obj->m_next;
        Remove_Ghost_Object(obj);
    }
}

void W3DGhostObjectManager::Remove_Ghost_Object(GhostObject *obj)
{
    W3DGhostObject *ghost_obj = static_cast<W3DGhostObject *>(obj);
    if (ghost_obj != nullptr) {
        ghost_obj->Free_All_SnapShots();
        W3DGhostObject *next_ghost_obj = ghost_obj->m_next;

        if (next_ghost_obj != nullptr) {
            next_ghost_obj->m_prev = ghost_obj->m_prev;
        }

        W3DGhostObject *prev_ghost_obj = ghost_obj->m_prev;

        if (prev_ghost_obj != nullptr) {
            prev_ghost_obj->m_next = ghost_obj->m_next;
        } else {
            m_objectList = ghost_obj->m_next;
        }

        ghost_obj->m_prev = nullptr;
        ghost_obj->m_next = m_freeObjectList;
        W3DGhostObject *free_ghost_obj = m_freeObjectList;

        if (free_ghost_obj != nullptr) {
            free_ghost_obj->m_prev = ghost_obj;
        }

        m_freeObjectList = ghost_obj;
    }
}

GhostObject *W3DGhostObjectManager::Add_Ghost_Object(Object *obj, PartitionData *data)
{
    if (m_isUpdatingMapBoundary || m_isLoading) {
        return nullptr;
    }

    W3DGhostObject *free_ghost_head = m_freeObjectList;

    if (free_ghost_head != nullptr) {
        W3DGhostObject *next_free_ghost = free_ghost_head->m_next;

        if (next_free_ghost != nullptr) {
            next_free_ghost->m_prev = free_ghost_head->m_prev;
        }

        W3DGhostObject *prev_free_ghost = free_ghost_head->m_prev;

        if (prev_free_ghost != nullptr) {
            prev_free_ghost->m_next = free_ghost_head->m_next;
        } else {
            m_freeObjectList = free_ghost_head->m_next;
        }
    } else {
        free_ghost_head = new W3DGhostObject();
    }

    free_ghost_head->m_prev = nullptr;
    free_ghost_head->m_next = m_objectList;
    W3DGhostObject *ghost_head = m_objectList;

    if (ghost_head != nullptr) {
        ghost_head->m_prev = free_ghost_head;
    }

    m_objectList = free_ghost_head;
    free_ghost_head->m_parentObject = obj;
    free_ghost_head->m_info.drawable = nullptr;
    free_ghost_head->m_info.ghost_object = free_ghost_head;
    free_ghost_head->m_parentPartitionData = data;
    return free_ghost_head;
}

void W3DGhostObjectManager::Update_Orphaned_Objects(int *unk, int unk2)
{
    W3DGhostObject *next_obj;

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = next_obj) {
        next_obj = obj->m_next;

        if (obj->m_parentObject == nullptr) {
            bool has_snapshot = false;
            obj->Get_Shroud_Status(m_localPlayerIndex);

            if (obj->m_parentShapshots[m_localPlayerIndex] != nullptr) {
                has_snapshot = true;
            }

            if (!has_snapshot) {
                g_thePartitionManager->Unregister_Ghost_Object(obj);
                obj->m_parentPartitionData = nullptr;
                Remove_Ghost_Object(obj);
            }
        }
    }
}

void W3DGhostObjectManager::Release_Partition_Data()
{
    W3DGhostObject *next_obj;

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = next_obj) {
        next_obj = obj->m_next;

        if (obj->m_parentObject != nullptr) {
            obj->m_parentPartitionData->Set_Ghost_Object(nullptr);
        } else {
            g_thePartitionManager->Unregister_Ghost_Object(obj);
        }

        obj->m_parentPartitionData = nullptr;
    }
}

void W3DGhostObjectManager::Restore_Partition_Data()
{
    W3DGhostObject *next_obj;

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = next_obj) {
        Object *parent_obj = obj->m_parentObject;
        next_obj = obj->m_next;

        if (parent_obj != nullptr) {
            parent_obj->Get_Partition_Data()->Set_Ghost_Object(obj);
            obj->m_parentPartitionData = obj->m_parentObject->Get_Partition_Data();
        } else {
            g_thePartitionManager->Register_Ghost_Object(obj);
        }

        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            if (obj->m_parentShapshots[i] != nullptr) {
                obj->m_parentPartitionData->Friend_Set_Previous_Shrouded_Status(i, SHROUDED_SEEN);
            }
        }
    }
}

void W3DGhostObjectManager::CRC_Snapshot(Xfer *xfer)
{
    GhostObjectManager::CRC_Snapshot(xfer);
}

void W3DGhostObjectManager::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    GhostObjectManager::Xfer_Snapshot(xfer);
    unsigned short ghost_count = 0;

    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = obj->m_next) {
        ghost_count++;
    }

    xfer->xferUnsignedShort(&ghost_count);
    ObjectID object_id;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = obj->m_next) {
            if (obj->m_parentObject != nullptr) {
                object_id = obj->m_parentObject->Get_ID();
            } else {
                object_id = INVALID_OBJECT_ID;
            }

            xfer->xferObjectID(&object_id);
            xfer->xferSnapshot(obj);
        }
    } else {
        g_theGhostObjectManager->Set_Is_Loading(false);
        for (unsigned int i = 0; i < ghost_count; i++) {
            xfer->xferObjectID(&object_id);
            Object *obj;
            GhostObject *ghost_obj;

            if (object_id != INVALID_OBJECT_ID) {
                obj = g_theGameLogic->Find_Object_By_ID(object_id);
            } else {
                obj = nullptr;
            }

            if (obj != nullptr) {
                ghost_obj = Add_Ghost_Object(obj, obj->Get_Partition_Data());
                obj->Get_Partition_Data()->Set_Ghost_Object(ghost_obj);
            } else {
                ghost_obj = Add_Ghost_Object(nullptr, nullptr);
                g_thePartitionManager->Register_Ghost_Object(ghost_obj);
            }

            xfer->xferSnapshot(ghost_obj);
        }
    }
}

void W3DGhostObjectManager::Load_Post_Process()
{
    GhostObjectManager::Load_Post_Process();
}

void W3DGhostObjectManager::Set_Local_Player_Index(int index)
{
    for (W3DGhostObject *obj = m_objectList; obj != nullptr; obj = obj->m_next) {
        obj->Remove_From_Scene(m_localPlayerIndex);

        if (obj->m_parentShapshots[index] != nullptr) {
            if (obj->m_parentShapshots[m_localPlayerIndex] == nullptr && obj->m_parentObject != nullptr) {
                obj->Remove_Parent_Object();
            }

            obj->Add_To_Scene(index);
        } else if (obj->m_parentShapshots[m_localPlayerIndex] != nullptr && obj->m_parentObject == nullptr) {
            obj->Restore_Parent_Object();
        }
    }

    m_localPlayerIndex = index;
}
