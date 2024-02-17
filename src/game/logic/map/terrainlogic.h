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
#include "asciistring.h"
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "terrainroads.h"

struct DataChunkInfo;
class DataChunkInput;
class Dict;
class Drawable;
class MapObject;
class Matrix3D;
class Object;
class PolygonTrigger;
class ThingTemplate;
class Vector3;

struct TBridgeAttackInfo
{
    Coord3D m_attackPoint1;
    Coord3D m_attackPoint2;
};

enum WaypointID : int32_t
{
    INVALID_WAYPOINT_ID,
};

class BridgeInfo
{
public:
    BridgeInfo();
    Coord3D from;
    Coord3D to;
    float bridge_width;
    Coord3D from_left;
    Coord3D from_right;
    Coord3D to_left;
    Coord3D to_right;
    int bridge_index;
    BodyDamageType cur_damage_state;
    ObjectID bridge_object_id;
    ObjectID tower_object_id[BRIDGE_MAX_TOWERS];
    bool is_destroyed;
};

class Bridge : public MemoryPoolObject
{
    IMPLEMENT_POOL(Bridge)

protected:
    virtual ~Bridge() override;

public:
    Bridge(BridgeInfo &info, Dict *props, Utf8String bridge_template_name);
    Bridge(Object *obj);

    Object *Create_Tower(
        Coord3D *world_pos, BridgeTowerType tower_type, const ThingTemplate *tower_template, Object *bridge);
    float Get_Bridge_Height(const Coord3D *loc, Coord3D *n);
    bool Is_Cell_Entry_Point(const Region2D *cell);
    bool Is_Cell_On_End(const Region2D *cell);
    bool Is_Cell_On_Side(const Region2D *cell);
    bool Is_Point_On_Bridge(const Coord3D *loc);
    Drawable *Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos);
    void Update_Damage_State();

    void Get_Bridge_Info(BridgeInfo *info) { *info = m_bridgeInfo; }
    const BridgeInfo *Peek_Bridge_Info() { return &m_bridgeInfo; }
    void Set_Bridge_Info(BridgeInfo *info) { m_bridgeInfo = *info; }
    void Set_Next(Bridge *bridge) { m_next = bridge; }
    Bridge *Get_Next() { return m_next; }
    void Set_Layer(PathfindLayerEnum layer) { m_layer = layer; }
    PathfindLayerEnum Get_Layer() { return m_layer; }
    Region2D Get_Bounds() { return m_bounds; }
    Utf8String Get_Name() { return m_templateName; }
    void Set_Object_ID(ObjectID id) { m_bridgeInfo.bridge_object_id = id; }

private:
    Bridge *m_next;
    Utf8String m_templateName;
    BridgeInfo m_bridgeInfo;
    Region2D m_bounds;
    PathfindLayerEnum m_layer;
};

class Waypoint : public MemoryPoolObject
{
    IMPLEMENT_POOL(Waypoint)

protected:
    virtual ~Waypoint() override;

public:
    Waypoint(WaypointID id,
        Utf8String name,
        const Coord3D *loc,
        Utf8String label1,
        Utf8String label2,
        Utf8String label3,
        bool bidirectional);

    void Set_Next(Waypoint *waypoint) { m_next = waypoint; }
    void Set_Link(Waypoint *waypoint)
    {
        if (m_numLinks < MAX_LINKS) {
            m_links[m_numLinks++] = waypoint;
        }
    }
    Waypoint *Get_Next() const { return m_next; }
    int Get_Num_Links() const { return m_numLinks; }
    Waypoint *Get_Link(int link) const
    {
        if (link > MAX_LINKS) {
            return nullptr;
        } else {
            return m_links[link];
        }
    }
    Utf8String Get_Name() const { return m_name; }
    WaypointID Get_ID() const { return m_id; }
    const Coord3D *Get_Location() const { return &m_location; }
    Utf8String Get_Path_Label_1() const { return m_pathLabel1; }
    Utf8String Get_Path_Label_2() const { return m_pathLabel2; }
    Utf8String Get_Path_Label_3() const { return m_pathLabel3; }
    bool Is_Bi_Directional() const { return m_pathIsBiDirectional; }
    void Set_Height(float height) { m_location.z = height; }

private:
    enum
    {
        MAX_LINKS = 8
    };

    WaypointID m_id;
    Utf8String m_name;
    Coord3D m_location;
    Waypoint *m_next;
    Waypoint *m_links[MAX_LINKS];
    int m_numLinks;
    Utf8String m_pathLabel1;
    Utf8String m_pathLabel2;
    Utf8String m_pathLabel3;
    bool m_pathIsBiDirectional;
};

class WaterHandle
{
public:
    WaterHandle() : m_polygon(nullptr) {}

    PolygonTrigger *m_polygon;
};

struct ShroudStatusStoreRestore
{
    std::vector<unsigned char> status[MAX_PLAYER_COUNT];
    int width;
};

class TerrainLogic : public SnapShot, public SubsystemInterface
{
public:
    TerrainLogic();
    virtual ~TerrainLogic() override;

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Load_Map(Utf8String filename, bool query);
    virtual void New_Map(bool b);
    virtual float Get_Ground_Height(float x, float y, Coord3D *n) const;
    virtual float Get_Layer_Height(float x, float y, PathfindLayerEnum layer, Coord3D *n, bool b) const;
    virtual void Get_Extent(Region3D *extent) const { captainslog_dbgassert(false, "not implemented"); }
    virtual void Get_Extent_Including_Border(Region3D *extent) const { captainslog_dbgassert(false, "not implemented"); }
    virtual void Get_Maximum_Pathfind_Extent(Region3D *extent) const { captainslog_dbgassert(false, "not implemented"); }
    virtual Coord3D Find_Closest_Edge_Point(const Coord3D *pos) const;
    virtual Coord3D Find_Farthest_Edge_Point(const Coord3D *pos) const;
    virtual bool Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const;
    virtual Utf8String Get_Source_Filename() { return m_filenameString; }
    virtual PathfindLayerEnum Align_On_Terrain(float angle, const Coord3D &pos, bool stick_to_ground, Matrix3D &mtx);
    virtual bool Is_Underwater(float x, float y, float *waterz, float *groundz);
    virtual bool Is_Cliff_Cell(float x, float y) const;
    virtual const WaterHandle *Get_Water_Handle(float x, float y);
    virtual const WaterHandle *Get_Water_Handle_By_Name(Utf8String name);
    virtual float Get_Water_Height(const WaterHandle *water);
    virtual void Set_Water_Height(const WaterHandle *water, float height, float damage_amount, bool force_pathfind_update);
    virtual void Change_Water_Height_Over_Time(
        const WaterHandle *water, float final_height, float transition_time_in_seconds, float damage_amount);
    virtual Waypoint *Get_First_Waypoint() { return m_waypointListHead; }
    virtual Waypoint *Get_Waypoint_By_Name(Utf8String name);
    virtual Waypoint *Get_Waypoint_By_ID(WaypointID id);
    virtual Waypoint *Get_Closest_Waypoint_On_Path(const Coord3D *pos, Utf8String label);
    virtual bool Is_Purpose_Of_Path(Waypoint *way, Utf8String label);
    virtual PolygonTrigger *Get_Trigger_Area_By_Name(Utf8String name);
    virtual Bridge *Get_First_Bridge() const { return m_bridgeListHead; }
    virtual Bridge *Find_Bridge_At(const Coord3D *loc) const;
    virtual Bridge *Find_Bridge_Layer_At(const Coord3D *loc, PathfindLayerEnum layer, bool b) const;
    virtual bool Object_Interacts_With_Bridge_Layer(Object *obj, int layer, bool b) const;
    virtual bool Object_Interacts_With_Bridge_End(Object *obj, int layer) const;
    virtual Drawable *Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos);
    virtual void Add_Bridge_To_Logic(BridgeInfo *info, Dict *props, Utf8String bridge_template_name);
    virtual void Add_Landmark_Bridge_To_Logic(Object *obj);
    virtual void Delete_Bridge(Bridge *bridge);
    virtual void Update_Bridge_Damage_States();

    bool Is_Bridge_Repaired(const Object *bridge);
    bool Is_Bridge_Broken(const Object *bridge);
    void Get_Bridge_Attack_Points(const Object *bridge, TBridgeAttackInfo *attack_info);
    void Enable_Water_Grid(bool enable);
    void Set_Active_Boundary(int new_active_boundary);
    bool Parse_Waypoint_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    void Add_Waypoint(MapObject *map_obj);
    void Add_Waypoint_Link(int id1, int id2);
    void Delete_Waypoints();
    void Delete_Bridges();
    void Find_Axis_Aligned_Bounding_Rect(const WaterHandle *water, Region3D *region);
    void Flatten_Terrain(Object *obj);
    void Create_Crater_In_Terrain(Object *obj);
    PathfindLayerEnum Get_Layer_For_Destination(const Coord3D *pos);
    PathfindLayerEnum Get_Highest_Layer_For_Destination(const Coord3D *pos, bool b);

    bool Have_Bridge_Damage_States_Changed() { return m_bridgeDamageStatesChanged; }

    static bool Parse_Waypoint_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);

protected:
#ifdef GAME_DLL
    static WaterHandle &m_gridWaterHandle;
#else
    static WaterHandle m_gridWaterHandle;
#endif

    enum
    {
        MAX_DYNAMIC_WATER = 64
    };

    struct DynamicWaterEntry
    {
        const WaterHandle *water_table;
        float change_per_frame;
        float target_height;
        float damage_amount;
        float current_height;
    };

    unsigned char *m_mapData;
    int m_mapDX;
    int m_mapDY;
    std::vector<ICoord2D> m_boundaries;
    int m_activeBoundary;
    Waypoint *m_waypointListHead;
    Bridge *m_bridgeListHead;
    bool m_bridgeDamageStatesChanged;
    Utf8String m_filenameString;
    bool m_waterGridEnabled;
    DynamicWaterEntry m_waterToUpdate[MAX_DYNAMIC_WATER];
    int m_numWaterToUpdate;
};

#ifdef GAME_DLL
extern TerrainLogic *&g_theTerrainLogic;
#else
extern TerrainLogic *g_theTerrainLogic;
#endif

bool Line_In_Region(const Coord2D *p1, const Coord2D *p2, const Region2D *clip_region);
void Make_Align_To_Normal_Matrix(float angle, const Coord3D &pos, const Coord3D &n, Matrix3D &tm);
