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
#include "always.h"
#include "coord.h"
#include "asciistring.h"
#include "dict.h"

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
public:
    IMPLEMENT_POOL(MapObject);
    virtual ~MapObject();
    static MapObject *Get_First_Map_Object() { return g_theMapObjectListPtr; }
    MapObject *Get_Next() { return m_nextMapObject; }
    bool Get_Flag(int flag) { return (m_flags & flag) != 0; }
    Coord3D *Get_Location() { return &m_location; }
    Utf8String Get_Name() { return m_objectName; }

private:
    Coord3D m_location;
    Utf8String m_objectName;
    ThingTemplate *m_thingTemplate;
    float m_angle;
    MapObject *m_nextMapObject;
    int m_flags;
    Dict m_properties;
    int m_color;
    RenderObjClass *m_renderObj;
    Shadow *m_shadowObj;
    RenderObjClass *m_bridgeRenderObjects[4];
    int m_runtimeFlags;
#ifdef GAME_DLL
    static MapObject *&g_theMapObjectListPtr;
#else
    static MapObject *g_theMapObjectListPtr;
#endif
};
