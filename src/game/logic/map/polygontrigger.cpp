/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "polygontrigger.h"
#include "globaldata.h"
#include "terrainlogic.h"
#include "xfer.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
PolygonTrigger *PolygonTrigger::s_thePolygonTriggerListPtr = nullptr;
#endif

int PolygonTrigger::s_currentID = 1;

// BUGFIX initalize all variables
PolygonTrigger::PolygonTrigger(int initial_allocation) :
    m_nextPolygonTrigger(nullptr),
    m_points(nullptr),
    m_numPoints(0),
    m_sizePoints(0),
    m_bounds{},
    m_radius(0),
    m_riverStart(0),
    m_boundsNeedsUpdate(false),
    m_exportWithScripts(false),
    m_isWaterArea(false),
    m_isRiver(false),
    m_isShownInLayer(true),
    m_isSelected(false)
{
    if (initial_allocation < 2) {
        initial_allocation = 2;
    }

    m_points = new ICoord3D[initial_allocation];
    m_sizePoints = initial_allocation;
    m_triggerID = s_currentID++;
    m_waterHandle.m_polygon = this;
}

PolygonTrigger::~PolygonTrigger()
{
    if (m_points != nullptr) {
        delete[] m_points;
        m_points = nullptr;
    }

    if (m_nextPolygonTrigger != nullptr) {
        PolygonTrigger *next;

        for (PolygonTrigger *p = m_nextPolygonTrigger; p != nullptr; p = next) {
            next = p->Get_Next();
            p->Set_Next(nullptr);
            p->Delete_Instance();
        }
    }
}

void PolygonTrigger::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferInt(&m_numPoints);

    for (int i = 0; i < m_numPoints; i++) {
        xfer->xferICoord3D(&m_points[i]);
    }

    xfer->xferIRegion2D(&m_bounds);
    xfer->xferReal(&m_radius);
    xfer->xferBool(&m_boundsNeedsUpdate);
}

void PolygonTrigger::Reallocate()
{
    captainslog_dbgassert(m_numPoints <= m_sizePoints, "Invalid m_numPoints.");

    if (m_numPoints == m_sizePoints) {
        m_sizePoints *= 2;
        ICoord3D *points = new ICoord3D[m_sizePoints];

        for (int i = 0; i < m_numPoints; i++) {
            points[i] = m_points[i];
        }

        delete[] m_points;
        m_points = points;
    }
}

void PolygonTrigger::Update_Bounds() const
{
    m_bounds.lo.y = 8388592;
    m_bounds.lo.x = 8388592;
    m_bounds.hi.y = -8388592;
    m_bounds.hi.x = -8388592;

    for (int i = 0; i < m_numPoints; i++) {
        if (m_points[i].x < m_bounds.lo.x) {
            m_bounds.lo.x = m_points[i].x;
        }

        if (m_points[i].y < m_bounds.lo.y) {
            m_bounds.lo.y = m_points[i].y;
        }

        if (m_points[i].x > m_bounds.hi.x) {
            m_bounds.hi.x = m_points[i].x;
        }

        if (m_points[i].y > m_bounds.hi.y) {
            m_bounds.hi.y = m_points[i].y;
        }
    }

    m_boundsNeedsUpdate = false;
    float x = (float)(m_bounds.hi.x - m_bounds.lo.x) / 2.0f;
    float y = (float)(m_bounds.lo.y + m_bounds.hi.y) / 2.0f;
    m_radius = GameMath::Sqrt(x * x + y * y);
}

void PolygonTrigger::Add_Point(ICoord3D const &point)
{
    captainslog_dbgassert(m_numPoints <= m_sizePoints, "Invalid m_numPoints.");

    if (m_numPoints == m_sizePoints) {
        Reallocate();
    }

    m_points[m_numPoints] = point;
    m_numPoints++;
    m_boundsNeedsUpdate = true;
}

void PolygonTrigger::Set_Point(ICoord3D const &point, int ndx)
{
    captainslog_dbgassert(ndx >= 0 && ndx <= m_numPoints, "Invalid ndx.");

    if (ndx >= 0) {
        if (ndx == m_numPoints) {
            Add_Point(point);
        } else if (ndx <= m_numPoints) {
            m_points[ndx] = point;
            m_boundsNeedsUpdate = true;
        }
    }
}

void PolygonTrigger::Insert_Point(ICoord3D const &point, int ndx)
{
    captainslog_dbgassert(ndx >= 0 && ndx <= m_numPoints, "Invalid ndx.");

    if (ndx >= 0) {
        if (ndx == m_numPoints) {
            Add_Point(point);
        } else {
            if (m_numPoints == m_sizePoints) {
                Reallocate();
            }

            for (int i = m_numPoints; i > ndx; --i) {
                m_points[i] = m_points[i - 1];
            }

            m_points[ndx] = point;
            m_numPoints++;
            m_boundsNeedsUpdate = true;
        }
    }
}

void PolygonTrigger::Delete_Point(int ndx)
{
    captainslog_dbgassert(ndx >= 0 && ndx < m_numPoints, "Invalid ndx.");

    if (ndx >= 0 && ndx < m_numPoints) {
        for (int i = ndx; i < m_numPoints - 1; i++) {
            m_points[i] = m_points[i + 1];
        }

        m_numPoints--;
        m_boundsNeedsUpdate = true;
    }
}

void PolygonTrigger::Get_Center_Point(Coord3D *pOutCoord) const
{
    captainslog_dbgassert(pOutCoord, "pOutCoord was null. Non-Fatal, but shouldn't happen.");

    if (pOutCoord) {
        if (m_boundsNeedsUpdate) {
            Update_Bounds();
        }

        pOutCoord->x = (float)(m_bounds.hi.x + m_bounds.lo.x) / 2.0f;
        pOutCoord->y = (float)(m_bounds.hi.y + m_bounds.lo.y) / 2.0f;
        pOutCoord->z = g_theTerrainLogic->Get_Ground_Height(pOutCoord->x, pOutCoord->y, nullptr);
    }
}

float PolygonTrigger::Get_Radius() const
{
    if (m_boundsNeedsUpdate) {
        Update_Bounds();
    }

    return m_radius;
}

bool PolygonTrigger::Is_Valid() const
{
    return m_numPoints != 0;
}

void PolygonTrigger::Add_Polygon_Trigger(PolygonTrigger *trigger)
{
    for (PolygonTrigger *p = Get_First_Polygon_Trigger(); p != nullptr; p = p->Get_Next()) {
        captainslog_dbgassert(p != trigger, "Attempting to add trigger already in list.");

        if (p == trigger) {
            return;
        }
    }

    trigger->m_nextPolygonTrigger = s_thePolygonTriggerListPtr;
    s_thePolygonTriggerListPtr = trigger;
}

void PolygonTrigger::Remove_Polygon_Trigger(PolygonTrigger *trigger)
{
    PolygonTrigger *t = nullptr;
    PolygonTrigger *p;

    for (p = Get_First_Polygon_Trigger(); p != nullptr && p != trigger; p = p->Get_Next()) {
        t = p;
    }

    captainslog_dbgassert(p != nullptr, "Attempting to remove a polygon not in the list.");

    if (p != nullptr) {
        if (t != nullptr) {
            captainslog_dbgassert(trigger == t->m_nextPolygonTrigger, "Logic error.");
            t->m_nextPolygonTrigger = p->m_nextPolygonTrigger;
        } else {
            captainslog_dbgassert(trigger == s_thePolygonTriggerListPtr, "Logic error.");
            s_thePolygonTriggerListPtr = p->m_nextPolygonTrigger;
        }
    }

    trigger->m_nextPolygonTrigger = nullptr;
}

PolygonTrigger *PolygonTrigger::Get_Polygon_Trigger_By_ID(int id)
{
    for (PolygonTrigger *p = Get_First_Polygon_Trigger(); p != nullptr; p = p->Get_Next()) {
        if (p->Get_ID() == id) {
            return p;
        }
    }

    return nullptr;
}

void PolygonTrigger::Clear_Selected()
{
    for (PolygonTrigger *p = Get_First_Polygon_Trigger(); p != nullptr; p = p->Get_Next()) {
        p->Set_Selected(false);
    }
}

// #TODO Check if maxTriggerId must be incremented before assigning to new PolygonTrigger. 2 PolygonTriggers may share same
// ID.
bool PolygonTrigger::Parse_Polygon_Triggers_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    int maxTriggerId = 0;
    Utf8String triggerName;
    Utf8String layerName;
    Delete_Triggers();
    PolygonTrigger *pPrevTrig = nullptr;
    int count = file.Read_Int32();

    while (count > 0) {
        count--;
        triggerName = file.Read_AsciiString();

        if (info->version >= 4) {
            layerName = file.Read_AsciiString();
        }

        int id = file.Read_Int32();
        bool isWaterArea = false;

        if (info->version >= 2) {
            isWaterArea = file.Read_Byte() != 0;
        }

        bool isRiver = false;
        int riverStart = 0;

        if (info->version >= 3) {
            isRiver = file.Read_Byte() != 0;
            riverStart = file.Read_Int32();
        }

        int numPoints = file.Read_Int32();

        PolygonTrigger *trigger = NEW_POOL_OBJ(PolygonTrigger, numPoints + 1);
        trigger->Set_Trigger_Name(triggerName);

        if (info->version >= 4) {
            trigger->Set_Layer_Name(layerName);
        }

        trigger->Set_Water_Area(isWaterArea);
        trigger->Set_River(isRiver);
        trigger->Set_River_Start(riverStart);
        trigger->m_triggerID = id;

        if (id > maxTriggerId) {
            maxTriggerId = id;
        }

        for (int i = 0; i < numPoints; i++) {
            ICoord3D loc;
            loc.x = file.Read_Int32();
            loc.y = file.Read_Int32();
            loc.z = file.Read_Int32();
            trigger->Add_Point(loc);
        }

        if (numPoints < 2) {
            captainslog_debug("Deleting polygon trigger '%s' with %d points.\n", triggerName.Str(), numPoints);
            trigger->Delete_Instance();
        } else {
            if (pPrevTrig != nullptr) {
                pPrevTrig->Set_Next(trigger);
            } else {
                Add_Polygon_Trigger(trigger);
            }

            pPrevTrig = trigger;
        }
    }

    if (info->version == 1) {
        PolygonTrigger *trigger = NEW_POOL_OBJ(PolygonTrigger, 4);
        trigger->Set_Water_Area(true);
        trigger->m_triggerID = maxTriggerId;
        maxTriggerId++;

        ICoord3D loc;
        loc.x = -300;
        loc.y = -300;
        loc.z = 7;
        trigger->Add_Point(loc);

        loc.x = (int)(g_theWriteableGlobalData->m_waterExtentX + 300.0f);
        trigger->Add_Point(loc);

        loc.y = (int)(g_theWriteableGlobalData->m_waterExtentY + 300.0f);
        trigger->Add_Point(loc);

        loc.x = -300;
        trigger->Add_Point(loc);

        if (pPrevTrig != nullptr) {
            pPrevTrig->Set_Next(trigger);
        } else {
            Add_Polygon_Trigger(trigger);
        }
        pPrevTrig = trigger;
    }

    s_currentID = maxTriggerId + 1;
    captainslog_dbgassert(file.At_End_Of_Chunk(), "Incorrect data file length.");
    return true;
}

void PolygonTrigger::Delete_Triggers()
{
    PolygonTrigger *p = s_thePolygonTriggerListPtr;
    s_thePolygonTriggerListPtr = nullptr;
    s_currentID = 1;
    p->Delete_Instance();
}

// Seems to be related to https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
bool PolygonTrigger::Point_In_Trigger(ICoord3D &point) const
{
    if (m_boundsNeedsUpdate) {
        Update_Bounds();
    }

    if (point.x < m_bounds.lo.x) {
        return false;
    }

    if (point.y < m_bounds.lo.y) {
        return false;
    }

    if (point.x > m_bounds.hi.x) {
        return false;
    }

    if (point.y > m_bounds.hi.y) {
        return false;
    }

    bool inside = false;

    for (int i = 0; i < m_numPoints; i++) {
        int x1 = m_points[i].x;
        int y1 = m_points[i].y;
        int x2;
        int y2;
        if (i == m_numPoints - 1) {
            x2 = m_points[0].x;
            y2 = m_points[0].y;
        } else {
            x2 = m_points[i + 1].x;
            y2 = m_points[i + 1].y;
        }

        if (y1 != y2 && (y1 >= point.y || y2 >= point.y) && (y1 < point.y || y2 < point.y)
            && (x1 >= point.x || x2 >= point.x)) {
            if ((float)point.x <= (float)x1 + (float)((point.y - y1) * (x2 - x1)) / (float)(y2 - y1)) {
                inside = !inside;
            }
        }
    }
    return inside;
}
