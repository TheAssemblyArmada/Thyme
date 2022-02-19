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
    PolygonTrigger(int32_t initial_allocation);
    virtual ~PolygonTrigger() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    int32_t Get_Num_Points() const { return m_numPoints; }
    int32_t Get_River_Start() const { return m_riverStart; }
    const PolygonTrigger *Get_Next() const { return m_nextPolygonTrigger; }
    PolygonTrigger *Get_Next() { return m_nextPolygonTrigger; }
    bool Is_Water_Area() const { return m_isWaterArea; }
    bool Is_River() const { return m_isRiver; }
    Utf8String Get_Layer_Name() const { return m_layerName; }
    bool Is_Shown_In_Layer() const { return m_isShownInLayer; }
    Utf8String Get_Trigger_Name() const { return m_triggerName; }
    void Set_Layer_Name(Utf8String name) { m_layerName = name; }
    void Set_Is_Shown_In_Layer(bool shown) { m_isShownInLayer = shown; }
    bool Is_Selected() { return m_isSelected; }
    void Set_Selected(bool selected) { m_isSelected = selected; }
    void Set_Trigger_Name(Utf8String name) { m_triggerName = name; }
    int32_t Get_ID() { return m_triggerID; }
    void Set_Do_Export_With_Scripts(bool do_export) { m_exportWithScripts = do_export; }
    void Set_Next(PolygonTrigger *next) { m_nextPolygonTrigger = next; }
    void Set_Water_Area(bool water) { m_isWaterArea = water; }
    void Set_River(bool river) { m_isRiver = river; }
    void Set_River_Start(int32_t start) { m_riverStart = start; }
    bool Do_Export_With_Scripts() { return m_exportWithScripts; }

    ICoord3D *Get_Point(int32_t ndx) const
    {
        if (ndx < 0) {
            ndx = 0;
        }

        if (ndx >= m_numPoints) {
            ndx = m_numPoints - 1;
        }

        return &m_points[ndx];
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
    void Reallocate();
    void Update_Bounds() const;
    void Add_Point(ICoord3D const &point);
    void Set_Point(ICoord3D const &point, int32_t ndx);
    void Insert_Point(ICoord3D const &point, int32_t ndx);
    void Delete_Point(int32_t ndx);
    void Get_Center_Point(Coord3D *pOutCoord) const;
    float Get_Radius() const;
    bool Is_Valid() const;

    static PolygonTrigger *Get_First_Polygon_Trigger() { return s_thePolygonTriggerListPtr; }

    static bool Parse_Polygon_Triggers_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static void Delete_Triggers();
    static void Write_Polygon_Triggers_Data_Chunk(DataChunkOutput &chunk_writer); // needs DataChunkOutput
    static void Add_Polygon_Trigger(PolygonTrigger *trigger);
    static void Remove_Polygon_Trigger(PolygonTrigger *trigger);
    static PolygonTrigger *Get_Polygon_Trigger_By_ID(int32_t id);
    static void Clear_Selected();

private:
#ifdef GAME_DLL
    static PolygonTrigger *&s_thePolygonTriggerListPtr;
#else
    static PolygonTrigger *s_thePolygonTriggerListPtr;
#endif
    static int32_t s_currentID;

    PolygonTrigger *m_nextPolygonTrigger;
    Utf8String m_triggerName;
    int32_t m_triggerID;
    WaterHandle m_waterHandle;
    ICoord3D *m_points;
    int32_t m_numPoints;
    int32_t m_sizePoints;
    mutable IRegion2D m_bounds;
    mutable float m_radius;
    int32_t m_riverStart;
    mutable bool m_boundsNeedsUpdate;
    bool m_exportWithScripts;
    bool m_isWaterArea;
    bool m_isRiver;
    Utf8String m_layerName;
    bool m_isShownInLayer;
    bool m_isSelected;
};
