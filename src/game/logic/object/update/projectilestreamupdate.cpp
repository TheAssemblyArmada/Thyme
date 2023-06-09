/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Projectile Stream Update Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "projectilestreamupdate.h"
#include "gamelogic.h"
#include "object.h"

ProjectileStreamUpdate::ProjectileStreamUpdate(Thing *thing, ModuleData const *module_data) :
    UpdateModule(thing, module_data),
    m_nextFreeIndex(0),
    m_firstValidIndex(0),
    m_owningObject(INVALID_OBJECT_ID),
    m_targetID(INVALID_OBJECT_ID)
{
    m_targetPos.Zero();

    // #BUGFIX Initialize all members
    for (int i = 0; i < MAX_SEGMENT_COUNT; ++i) {
        m_projectileIDs[i] = INVALID_OBJECT_ID;
    }
}

NameKeyType ProjectileStreamUpdate::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("ProjectileStreamUpdate");
    return _key;
}

void ProjectileStreamUpdate::CRC_Snapshot(Xfer *xfer)
{
    UpdateModule::CRC_Snapshot(xfer);
}

void ProjectileStreamUpdate::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 2;
    xfer->xferVersion(&ver, 2);
    UpdateModule::Xfer_Snapshot(xfer);
    xfer->xferUser(m_projectileIDs, sizeof(m_projectileIDs));
    xfer->xferInt(&m_nextFreeIndex);
    xfer->xferInt(&m_firstValidIndex);
    xfer->xferObjectID(&m_owningObject);

    if (ver >= 2) {
        xfer->xferObjectID(&m_targetID);
        xfer->xferCoord3D(&m_targetPos);
    }
}

void ProjectileStreamUpdate::Load_Post_Process()
{
    UpdateModule::Load_Post_Process();
}

UpdateSleepTime ProjectileStreamUpdate::Update()
{
    Cull_Front_Of_List();

    if (Consider_Dying()) {
        g_theGameLogic->Destroy_Object(Get_Object());
    }

    return UPDATE_SLEEP_TIME_MIN;
}

Module *ProjectileStreamUpdate::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(ProjectileStreamUpdate, thing, module_data);
}

void ProjectileStreamUpdate::Add_Projectile(
    ObjectID source_id, ObjectID projectile_id, ObjectID target_id, const Coord3D *target_pos)
{
    captainslog_dbgassert(m_owningObject == INVALID_OBJECT_ID || m_owningObject == source_id,
        "Two objects are trying to use the same Projectile stream.");

    if (m_owningObject == INVALID_OBJECT_ID) {
        m_owningObject = source_id;
    }

    if (target_id != INVALID_OBJECT_ID) {
        if (target_id != m_targetID) {
            m_projectileIDs[m_nextFreeIndex] = INVALID_OBJECT_ID;
            m_nextFreeIndex = (m_nextFreeIndex + 1) % MAX_SEGMENT_COUNT;
            m_targetID = target_id;
        }

        m_targetPos.Zero();
    } else if (target_pos != nullptr) {
        if (!(m_targetPos == *target_pos)) {
            m_projectileIDs[m_nextFreeIndex] = INVALID_OBJECT_ID;
            m_nextFreeIndex = (m_nextFreeIndex + 1) % MAX_SEGMENT_COUNT;
            m_targetPos = *target_pos;
        }

        m_targetID = INVALID_OBJECT_ID;
    } else {
        captainslog_debug("A projectile stream was fired at neither an object nor a position.  Probably bad.");
    }

    m_projectileIDs[m_nextFreeIndex] = projectile_id;
    m_nextFreeIndex = (m_nextFreeIndex + 1) % MAX_SEGMENT_COUNT;

    captainslog_dbgassert(m_nextFreeIndex != m_firstValidIndex,
        "Need to increase the allowed number of simultaneous particles in ProjectileStreamUpdate.");
}

void ProjectileStreamUpdate::Cull_Front_Of_List()
{
    while (m_firstValidIndex != m_nextFreeIndex && !g_theGameLogic->Find_Object_By_ID(m_projectileIDs[m_firstValidIndex])) {
        m_firstValidIndex = (m_firstValidIndex + 1) % MAX_SEGMENT_COUNT;
    }
}

bool ProjectileStreamUpdate::Consider_Dying()
{
    return m_firstValidIndex == m_nextFreeIndex && m_owningObject != INVALID_OBJECT_ID
        && !g_theGameLogic->Find_Object_By_ID(m_owningObject);
}

void ProjectileStreamUpdate::Get_All_Points(Vector3 *points, int *count)
{
    int num = 0;
    int index = m_firstValidIndex;
    Object *object = g_theGameLogic->Find_Object_By_ID(m_owningObject);

    while (index != m_nextFreeIndex) {
        Object *projectile = g_theGameLogic->Find_Object_By_ID(m_projectileIDs[index]);

        if (projectile != nullptr) {
            const Coord3D *pos = projectile->Get_Position();
            points[num].X = pos->x;
            points[num].Y = pos->y;
            points[num].Z = pos->z;

            if (object != nullptr) {
                if (object->Is_KindOf(KINDOF_VEHICLE)) {
                    pos = object->Get_Position();
                    float z = object->Get_Geometry_Info().Get_Max_Height_Above_Position() + pos->z + 0.5f;
                    Coord3D c;
                    c.x = pos->x - points[num].X;
                    c.y = pos->y - points[num].Y;
                    c.z = 0.0f;

                    if (object->Get_Geometry_Info().Get_Major_Radius() * 1.5f >= c.Length()) {
                        if (points[num].Z <= z) {
                            points[num].Z = z;
                        }
                    }
                }
            }
        } else {
            points[num].X = 0.0f;
            points[num].Y = 0.0f;
            points[num].Z = 0.0f;
        }

        index = (index + 1) % MAX_SEGMENT_COUNT;
        num++;
    }

    *count = num;
}

void ProjectileStreamUpdate::Set_Position(const Coord3D *new_position)
{
    Get_Object()->Set_Position(new_position);
}
