/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Map Object
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
#include "dict.h"
#include "terrainlogic.h"
#include "terrainroads.h"

class ThingTemplate;
class RenderObjClass;
class Shadow;

enum
{
    FLAG_DRAWS_IN_MIRROR = 0x1,
    FLAG_ROAD_POINT1 = 0x2,
    FLAG_ROAD_POINT2 = 0x4,
    FLAG_ROAD_FLAGS = 0x6,
    FLAG_ROAD_CORNER_ANGLED = 0x8,
    FLAG_BRIDGE_POINT1 = 0x10,
    FLAG_BRIDGE_POINT2 = 0x20,
    FLAG_BRIDGE_FLAGS = 0x30,
    FLAG_ROAD_CORNER_TIGHT = 0x40,
    FLAG_ROAD_JOIN = 0x80,
    FLAG_DONT_RENDER = 0x100,
};

class MapObject : public MemoryPoolObject
{
    IMPLEMENT_POOL(MapObject)

protected:
    virtual ~MapObject() override;

public:
    static MapObject *Get_First_Map_Object() { return s_theMapObjectListPtr; }
    static Dict *Get_World_Dict() { return &s_theWorldDict; }
    MapObject *Get_Next() { return m_nextMapObject; }
    bool Get_Flag(int flag) { return (m_flags & flag) != 0; }
    const Coord3D *Get_Location() { return &m_location; }
    Utf8String Get_Name() { return m_objectName; }
    Dict *Get_Properties() { return &m_properties; }
    void Set_Next_Map(MapObject *obj) { m_nextMapObject = obj; }
    int Get_Flags() { return m_flags; }
    bool Is_Selected() { return (m_runtimeFlags & MO_SELECTED) != 0; }
    bool Is_Light() { return (m_runtimeFlags & MO_LIGHT) != 0; }
    bool Is_Waypoint() { return (m_runtimeFlags & MO_WAYPOINT) != 0; }
    bool Is_Scorch() { return (m_runtimeFlags & MO_SCORCH) != 0; }
    void Set_Flag(int flag) { m_flags |= flag; }
    void Set_Shadow_Obj(Shadow *shadow) { m_shadowObj = shadow; }
    float Get_Angle() { return m_angle; }
    int Get_Color() { return m_color; }
    void Set_Color(int color) { m_color = color; }
    void Set_Angle(float angle) { m_angle = angle; }
    void Set_Location(Coord3D *loc) { m_location = *loc; }
    void Clear_Flag(int flag) { m_flags &= ~flag; }
    void Set_Selected(bool selected)
    {
        if (selected) {
            m_runtimeFlags |= MO_SELECTED;
        } else {
            m_runtimeFlags &= ~MO_SELECTED;
        }
    }
    void Set_Is_Light() { m_runtimeFlags |= MO_LIGHT; }
    void Set_Is_Waypoint() { m_runtimeFlags |= MO_WAYPOINT; }
    void Set_Is_Scorch() { m_runtimeFlags |= MO_SCORCH; }
    RenderObjClass *Get_Render_Obj() { return m_renderObj; }
    MapObject(Coord3D loc, Utf8String name, float angle, int flags, const Dict *props, const ThingTemplate *thing);
    MapObject *Duplicate();
    void Set_Render_Obj(RenderObjClass *obj);
    void Set_Bridge_Render_Object(BridgeTowerType type, RenderObjClass *obj);
    RenderObjClass *Get_Bridge_Render_Object(BridgeTowerType type);
    void Validate();
    void Verify_Valid_Team();
    void Verify_Valid_Unique_ID();
    static void Fast_Assign_All_Unique_IDs();
    void Set_Thing_Template(const ThingTemplate *thing);
    void Set_Name(Utf8String name);
    WaypointID Get_Waypoint_ID();
    Utf8String Get_Waypoint_Name();
    void Set_Waypoint_ID(WaypointID i);
    void Set_Waypoint_Name(Utf8String n);
    static int Count_Map_Objects_With_Owner(const Utf8String &n);
    const ThingTemplate *Get_Thing_Template();

#ifdef GAME_DLL
    MapObject *Hook_Ctor(Coord3D loc, Utf8String name, float angle, int flags, const Dict *props, const ThingTemplate *thing)
    {
        return new (this) MapObject(loc, name, angle, flags, props, thing);
    }
#endif

private:
    enum
    {
        MO_SELECTED = 1,
        MO_LIGHT = 2,
        MO_WAYPOINT = 4,
        MO_SCORCH = 8,
    };

    Coord3D m_location;
    Utf8String m_objectName;
    const ThingTemplate *m_thingTemplate;
    float m_angle;
    MapObject *m_nextMapObject;
    int m_flags;
    Dict m_properties;
    int m_color;
    RenderObjClass *m_renderObj;
    Shadow *m_shadowObj;
    RenderObjClass *m_bridgeRenderObj[BRIDGE_MAX_TOWERS];
    int m_runtimeFlags;
#ifdef GAME_DLL
    static MapObject *&s_theMapObjectListPtr;
    static Dict &s_theWorldDict;
#else
    static MapObject *s_theMapObjectListPtr;
    static Dict s_theWorldDict;
#endif
    friend class WorldHeightMap;
};
