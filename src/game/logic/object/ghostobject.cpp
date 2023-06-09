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
#include "ghostobject.h"
#include "gamelogic.h"
#include "object.h"
#include "xfer.h"

#ifndef GAME_DLL
GhostObjectManager *g_theGhostObjectManager = nullptr;
#endif

GhostObject::GhostObject() :
    m_parentObject(nullptr),
    m_geoType(GEOMETRY_SPHERE),
    m_geoIsSmall(false),
    m_geoMajorRadius(0.0f),
    m_geoMinorRadius(0.0f),
    m_cachedAngle(0.0f),
    m_parentPartitionData(nullptr)
{
    m_cachedPos.Zero();
}

void GhostObject::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectID parent_id = INVALID_OBJECT_ID;

    if (m_parentObject != nullptr) {
        parent_id = m_parentObject->Get_ID();
    }

    xfer->xferObjectID(&parent_id);

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_parentObject = g_theGameLogic->Find_Object_By_ID(parent_id);

        if (parent_id != INVALID_OBJECT_ID) {
            captainslog_relassert(
                m_parentObject != nullptr, CODE_06, "GhostObject::Xfer_Snapshot - Unable to connect m_parentObject");
        }
    }

    xfer->xferUser(&m_geoType, sizeof(m_geoType));
    xfer->xferBool(&m_geoIsSmall);
    xfer->xferReal(&m_geoMajorRadius);
    xfer->xferReal(&m_geoMinorRadius);
    xfer->xferReal(&m_cachedAngle);
    xfer->xferCoord3D(&m_cachedPos);
}

GhostObjectManager::GhostObjectManager() : m_localPlayerIndex(0), m_isUpdatingMapBoundary(false), m_isLoading(false) {}

void GhostObjectManager::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferInt(&m_localPlayerIndex);
}
