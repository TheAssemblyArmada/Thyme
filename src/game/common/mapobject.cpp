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
#include "mapobject.h"
#include "color.h"
#include "colorspace.h"
#include "rendobj.h"
#include "sideslist.h"
#include "staticnamekey.h"
#include "thingtemplate.h"
#include <deque>
#ifndef GAME_DLL
MapObject *MapObject::s_theMapObjectListPtr;
Dict MapObject::s_theWorldDict;
#endif

MapObject::MapObject(Coord3D loc, Utf8String name, float angle, int flags, const Dict *props, const ThingTemplate *thing) :
    m_location(loc),
    m_objectName(name),
    m_thingTemplate(thing),
    m_angle(Normalize_Angle(angle)),
    m_nextMapObject(nullptr),
    m_flags(flags),
    m_properties(0),
    m_color(Make_Color(0, 255, 0, 0)),
    m_renderObj(nullptr),
    m_shadowObj(nullptr),
    m_runtimeFlags(0)
{
    if (props) {
        m_properties = *props;
    } else {
        m_properties.Set_Int(g_objectInitialHealthKey, 100);
        m_properties.Set_Bool(g_objectEnabledKey, true);
        m_properties.Set_Bool(g_objectIndestructibleKey, false);
        m_properties.Set_Bool(g_objectUnsellableKey, false);
        m_properties.Set_Bool(g_objectPoweredKey, true);
        m_properties.Set_Bool(g_objectRecruitableAIKey, true);
        m_properties.Set_Bool(g_objectTargetableKey, false);
    }

    for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
        Set_Bridge_Render_Object((BridgeTowerType)i, nullptr);
    }
}

MapObject::~MapObject()
{
    Set_Render_Obj(nullptr);
    Set_Shadow_Obj(nullptr);

    if (m_nextMapObject) {
        MapObject *next;

        for (MapObject *m = m_nextMapObject; m; m = next) {
            next = m->Get_Next();
            m->Set_Next_Map(nullptr);
            m->Delete_Instance();
        }
    }

    for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
        Set_Bridge_Render_Object((BridgeTowerType)i, nullptr);
    }
}

// untested, worldbuilder only
MapObject *MapObject::Duplicate()
{
    MapObject *m = NEW_POOL_OBJ(MapObject, m_location, m_objectName, m_angle, m_flags, &m_properties, m_thingTemplate);
    Set_Color(m->Get_Color());
    m_runtimeFlags = m->m_runtimeFlags;
    return m;
}

void MapObject::Set_Render_Obj(RenderObjClass *obj)
{
    Ref_Ptr_Set(m_renderObj, obj);
}

void MapObject::Set_Bridge_Render_Object(BridgeTowerType type, RenderObjClass *obj)
{
    if (type >= BRIDGE_TOWER_FROM_LEFT && type < BRIDGE_MAX_TOWERS) {
        Ref_Ptr_Set(m_bridgeRenderObj[type], obj);
    }
}

RenderObjClass *MapObject::Get_Bridge_Render_Object(BridgeTowerType type)
{
    if (type >= BRIDGE_TOWER_FROM_LEFT && type < BRIDGE_MAX_TOWERS) {
        return m_bridgeRenderObj[type];
    }

    return nullptr;
}

// untested, worldbuilder only
void MapObject::Validate()
{
    Verify_Valid_Team();
    Verify_Valid_Unique_ID();
}

// untested, worldbuilder only
void MapObject::Verify_Valid_Team()
{
    bool exists;
    Utf8String owner = Get_Properties()->Get_AsciiString(g_originalOwnerKey, &exists);

    if (exists) {
        bool match = false;

        for (int i = 0; i < g_theSidesList->Get_Num_Teams(); i++) {

            TeamsInfo *info = g_theSidesList->Get_Team_Info(i);

            if (info) {
                Utf8String team = info->Get_Dict()->Get_AsciiString(g_teamNameKey, &exists);

                if (exists && !team.Compare(owner)) {
                    match = true;
                }
            }
        }

        if (!match) {
            Get_Properties()->Remove(g_originalOwnerKey);
        }
    }
}

// untested, worldbuilder only
void MapObject::Verify_Valid_Unique_ID()
{
    MapObject *o = Get_First_Map_Object();
    int id = -1;

    while (o) {
        if (o != this && !o->Is_Waypoint()) {
            bool exists;
            Utf8String id2 = o->Get_Properties()->Get_AsciiString(g_uniqueIDKey, &exists);
            const char *str = id2.Reverse_Find(' ');
            int newid = -1;

            if (str) {
                newid = atoi(str);
            }

            if (newid > id) {
                id = newid;
            }

            break;
        }
        o = o->Get_Next();
    }

    const char *str;

    if (Get_Thing_Template()) {
        str = Get_Thing_Template()->Get_Name().Str();
    } else if (Is_Waypoint()) {
        str = Get_Waypoint_Name().Str();
    } else {
        str = Get_Name().Str();
    }

    const char *str2 = str;

    while (*str) {
        if (*str == '/') {
            str2 = str + 1;
        }
        str++;
    }

    Utf8String s;

    if (Is_Waypoint()) {
        s.Format("%s", str2);
    } else {
        s.Format("%s %d", str2, id + 1);
    }

    Get_Properties()->Set_AsciiString(g_uniqueIDKey, s);
}

// untested, worldbuilder only
void MapObject::Fast_Assign_All_Unique_IDs()
{
    MapObject *o = Get_First_Map_Object();
    std::deque<MapObject *> objs;
    int count = 0;

    while (o) {
        count++;
        objs.push_back(o);
        o = o->Get_Next();
    }

    int id = 0;

    while (count) {
        MapObject *m = objs.back();

        const char *str;

        if (m->Get_Thing_Template()) {
            str = m->Get_Thing_Template()->Get_Name().Str();
        } else if (m->Is_Waypoint()) {
            str = m->Get_Waypoint_Name().Str();
        } else {
            str = m->Get_Name().Str();
        }

        const char *str2 = str;

        while (*str) {
            if (*str == '/') {
                str2 = str + 1;
            }
            str++;
        }

        Utf8String s;

        if (m->Is_Waypoint()) {
            s.Format("%s", str2);
        } else {
            s.Format("%s %d", str2, id + 1);
        }

        m->Get_Properties()->Set_AsciiString(g_uniqueIDKey, s);
        objs.pop_back();
        id++;
        count--;
    }
}

void MapObject::Set_Thing_Template(const ThingTemplate *thing)
{
    m_thingTemplate = thing;
    m_objectName = thing->Get_Name();
}

void MapObject::Set_Name(Utf8String name)
{
    m_objectName = name;
}

WaypointID MapObject::Get_Waypoint_ID()
{
    return static_cast<WaypointID>(Get_Properties()->Get_Int(g_waypointIDKey));
}

Utf8String MapObject::Get_Waypoint_Name()
{
    return Get_Properties()->Get_AsciiString(g_waypointNameKey);
}

void MapObject::Set_Waypoint_ID(WaypointID i)
{
    Get_Properties()->Set_Int(g_waypointIDKey, i);
}

void MapObject::Set_Waypoint_Name(Utf8String n)
{
    Get_Properties()->Set_AsciiString(g_waypointIDKey, n);
}

// untested, worldbuilder only
int MapObject::Count_Map_Objects_With_Owner(const Utf8String &n)
{
    int count = 0;

    for (MapObject *m = Get_First_Map_Object(); m; m = m->Get_Next()) {
        if (m->Get_Properties()->Get_AsciiString(g_originalOwnerKey) == n) {
            count++;
        }
    }

    return count;
}

const ThingTemplate *MapObject::Get_Thing_Template()
{
    if (m_thingTemplate) {
        return static_cast<const ThingTemplate *>(m_thingTemplate->Get_Final_Override());
    } else {
        return nullptr;
    }
}
