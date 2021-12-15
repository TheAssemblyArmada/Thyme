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
#pragma once

#include "always.h"
#include "datachunk.h"
#include "terrainlogic.h"

class PolygonTrigger : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(PolygonTrigger);

public:
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    int Get_Num_Points() const { return m_numPoints; }
    int Get_River_Start() const { return m_riverStart; }
    PolygonTrigger *Get_Next() const { return m_nextPolygonTrigger; }
    bool Is_Water_Area() const { return m_isWaterArea; }
    bool Is_River() const { return m_isRiver; }

    ICoord3D *Get_Point(int i) const
    {
        if (i < 0) {
            i = 0;
        }

        if (i >= m_numPoints) {
            i = m_numPoints - 1;
        }

        return &m_points[i];
    }

    const WaterHandle *Get_Water_Handle() const
    {
        if (Is_Water_Area()) {
            return &m_waterHandle;
        } else {
            return nullptr;
        }
    }

    bool Point_In_Trigger(ICoord3D &point) const;

    static PolygonTrigger *Get_First_Polygon_Trigger() { return s_thePolygonTriggerListPtr; }

    static bool Parse_Polygon_Triggers_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *userdata);
    static void Delete_Triggers();
#ifdef GAME_DLL
    static PolygonTrigger *&s_thePolygonTriggerListPtr;
#else
    static PolygonTrigger *s_thePolygonTriggerListPtr;
#endif
private:
    PolygonTrigger *m_nextPolygonTrigger;
    Utf8String m_triggerName;
    int m_triggerID;
    WaterHandle m_waterHandle;
    ICoord3D *m_points;
    int m_numPoints;
    int m_sizePoints;
    IRegion2D m_bounds;
    float m_radius;
    int m_riverStart;
    char m_boundsNeedsUpdate;
    bool m_exportWithScripts;
    bool m_isWaterArea;
    bool m_isRiver;
    Utf8String m_layerName;
    bool m_isShownInLayer;
    bool m_unk;
};
