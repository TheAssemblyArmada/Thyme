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
#include "terrainlogic.h"
#include "ai.h"
#include "aipathfind.h"
#include "bodymodule.h"
#include "bridgebehavior.h"
#include "bridgetowerbehavior.h"
#include "cachedfileinputstream.h"
#include "damage.h"
#include "gamelogic.h"
#include "ghostobject.h"
#include "globaldata.h"
#include "mapobject.h"
#include "object.h"
#include "plane.h"
#include "polygontrigger.h"
#include "radar.h"
#include "simpleobjectiterator.h"
#include "staticnamekey.h"
#include "terrainroads.h"
#include "terrainvisual.h"
#include "thingfactory.h"
#include "tri.h"
#include "view.h"
#include "worldheightmap.h"

#ifndef GAME_DLL
TerrainLogic *g_theTerrainLogic = nullptr;
WaterHandle TerrainLogic::m_gridWaterHandle;
#endif

BridgeInfo::BridgeInfo() :
    bridge_width(0.0f),
    bridge_index(0),
    cur_damage_state(BODY_PRISTINE),
    bridge_object_id(INVALID_OBJECT_ID),
    is_destroyed(false)
{
    from.Zero();
    to.Zero();
    from_left.Zero();
    from_right.Zero();
    to_left.Zero();
    to_right.Zero();

    for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
        tower_object_id[i] = INVALID_OBJECT_ID;
    }
}

Waypoint::Waypoint(WaypointID id,
    Utf8String name,
    const Coord3D *loc,
    Utf8String label1,
    Utf8String label2,
    Utf8String label3,
    bool bidirectional) :
    m_id(id),
    m_name(name),
    m_location(*loc),
    m_next(nullptr),
    m_numLinks(0),
    m_pathLabel1(label1),
    m_pathLabel2(label2),
    m_pathLabel3(label3),
    m_pathIsBiDirectional(bidirectional)
{
    for (int i = 0; i < MAX_LINKS; i++) {
        m_links[i] = nullptr;
    }
}

Waypoint::~Waypoint() {}

Object *Bridge::Create_Tower(
    Coord3D *world_pos, BridgeTowerType tower_type, const ThingTemplate *tower_template, Object *bridge)
{
    if (tower_template != nullptr && bridge != nullptr) {
        Object *tower = g_theThingFactory->New_Object(tower_template, bridge->Get_Team(), OBJECT_STATUS_MASK_NONE);

        switch (tower_type) {
            case BRIDGE_TOWER_FROM_LEFT:
            case BRIDGE_TOWER_FROM_RIGHT: {
                float angle = bridge->Get_Orientation() + GAMEMATH_PI;
                tower->Set_Position(world_pos);
                tower->Set_Orientation(angle);
                BridgeBehaviorInterface *bridge_behavior = BridgeBehavior::Get_Bridge_Behavior_Interface_From_Object(bridge);
                captainslog_dbgassert(
                    bridge_behavior != nullptr, "Bridge::Create_Tower - no 'BridgeBehaviorInterface' found");

                if (bridge_behavior != nullptr) {
                    bridge_behavior->Set_Tower(tower_type, tower);
                }

                BridgeTowerBehaviorInterface *bridge_tower_behavior =
                    BridgeTowerBehavior::Get_Bridge_Tower_Behavior_Interface_From_Object(tower);
                captainslog_dbgassert(
                    bridge_tower_behavior != nullptr, "Bridge::Create_Tower - no 'BridgeTowerBehaviorInterface' found");

                if (bridge_tower_behavior != nullptr) {
                    bridge_tower_behavior->Set_Bridge(bridge);
                    bridge_tower_behavior->Set_Tower_Type(tower_type);
                }

                if (bridge->Get_Body_Module()->Is_Indestructible()) {
                    tower->Get_Body_Module()->Set_Indestructible(true);
                }

                return tower;
            }
            case BRIDGE_TOWER_TO_LEFT:
            case BRIDGE_TOWER_TO_RIGHT: {
                float angle = bridge->Get_Orientation();
                tower->Set_Position(world_pos);
                tower->Set_Orientation(angle);
                BridgeBehaviorInterface *bridge_behavior = BridgeBehavior::Get_Bridge_Behavior_Interface_From_Object(bridge);
                captainslog_dbgassert(
                    bridge_behavior != nullptr, "Bridge::Create_Tower - no 'BridgeBehaviorInterface' found");

                if (bridge_behavior != nullptr) {
                    bridge_behavior->Set_Tower(tower_type, tower);
                }

                BridgeTowerBehaviorInterface *bridge_tower_behavior =
                    BridgeTowerBehavior::Get_Bridge_Tower_Behavior_Interface_From_Object(tower);
                captainslog_dbgassert(
                    bridge_tower_behavior != nullptr, "Bridge::Create_Tower - no 'BridgeTowerBehaviorInterface' found");

                if (bridge_tower_behavior != nullptr) {
                    bridge_tower_behavior->Set_Bridge(bridge);
                    bridge_tower_behavior->Set_Tower_Type(tower_type);
                }

                if (bridge->Get_Body_Module()->Is_Indestructible()) {
                    tower->Get_Body_Module()->Set_Indestructible(true);
                }

                return tower;
            }
            default: {
                captainslog_dbgassert(false, "Bridge::Create_Tower - Unknown bridge tower type '%d'", tower_type);
                return nullptr;
            }
        }
    } else {
        captainslog_dbgassert(false, "Bridge::Create_Tower(): Invalid params");
        return nullptr;
    }
}

Bridge::Bridge(BridgeInfo &info, Dict *props, Utf8String bridge_template_name) : m_bridgeInfo(info)
{
    m_templateName = bridge_template_name;
    m_bounds.lo.x = m_bridgeInfo.from_left.x;
    m_bounds.lo.y = m_bridgeInfo.from_left.y;
    m_bounds.hi = m_bounds.lo;

    if (m_bounds.lo.x > m_bridgeInfo.from_right.x) {
        m_bounds.lo.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.from_right.y) {
        m_bounds.lo.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.from_right.x) {
        m_bounds.hi.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.from_right.y) {
        m_bounds.hi.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_left.x) {
        m_bounds.lo.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_left.y) {
        m_bounds.lo.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_left.x) {
        m_bounds.hi.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_left.y) {
        m_bounds.hi.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_right.x) {
        m_bounds.lo.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_right.y) {
        m_bounds.lo.y = m_bridgeInfo.to_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_right.x) {
        m_bounds.hi.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_right.y) {
        m_bounds.hi.y = m_bridgeInfo.to_right.y;
    }

    m_bridgeInfo.cur_damage_state = BODY_PRISTINE;
    static ThingTemplate *genericBridgeTemplate = g_theThingFactory->Find_Template("GenericBridge", true);

    if (genericBridgeTemplate != nullptr) {
        Object *bridge = g_theThingFactory->New_Object(genericBridgeTemplate, nullptr, OBJECT_STATUS_MASK_NONE);
        Coord3D pos = (m_bridgeInfo.from_left + m_bridgeInfo.to_right) / 2.0f;
        bridge->Set_Position(&pos);
        m_bridgeInfo.bridge_object_id = bridge->Get_ID();
        bridge->Update_Obj_Values_From_Map_Properties(props);
        Coord2D c;
        c.x = m_bridgeInfo.to_left.x - m_bridgeInfo.from_left.x;
        c.y = m_bridgeInfo.to_left.y - m_bridgeInfo.from_left.y;
        bridge->Set_Orientation(c.To_Angle());

        if (g_theTerrainRoads->Find_Bridge(bridge_template_name)) {
            m_next = nullptr;
        } else {
            captainslog_debug("*** Bridge Template Not Found '%s'.", bridge_template_name.Str());
        }
    } else {
        captainslog_debug("*** GenericBridge template not found.");
    }
}

Bridge::Bridge(Object *obj)
{
    m_templateName = obj->Get_Template()->Get_Name();

    if (obj->Get_Geometry_Info().Get_Type() != GEOMETRY_BOX) {
        captainslog_debug("Bridges need to be rectangles.");
    }

    const Coord3D *pos = obj->Get_Position();
    float orientation = obj->Get_Orientation();
    float major_radius = obj->Get_Geometry_Info().Get_Major_Radius();
    float minor_radius = obj->Get_Geometry_Info().Get_Minor_Radius();
    m_bridgeInfo.bridge_width = 2.0f * minor_radius;
    float cos = GameMath::Cos(orientation);
    float sin = GameMath::Sin(orientation);

    m_bridgeInfo.from_left.Set(
        pos->x - major_radius * cos - minor_radius * sin, minor_radius * cos + pos->y - major_radius * sin, pos->z);
    m_bridgeInfo.to_left.Set(
        major_radius * cos + pos->x - minor_radius * sin, minor_radius * cos + pos->y + major_radius * sin, pos->z);
    m_bridgeInfo.from_right.Set(
        pos->x - major_radius * cos + minor_radius * sin, pos->y - minor_radius * cos - major_radius * sin, pos->z);
    m_bridgeInfo.to_right.Set(
        major_radius * cos + pos->x + minor_radius * sin, pos->y - minor_radius * cos + major_radius * sin, pos->z);

    m_bridgeInfo.from = (m_bridgeInfo.from_left + m_bridgeInfo.from_right) / 2.0f;
    m_bridgeInfo.to = (m_bridgeInfo.to_left + m_bridgeInfo.to_right) / 2.0f;

    m_bounds.lo.x = m_bridgeInfo.from_left.x;
    m_bounds.lo.y = m_bridgeInfo.from_left.y;
    m_bounds.hi = m_bounds.lo;

    if (m_bounds.lo.x > m_bridgeInfo.from_right.x) {
        m_bounds.lo.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.from_right.y) {
        m_bounds.lo.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.from_right.x) {
        m_bounds.hi.x = m_bridgeInfo.from_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.from_right.y) {
        m_bounds.hi.y = m_bridgeInfo.from_right.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_left.x) {
        m_bounds.lo.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_left.y) {
        m_bounds.lo.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_left.x) {
        m_bounds.hi.x = m_bridgeInfo.to_left.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_left.y) {
        m_bounds.hi.y = m_bridgeInfo.to_left.y;
    }

    if (m_bounds.lo.x > m_bridgeInfo.to_right.x) {
        m_bounds.lo.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.lo.y > m_bridgeInfo.to_right.y) {
        m_bounds.lo.y = m_bridgeInfo.to_right.y;
    }

    if (m_bounds.hi.x < m_bridgeInfo.to_right.x) {
        m_bounds.hi.x = m_bridgeInfo.to_right.x;
    }

    if (m_bounds.hi.y < m_bridgeInfo.to_right.y) {
        m_bounds.hi.y = m_bridgeInfo.to_right.y;
    }

    m_bridgeInfo.cur_damage_state = BODY_PRISTINE;
    m_bridgeInfo.bridge_object_id = obj->Get_ID();
    Utf8String bridge_template_name = obj->Get_Template()->Get_Name();
    TerrainRoadType *bridge = g_theTerrainRoads->Find_Bridge(bridge_template_name);

    if (bridge != nullptr) {
        Coord2D c;
        c.x = m_bridgeInfo.to_left.x - m_bridgeInfo.to_right.x;
        c.y = m_bridgeInfo.to_left.y - m_bridgeInfo.to_right.y;
        c.Normalize();
        Coord3D coords[BRIDGE_MAX_TOWERS];
        coords[0] = m_bridgeInfo.from_left;
        coords[1] = m_bridgeInfo.from_right;
        coords[2] = m_bridgeInfo.to_left;
        coords[3] = m_bridgeInfo.to_right;
        float radius = 5.0f;

        for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
            ThingTemplate *tower_template =
                g_theThingFactory->Find_Template(bridge->Get_Tower_Object_Name(static_cast<BridgeTowerType>(i)), true);

            if (tower_template != nullptr) {
                radius = tower_template->Get_Template_Geometry_Info().Get_Major_Radius();
            }

            Coord3D world_pos = coords[i];

            switch (i) {
                case BRIDGE_TOWER_FROM_LEFT:
                case BRIDGE_TOWER_TO_LEFT:
                    world_pos.x = c.x * radius + world_pos.x;
                    world_pos.y = c.y * radius + world_pos.y;
                    break;
                case BRIDGE_TOWER_FROM_RIGHT:
                case BRIDGE_TOWER_TO_RIGHT:
                    world_pos.x = world_pos.x - c.x * radius;
                    world_pos.y = world_pos.y - c.y * radius;
                    break;
                default:
                    break;
            }

            Object *tower = Create_Tower(&world_pos, static_cast<BridgeTowerType>(i), tower_template, obj);

            if (tower != nullptr) {
                m_bridgeInfo.tower_object_id[i] = tower->Get_ID();
            }

            m_next = nullptr;
        }
    } else {
        captainslog_debug("*** Bridge Template Not Found '%s'.", bridge_template_name.Str());
    }
}

Bridge::~Bridge() {}

bool Bridge::Is_Point_On_Bridge(const Coord3D *loc)
{
    if (loc->x < m_bounds.lo.x) {
        return false;
    }

    if (loc->x > m_bounds.hi.x) {
        return false;
    }

    if (loc->y < m_bounds.lo.y) {
        return false;
    }

    if (loc->y > m_bounds.hi.y) {
        return false;
    }

    Vector3 loc_vec(loc->x, loc->y, loc->z);
    Vector3 from_left_vec(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 from_right_vec(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 to_left_vec(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);
    Vector3 to_right_vec(m_bridgeInfo.to_right.x, m_bridgeInfo.to_right.y, m_bridgeInfo.to_right.z);
    unsigned char flags;
    return Point_In_Triangle_2D(from_left_vec, from_right_vec, to_left_vec, loc_vec, 0, 1, flags)
        || Point_In_Triangle_2D(from_right_vec, to_left_vec, to_right_vec, loc_vec, 0, 1, flags);
}

bool Line_In_Region(const Coord2D *p1, const Coord2D *p2, const Region2D *clip_region)
{
    float c_l_x = clip_region->lo.x;
    float c_h_x = clip_region->hi.x;
    float c_l_y = clip_region->lo.y;
    float c_h_y = clip_region->hi.y;
    float p1_x = p1->x;
    float p1_y = p1->y;
    float p2_x = p2->x;
    float p2_y = p2->y;
    int clipCode1 = 0;

    if (p1->x >= c_l_x) {
        if (p1_x > c_h_x) {
            clipCode1 = 2;
        }
    } else {
        clipCode1 = 1;
    }

    if (p1_y >= c_l_y) {
        if (p1_y > c_h_y) {
            clipCode1 |= 4u;
        }
    } else {
        clipCode1 |= 8u;
    }

    int clipCode2 = 0;

    if (p2_x >= c_l_x) {
        if (p2_x > c_h_x) {
            clipCode2 = 2;
        }
    } else {
        clipCode2 = 1;
    }

    if (p2_y >= c_l_y) {
        if (p2_y > c_h_y) {
            clipCode2 |= 4u;
        }
    } else {
        clipCode2 |= 8u;
    }

    if ((clipCode2 | clipCode1) == 0) {
        return true;
    }

    if ((clipCode2 & clipCode1) != 0) {
        return false;
    }

    if (clipCode1) {
        if ((clipCode1 & 8) != 0) {
            if (p2_y - p1_y == 0.0f) {
                return false;
            }

            p1_x = (p2_x - p1_x) * (c_l_y - p1_y) / (p2_y - p1_y) + p1_x;
            p1_y = c_l_y;
        } else if ((clipCode1 & 4) != 0) {
            if (p2_y - p1_y == 0.0f) {
                return false;
            }

            p1_x = (p2_x - p1_x) * (c_h_y - p1_y) / (p2_y - p1_y) + p1_x;
            p1_y = c_h_y;
        }

        if (p1_x <= c_h_x) {
            if (p1_x < c_l_x) {
                if (p2_x - p1_x == 0.0f) {
                    return false;
                }

                p1_y = (p2_y - p1_y) * (c_l_x - p1_x) / (p2_x - p1_x) + p1_y;
                p1_x = c_l_x;
            }
        } else {
            if (p2_x - p1_x == 0.0f) {
                return false;
            }

            p1_y = (p2_y - p1_y) * (c_h_x - p1_x) / (p2_x - p1_x) + p1_y;
            p1_x = c_h_x;
        }
    }
    if (clipCode2) {
        if ((clipCode2 & 8) != 0) {
            if (p2_y - p1_y == 0.0f) {
                return false;
            }

            p2_x = (p2_x - p1_x) * (c_l_y - p2_y) / (p2_y - p1_y) + p2_x;
            p2_y = c_l_y;
        } else if ((clipCode2 & 4) != 0) {
            if (p2_y - p1_y == 0.0f) {
                return false;
            }

            p2_x = (p2_x - p1_x) * (c_h_y - p2_y) / (p2_y - p1_y) + p2_x;
            p2_y = c_h_y;
        }
        if (p2_x <= c_h_x) {
            if (p2_x < c_l_x) {
                if (p2_x - p1_x == 0.0f) {
                    return false;
                }

                p2_y = (p2_y - p1_y) * (c_l_x - p2_x) / (p2_x - p1_x) + p2_y;
                p2_x = c_l_x;
            }
        } else {
            if (p2_x - p1_x == 0.0f) {
                return 0;
            }

            p2_y = (p2_y - p1_y) * (c_h_x - p2_x) / (p2_x - p1_x) + p2_y;
            p2_x = c_h_x;
        }
    }

    return p1_x >= c_l_x && p1_x <= c_h_x && p1_y >= c_l_y && p1_y <= c_h_y && p2_x >= c_l_x && p2_x <= c_h_x
        && p2_y >= c_l_y && p2_y <= c_h_y;
}

bool Bridge::Is_Cell_On_End(const Region2D *cell)
{
    Coord3D c = m_bridgeInfo.from_right - m_bridgeInfo.from_left;
    c.Normalize();
    c.x *= 10.0f;
    c.y *= 10.0f;

    Coord3D from_left = m_bridgeInfo.from_left;
    from_left.x += c.x;
    from_left.y += c.y;

    Coord3D from_right = m_bridgeInfo.from_right;
    from_right.x -= c.x;
    from_right.y -= c.y;

    Coord3D to_left = m_bridgeInfo.to_left;
    to_left.x += c.x;
    to_left.y += c.y;

    Coord3D to_right = m_bridgeInfo.to_right;
    to_right.x -= c.x;
    to_right.y -= c.y;

    Coord2D p1;
    p1.x = from_left.x;
    p1.y = from_left.y;

    Coord2D p2;
    p2.x = from_right.x;
    p2.y = from_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    p1.x = to_left.x;
    p1.y = to_left.y;

    p2.x = to_right.x;
    p2.y = to_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    return false;
}

bool Bridge::Is_Cell_On_Side(const Region2D *cell)
{
    Coord3D c = m_bridgeInfo.from_right - m_bridgeInfo.from_left;
    c.Normalize();
    c.x *= 5.1f;
    c.y *= 5.1f;

    Coord3D from_left = m_bridgeInfo.from_left;
    from_left.x -= c.x;
    from_left.y -= c.y;

    Coord3D from_right = m_bridgeInfo.from_right;
    from_right.x += c.x;
    from_right.y += c.y;

    Coord3D to_left = m_bridgeInfo.to_left;
    to_left.x -= c.x;
    to_left.y -= c.y;

    Coord3D to_right = m_bridgeInfo.to_right;
    to_right.x += c.x;
    to_right.y += c.y;

    Coord2D p1;
    p1.x = from_left.x;
    p1.y = from_left.y;

    Coord2D p2;
    p2.x = to_left.x;
    p2.y = to_left.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    p1.x = from_right.x;
    p1.y = from_right.y;

    p2.x = to_right.x;
    p2.y = to_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    from_left.x -= c.x;
    from_left.y -= c.y;

    from_right.x += c.x;
    from_right.y += c.y;

    to_left.x -= c.x;
    to_left.y -= c.y;

    to_right.x += c.x;
    to_right.y += c.y;

    p1.x = from_left.x;
    p1.y = from_left.y;

    p2.x = to_left.x;
    p2.y = to_left.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    p1.x = from_right.x;
    p1.y = from_right.y;

    p2.x = to_right.x;
    p2.y = to_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    return false;
}

bool Bridge::Is_Cell_Entry_Point(const Region2D *cell)
{
    Coord3D c = m_bridgeInfo.from_right - m_bridgeInfo.from_left;
    c.Normalize();
    c.x *= 10.0f;
    c.y *= 10.0f;

    Coord3D c2 = m_bridgeInfo.to - m_bridgeInfo.from;
    c2.Normalize();
    c2.x *= 5.0f;
    c2.y *= 5.0f;

    Coord3D from_left = m_bridgeInfo.from_left;
    from_left.x -= c2.x;
    from_left.y -= c2.y;
    from_left.x += c.x;
    from_left.y += c.y;

    Coord3D from_right = m_bridgeInfo.from_right;
    from_right.x -= c2.x;
    from_right.y -= c2.y;
    from_right.x -= c.x;
    from_right.y -= c.y;

    Coord3D to_left = m_bridgeInfo.to_left;
    to_left.x += c2.x;
    to_left.y += c2.y;
    to_left.x += c.x;
    to_left.y += c.y;

    Coord3D to_right = m_bridgeInfo.to_right;
    to_right.x += c2.x;
    to_right.y += c2.y;
    to_right.x -= c.x;
    to_right.y -= c.y;

    Coord2D p1;
    p1.x = from_left.x;
    p1.y = from_left.y;

    Coord2D p2;
    p2.x = from_right.x;
    p2.y = from_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    p1.x = to_left.x;
    p1.y = to_left.y;

    p2.x = to_right.x;
    p2.y = to_right.y;

    if (Line_In_Region(&p1, &p2, cell)) {
        return true;
    }

    return false;
}

Drawable *Bridge::Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos)
{
    Vector3 point1(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 point2(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 point3(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);
    PlaneClass plane(point1, point2, point3);

    float k;
    plane.Compute_Intersection(from, to, &k);

    Vector3 point = from + ((to - from) * k);
    Coord3D p;
    p.x = point.X;
    p.y = point.Y;
    p.z = point.Z;

    if (Is_Point_On_Bridge(&p)) {
        *pos = point;
        Object *obj = g_theGameLogic->Find_Object_By_ID(m_bridgeInfo.bridge_object_id);

        if (obj != nullptr) {
            return obj->Get_Drawable();
        }
    }

    return nullptr;
}

void Bridge::Update_Damage_State()
{
    m_bridgeInfo.is_destroyed = false;

    if (m_bridgeInfo.bridge_object_id != INVALID_OBJECT_ID) {
        Object *bridge = g_theGameLogic->Find_Object_By_ID(m_bridgeInfo.bridge_object_id);

        if (bridge != nullptr) {
            BodyDamageType damage_state = bridge->Get_Body_Module()->Get_Damage_State();
            BodyDamageType cur_damage_state = m_bridgeInfo.cur_damage_state;

            if (damage_state != cur_damage_state) {
                m_bridgeInfo.cur_damage_state = damage_state;

                if (damage_state == BODY_RUBBLE) {
                    g_theAI->Get_Pathfinder()->Change_Bridge_State(m_layer, false);
                    m_bridgeInfo.is_destroyed = true;

                    for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
                        if (obj->Get_Layer() == m_layer) {
                            if (g_theTerrainLogic->Object_Interacts_With_Bridge_Layer(obj, obj->Get_Layer(), false)) {
                                DamageInfo info;
                                info.m_in.m_damageType = DAMAGE_FALLING;
                                info.m_in.m_deathType = DEATH_SPLATTED;
                                info.m_in.m_sourceID = obj->Get_ID();
                                info.m_in.m_amount = 999999.0f;
                                obj->Attempt_Damage(&info);
                            }
                        }
                    }
                }

                if (cur_damage_state == BODY_RUBBLE) {
                    BridgeBehaviorInterface *bridge_behavior =
                        BridgeBehavior::Get_Bridge_Behavior_Interface_From_Object(bridge);

                    if (bridge_behavior == nullptr || !bridge_behavior->Is_Scaffold_Present()) {
                        g_theAI->Get_Pathfinder()->Change_Bridge_State(m_layer, true);
                    }

                    m_bridgeInfo.is_destroyed = true;
                }
            }
        } else {
            m_bridgeInfo.bridge_object_id = INVALID_OBJECT_ID;
            captainslog_dbgassert(false, "Bridge object disappeared - unexpected.");
        }
    }
}

float Bridge::Get_Bridge_Height(const Coord3D *loc, Coord3D *n)
{
    Vector3 point1(m_bridgeInfo.from_left.x, m_bridgeInfo.from_left.y, m_bridgeInfo.from_left.z);
    Vector3 point2(m_bridgeInfo.from_right.x, m_bridgeInfo.from_right.y, m_bridgeInfo.from_right.z);
    Vector3 point3(m_bridgeInfo.to_left.x, m_bridgeInfo.to_left.y, m_bridgeInfo.to_left.z);
    PlaneClass plane(point1, point2, point3);

    float z = 1000.0f;
    Vector3 p0(loc->x, loc->y, 0.0f);
    Vector3 p1(loc->x, loc->y, z);
    float k;
    plane.Compute_Intersection(p0, p1, &k);

    if (n != nullptr) {
        n->x = plane.N.X;
        n->y = plane.N.Y;
        n->z = plane.N.Z;
    }

    return k * z;
}

TerrainLogic::TerrainLogic() :
    m_mapData(nullptr),
    m_mapDX(0),
    m_mapDY(0),
    m_activeBoundary(0),
    m_waypointListHead(nullptr),
    m_bridgeListHead(nullptr),
    m_bridgeDamageStatesChanged(false),
    m_waterGridEnabled(false),
    m_numWaterToUpdate(0)
{
    for (int i = 0; i < MAX_DYNAMIC_WATER; i++) {
        m_waterToUpdate[i].water_table = nullptr;
        m_waterToUpdate[i].change_per_frame = 0.0f;
        m_waterToUpdate[i].target_height = 0.0f;
        m_waterToUpdate[i].damage_amount = 0.0f;
        m_waterToUpdate[i].current_height = 0.0f;
    }
}

TerrainLogic::~TerrainLogic()
{
    Reset();
}

void TerrainLogic::Init() {}

void TerrainLogic::Reset()
{
    Delete_Waypoints();
    Delete_Bridges();
    PolygonTrigger::Delete_Triggers();
    m_numWaterToUpdate = 0;
}

bool TerrainLogic::Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const
{
    captainslog_dbgassert(false, "implement ME");
    return false;
}

float TerrainLogic::Get_Ground_Height(float x, float y, Coord3D *n) const
{
    if (n != nullptr) {
        n->Zero();
    }

    return 0.0f;
}

float TerrainLogic::Get_Layer_Height(float x, float y, PathfindLayerEnum layer, Coord3D *n, bool b) const
{
    if (n != nullptr) {
        n->Zero();
    }

    return 0.0f;
}

bool TerrainLogic::Is_Cliff_Cell(float x, float y) const
{
    return false;
}

bool TerrainLogic::Parse_Waypoint_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    return static_cast<TerrainLogic *>(user_data)->Parse_Waypoint_Data(file, info, user_data);
}

bool TerrainLogic::Parse_Waypoint_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    int count = file.Read_Int32();

    for (int i = 0; i < count; i++) {
        int id1 = file.Read_Int32();
        int id2 = file.Read_Int32();
        Add_Waypoint_Link(id1, id2);
    }

    captainslog_dbgassert(file.At_End_Of_Chunk(), "Unexpected data left over.");
    return true;
}

void TerrainLogic::Add_Waypoint(MapObject *map_obj)
{
    Coord3D loc = *map_obj->Get_Location();
    loc.z = Get_Ground_Height(loc.x, loc.y, nullptr);
    Utf8String label1;
    Utf8String label2;
    Utf8String label3;
    bool exists;
    label1 = map_obj->Get_Properties()->Get_AsciiString(g_waypointPathLabel1, &exists);
    label2 = map_obj->Get_Properties()->Get_AsciiString(g_waypointPathLabel2, &exists);
    label3 = map_obj->Get_Properties()->Get_AsciiString(g_waypointPathLabel3, &exists);
    bool bidir = map_obj->Get_Properties()->Get_Bool(g_waypointPathBiDirectional, &exists);
    captainslog_dbgassert(map_obj->Is_Waypoint(), "not a waypoint");
    Waypoint *waypoint =
        new Waypoint(map_obj->Get_Waypoint_ID(), map_obj->Get_Waypoint_Name(), &loc, label1, label2, label3, bidir);
    waypoint->Set_Next(m_waypointListHead);
    m_waypointListHead = waypoint;
}

void TerrainLogic::Add_Waypoint_Link(int id1, int id2)
{
    Waypoint *waypoint1 = nullptr;
    Waypoint *waypoint2 = nullptr;

    for (Waypoint *waypoint = Get_First_Waypoint(); waypoint != nullptr; waypoint = waypoint->Get_Next()) {
        if (waypoint->Get_ID() == id1) {
            waypoint1 = waypoint;
        }

        if (waypoint->Get_ID() == id2) {
            waypoint2 = waypoint;
        }
    }

    if (waypoint1 != nullptr && waypoint2 != nullptr && waypoint1 != waypoint2) {
        for (int i = 0; i < waypoint1->Get_Num_Links(); i++) {
            if (waypoint1->Get_Link(i) == waypoint2) {
                return;
            }
        }

        waypoint1->Set_Link(waypoint2);

        if (waypoint1->Is_Bi_Directional()) {
            for (int i = 0; i < waypoint2->Get_Num_Links(); i++) {
                if (waypoint2->Get_Link(i) == waypoint1) {
                    return;
                }
            }

            waypoint2->Set_Link(waypoint1);
        }
    }
}

void TerrainLogic::Delete_Waypoints()
{
    Waypoint *next;
    for (Waypoint *waypoint = Get_First_Waypoint(); waypoint != nullptr; waypoint = next) {
        next = waypoint->Get_Next();
        waypoint->Set_Next(nullptr);
        waypoint->Delete_Instance();
    }

    m_waypointListHead = nullptr;
}

void TerrainLogic::Add_Bridge_To_Logic(BridgeInfo *info, Dict *props, Utf8String bridge_template_name)
{
    Bridge *bridge = new Bridge(*info, props, bridge_template_name);
    bridge->Set_Next(m_bridgeListHead);
    m_bridgeListHead = bridge;
    bridge->Set_Layer(g_theAI->Get_Pathfinder()->Add_Bridge(bridge));
}

void TerrainLogic::Add_Landmark_Bridge_To_Logic(Object *obj)
{
    Bridge *bridge = new Bridge(obj);
    bridge->Set_Next(m_bridgeListHead);
    m_bridgeListHead = bridge;
    bridge->Set_Layer(g_theAI->Get_Pathfinder()->Add_Bridge(bridge));
}

Waypoint *TerrainLogic::Get_Waypoint_By_Name(Utf8String name)
{
    for (Waypoint *waypoint = Get_First_Waypoint(); waypoint != nullptr; waypoint = waypoint->Get_Next()) {
        if (name == waypoint->Get_Name()) {
            return waypoint;
        }
    }

    return nullptr;
}

Waypoint *TerrainLogic::Get_Waypoint_By_ID(WaypointID id)
{
    for (Waypoint *waypoint = Get_First_Waypoint(); waypoint != nullptr; waypoint = waypoint->Get_Next()) {
        if (waypoint->Get_ID() == id) {
            return waypoint;
        }
    }

    return nullptr;
}

Waypoint *TerrainLogic::Get_Closest_Waypoint_On_Path(const Coord3D *pos, Utf8String label)
{
    float distance = 0.0f;
    Waypoint *waypoint = nullptr;

    if (label.Is_Empty()) {
        captainslog_debug("***Warning - asking for empty path label.");
        return nullptr;
    } else {
        for (Waypoint *w = Get_First_Waypoint(); w != nullptr; w = w->Get_Next()) {
            bool found = false;

            if (label.Compare_No_Case(w->Get_Path_Label_1()) == 0) {
                found = true;
            }

            if (label.Compare_No_Case(w->Get_Path_Label_2()) == 0) {
                found = true;
            }

            if (label.Compare_No_Case(w->Get_Path_Label_3()) == 0) {
                found = true;
            }

            if (found) {
                Coord3D loc = *w->Get_Location();
                float dist = (loc.x - pos->x) * (loc.x - pos->x) + (loc.y - pos->y) * (loc.y - pos->y);

                if (waypoint != nullptr) {
                    if (dist < distance) {
                        waypoint = w;
                        distance = dist;
                    }
                } else {
                    waypoint = w;
                    distance = dist;
                }
            }
        }

        return waypoint;
    }
}

bool TerrainLogic::Is_Purpose_Of_Path(Waypoint *way, Utf8String label)
{
    if (!label.Is_Empty() && way != nullptr) {
        if (way->Get_Path_Label_1() == label) {
            return true;
        }

        if (way->Get_Path_Label_2() == label) {
            return true;
        }

        if (way->Get_Path_Label_3() == label) {
            return true;
        }

        return false;
    } else {
        captainslog_debug("***Warning - asking for empth path label.");
        return false;
    }
}

Bridge *TerrainLogic::Find_Bridge_At(const Coord3D *loc) const
{
    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if (bridge->Is_Point_On_Bridge(loc)) {
            return bridge;
        }
    }

    return nullptr;
}

Bridge *TerrainLogic::Find_Bridge_Layer_At(const Coord3D *loc, PathfindLayerEnum layer, bool b) const
{
    if (layer == LAYER_GROUND) {
        return nullptr;
    }

    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if (bridge->Get_Layer() == layer && (!b || bridge->Is_Point_On_Bridge(loc))) {
            return bridge;
        }
    }

    return nullptr;
}

PathfindLayerEnum TerrainLogic::Get_Highest_Layer_For_Destination(const Coord3D *pos, bool b)
{
    PathfindLayerEnum layer = LAYER_GROUND;

    float height = pos->z - Get_Ground_Height(pos->x, pos->y, nullptr);

    if (g_theAI->Get_Pathfinder()->Get_Wall_Height() / 2.0f < height) {
        if (g_theAI->Get_Pathfinder()->Is_Point_On_Wall(pos)) {
            float height_above_wall = pos->z - g_theAI->Get_Pathfinder()->Get_Wall_Height();

            if (height_above_wall >= 0.0f) {
                if (GameMath::Fabs(height) > GameMath::Fabs(height_above_wall)) {
                    layer = LAYER_WALLS;
                    height = height_above_wall;
                }
            }
        }
    }

    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if ((!b || bridge->Peek_Bridge_Info()->cur_damage_state != BODY_RUBBLE) && bridge->Is_Point_On_Bridge(pos)) {
            float height_above_bridge = pos->z - bridge->Get_Bridge_Height(pos, nullptr);

            if (height_above_bridge >= 0.0f) {
                if (GameMath::Fabs(height) > GameMath::Fabs(height_above_bridge)) {
                    layer = bridge->Get_Layer();
                    height = height_above_bridge;
                }
            }
        }
    }

    return layer;
}

bool TerrainLogic::Object_Interacts_With_Bridge_Layer(Object *obj, int layer, bool b) const
{
    if (layer == LAYER_GROUND) {
        return false;
    }

    if (layer == LAYER_WALLS) {
        if (obj->Get_Layer() == LAYER_WALLS) {
            return true;
        } else {
            return g_theAI->Get_Pathfinder()->Is_Point_On_Wall(obj->Get_Position());
        }
    } else {
        Bridge *bridge;
        for (bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
            if (bridge == nullptr) {
                return false;
            }

            if (bridge->Get_Layer() == layer) {
                break;
            }
        }

        bool found = false;

        if (bridge->Is_Point_On_Bridge(obj->Get_Position())) {
            found = true;
        }

        float radius = obj->Get_Geometry_Info().Get_Minor_Radius() + 5.0f;
        Region2D cell;
        cell.lo.x = obj->Get_Position()->x;
        cell.lo.y = obj->Get_Position()->y;
        cell.hi = cell.lo;
        cell.lo.x -= radius;
        cell.lo.y -= radius;
        cell.hi.x += radius;
        cell.hi.y += radius;

        if (bridge->Is_Cell_On_End(&cell)) {
            found = true;
        }

        if (found) {
            float height = bridge->Get_Bridge_Height(obj->Get_Position(), nullptr);
            return GameMath::Fabs(obj->Get_Position()->z - height) <= 10.0f
                || (!b || bridge->Peek_Bridge_Info()->cur_damage_state != BODY_RUBBLE);
        } else {
            return false;
        }
    }
}

bool TerrainLogic::Object_Interacts_With_Bridge_End(Object *obj, int layer) const
{
    if (layer == LAYER_GROUND) {
        return false;
    }

    Bridge *bridge;
    for (bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if (bridge == nullptr) {
            return false;
        }

        if (bridge->Get_Layer() == layer) {
            break;
        }
    }

    bool found = false;

    float radius = obj->Get_Geometry_Info().Get_Minor_Radius() + 5.0f;
    Region2D cell;
    cell.lo.x = obj->Get_Position()->x;
    cell.lo.y = obj->Get_Position()->y;
    cell.hi = cell.lo;
    cell.lo.x = cell.lo.x - radius;
    cell.lo.y = cell.lo.y - radius;
    cell.hi.x = cell.hi.x + radius;
    cell.hi.y = cell.hi.y + radius;

    if (bridge->Is_Cell_On_End(&cell)) {
        found = true;
    }

    if (found) {
        float height = bridge->Get_Bridge_Height(obj->Get_Position(), nullptr);
        return GameMath::Fabs(obj->Get_Position()->z - height) <= 10.0f;
    } else {
        return false;
    }
}

void TerrainLogic::Update_Bridge_Damage_States()
{
    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        bridge->Update_Damage_State();
    }

    m_bridgeDamageStatesChanged = true;
}

bool TerrainLogic::Is_Bridge_Repaired(const Object *bridge)
{
    if (bridge == nullptr) {
        return false;
    }

    ObjectID id = bridge->Get_ID();

    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if (bridge == nullptr) {
            return false;
        }

        const BridgeInfo *info = bridge->Peek_Bridge_Info();
        if (info->bridge_object_id == id) {
            return info->is_destroyed && info->cur_damage_state != BODY_RUBBLE;
        }
    }

    return false;
}

bool TerrainLogic::Is_Bridge_Broken(const Object *bridge)
{
    if (bridge == nullptr) {
        return false;
    }

    ObjectID id = bridge->Get_ID();

    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        if (bridge == nullptr) {
            return false;
        }

        const BridgeInfo *info = bridge->Peek_Bridge_Info();
        if (info->bridge_object_id == id) {
            return info->is_destroyed && info->cur_damage_state == BODY_RUBBLE;
        }
    }

    return false;
}

void TerrainLogic::Get_Bridge_Attack_Points(const Object *bridge, TBridgeAttackInfo *attack_info)
{
    ObjectID id = bridge->Get_ID();

    for (Bridge *b = Get_First_Bridge(); b != nullptr; b = b->Get_Next()) {
        const BridgeInfo *info = b->Peek_Bridge_Info();

        if (info->bridge_object_id == id) {
            Coord3D distance = info->to - info->from;
            distance.Normalize();
            float f = (info->from_right - info->from_left).Length() / 2.0f;
            attack_info->m_attackPoint1 = info->from + distance * f;
            attack_info->m_attackPoint2 = info->to - distance * f;
            return;
        }
    }

    attack_info->m_attackPoint1 = *bridge->Get_Position();
    attack_info->m_attackPoint2 = *bridge->Get_Position();
}

Drawable *TerrainLogic::Pick_Bridge(const Vector3 &from, const Vector3 &to, Vector3 *pos)
{
    Drawable *drawable = nullptr;
    Vector3 loc(0.0f, 0.0f, 0.0f);

    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = bridge->Get_Next()) {
        Vector3 v;
        Drawable *d = bridge->Pick_Bridge(from, to, &v);

        if (drawable == nullptr) {
            drawable = d;
            loc = v;
        }
    }

    *pos = loc;
    return drawable;
}

void TerrainLogic::Delete_Bridges()
{
    Bridge *next;
    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = next) {
        next = bridge->Get_Next();
        bridge->Set_Next(nullptr);
        bridge->Delete_Instance();
    }

    m_bridgeListHead = nullptr;
}

void TerrainLogic::Delete_Bridge(Bridge *bridge)
{
    if (bridge != nullptr) {
        if (m_bridgeListHead == bridge) {
            m_bridgeListHead = bridge->Get_Next();
        } else {
            for (Bridge *b = Get_First_Bridge(); b != nullptr; b = b->Get_Next()) {
                if (b->Get_Next() == bridge) {
                    b->Set_Next(bridge->Get_Next());
                    break;
                }
            }
        }

        BridgeInfo info;
        bridge->Get_Bridge_Info(&info);
        g_theAI->Get_Pathfinder()->Change_Bridge_State(bridge->Get_Layer(), false);
        Object *obj = g_theGameLogic->Find_Object_By_ID(info.bridge_object_id);

        if (obj != nullptr) {
            g_theGameLogic->Destroy_Object(obj);
        }

        bridge->Delete_Instance();
    }
}

void TerrainLogic::Set_Active_Boundary(int new_active_boundary)
{
    if (new_active_boundary >= 0 && static_cast<size_t>(new_active_boundary) < m_boundaries.size()
        && new_active_boundary != m_activeBoundary && m_boundaries[new_active_boundary].x != 0
        && m_boundaries[new_active_boundary].y != 0) {
        ShroudStatusStoreRestore shroud;
        g_thePartitionManager->Process_Entire_Pending_Undo_Shroud_Reveal_Queue();
        g_thePartitionManager->Store_Fogged_Cells(shroud, true);
        m_activeBoundary = new_active_boundary;
        g_theGhostObjectManager->Release_Partition_Data();

        for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
            obj->Friend_Prepare_For_Map_Boundary_Adjust();
        }

        g_thePartitionManager->Store_Fogged_Cells(shroud, false);
        g_thePartitionManager->Reset();
        g_thePartitionManager->Init();
        g_theRadar->New_Map(g_theTerrainLogic);
        g_thePartitionManager->Restore_Fogged_Cells(shroud, false);
        g_theGhostObjectManager->Set_Updating_Map_Boundary(true);

        for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
            obj->Friend_Notify_Of_New_Map_Boundary();
        }

        g_thePartitionManager->Restore_Fogged_Cells(shroud, true);
        g_theGhostObjectManager->Restore_Partition_Data();
        g_theGhostObjectManager->Set_Updating_Map_Boundary(false);
        g_theTacticalView->Force_Camera_Constraint_Recalc();
    }
}

void TerrainLogic::CRC_Snapshot(Xfer *xfer) {}

void TerrainLogic::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 2;
    xfer->xferVersion(&version, 2);
    int boundary = m_activeBoundary;
    xfer->xferInt(&boundary);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Set_Active_Boundary(boundary);
    }

    if (version >= 2) {
        xfer->xferInt(&m_numWaterToUpdate);

        for (int i = 0; i < m_numWaterToUpdate; i++) {
            if (xfer->Get_Mode() == XFER_SAVE) {
                int id = m_waterToUpdate[i].water_table->m_polygon->Get_ID();
                xfer->xferInt(&id);
            } else if (xfer->Get_Mode() == XFER_LOAD) {
                int id;
                xfer->xferInt(&id);
                PolygonTrigger *trigger = PolygonTrigger::Get_Polygon_Trigger_By_ID(id);
                captainslog_relassert(trigger != nullptr,
                    6,
                    "TerrainLogic::Xfer_Snapshot - Unable to find polygon trigger for water table with trigger ID '%d'",
                    id);
                m_waterToUpdate[i].water_table = trigger->Get_Water_Handle();
                captainslog_relassert(m_waterToUpdate[i].water_table != nullptr,
                    6,
                    "TerrainLogic::Xfer_Snapshot - Polygon trigger to use for water handle has no water handle!");
            }

            xfer->xferReal(&m_waterToUpdate[i].change_per_frame);
            xfer->xferReal(&m_waterToUpdate[i].target_height);
            xfer->xferReal(&m_waterToUpdate[i].damage_amount);
            xfer->xferReal(&m_waterToUpdate[i].current_height);
        }
    }
}

void TerrainLogic::Load_Post_Process()
{
    Bridge *next;
    for (Bridge *bridge = Get_First_Bridge(); bridge != nullptr; bridge = next) {
        next = bridge->Get_Next();

        if (g_theGameLogic->Find_Object_By_ID(bridge->Peek_Bridge_Info()->bridge_object_id) == nullptr) {
            Delete_Bridge(bridge);
        }
    }
}

bool TerrainLogic::Load_Map(Utf8String filename, bool query)
{
    if (filename.Is_Empty()) {
        return false;
    }

    m_filenameString = filename;

    for (MapObject *obj = MapObject::Get_First_Map_Object(); obj != nullptr; obj = obj->Get_Next()) {
        if (obj->Is_Waypoint()) {
            Add_Waypoint(obj);
        }
    }

    CachedFileInputStream stream;

    if (stream.Open(m_filenameString)) {
        stream.Absolute_Seek(0);
        DataChunkInput input(&stream);

        if (input.Is_Valid_File()) {
            input.Register_Parser("WaypointsList", Utf8String::s_emptyString, Parse_Waypoint_Data_Chunk, nullptr);

            if (!input.Parse(this)) {
                captainslog_dbgassert(false, "Unable to read waypoint info.");
                return false;
            }
        }
    }

    if (!query) {
        g_theTerrainVisual->Load(Get_Source_Filename());
    }

    return true;
}

void TerrainLogic::New_Map(bool b)
{
    for (Waypoint *wp = m_waypointListHead; wp != nullptr; wp = wp->Get_Next()) {
        const Coord3D *loc = wp->Get_Location();
        wp->Set_Height(Get_Ground_Height(loc->x, loc->y, nullptr));
    }

    Enable_Water_Grid(Get_Waypoint_By_Name("WaveGuide1") != nullptr);
}

void TerrainLogic::Enable_Water_Grid(bool enable)
{
    m_waterGridEnabled = enable;

    if (!enable) {
        g_theTerrainVisual->Enable_Water_Grid(enable);
        return;
    }

    int index = -1;
    for (int i = 0; i < 4; i++) {
        if (!g_theWriteableGlobalData->m_mapName.Compare_No_Case(
                g_theWriteableGlobalData->m_vertexWaterAvailableMaps[i].Str())) {
            index = i;
        }

        Utf8String mapfile;
        Utf8String vertexwaterfile;
        const char *filename = strrchr(g_theWriteableGlobalData->m_mapName.Str(), '\\');

        if (filename != nullptr) {
            mapfile.Set(filename);
        } else {
            mapfile = g_theWriteableGlobalData->m_mapName;
        }

        filename = strrchr(g_theWriteableGlobalData->m_vertexWaterAvailableMaps[i].Str(), '\\');

        if (filename != nullptr) {
            vertexwaterfile.Set(filename);
        } else {
            vertexwaterfile = g_theWriteableGlobalData->m_mapName;
        }

        if (!mapfile.Compare_No_Case(vertexwaterfile.Str())) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        g_theTerrainVisual->Set_Water_Grid_Height_Clamps(nullptr,
            g_theWriteableGlobalData->m_vertexWaterHeightClampLow[index],
            g_theWriteableGlobalData->m_vertexWaterHeightClampHigh[index]);
        g_theTerrainVisual->Set_Water_Transform(nullptr);
        g_theTerrainVisual->Set_Water_Grid_Resolution(nullptr,
            g_theWriteableGlobalData->m_vertexWaterXGridCells[index],
            g_theWriteableGlobalData->m_vertexWaterYGridCells[index],
            g_theWriteableGlobalData->m_vertexWaterGridSize[index]);
        g_theTerrainVisual->Set_Water_Attenuation_Factors(nullptr,
            g_theWriteableGlobalData->m_vertexWaterAttenuationA[index],
            g_theWriteableGlobalData->m_vertexWaterAttenuationB[index],
            g_theWriteableGlobalData->m_vertexWaterAttenuationC[index],
            g_theWriteableGlobalData->m_vertexWaterAttenuationRange[index]);
        g_theTerrainVisual->Enable_Water_Grid(enable);
        return;
    }

    captainslog_dbgassert(false,
        "!!!!!! Deformable water won't work because there was no group of vertex water data defined in GameData.INI for "
        "this map name '%s' !!!!!! (C. Day)",
        g_theWriteableGlobalData->m_mapName.Str());
}

Coord3D TerrainLogic::Find_Closest_Edge_Point(const Coord3D *pos) const
{
    Region3D region;
    Get_Extent(&region);
    float points[4];
    points[0] = GameMath::Fabs(pos->y - region.lo.y);
    points[1] = GameMath::Fabs(pos->x - region.hi.x);
    points[2] = GameMath::Fabs(pos->y - region.hi.y);
    points[3] = GameMath::Fabs(pos->x - region.lo.x);
    float min = points[0];
    int index = 0;

    for (int i = 1; i < 4; i++) {
        if (points[i] < min) {
            min = points[i];
            index = i;
        }
    }

    Coord3D c;
    c.x = pos->x;
    c.y = pos->y;

    if (index == 0) {
        c.y = region.lo.y;
        c.z = Get_Ground_Height(c.x, region.lo.y, nullptr);
    } else if (index == 1) {
        c.x = region.hi.x;
        c.z = Get_Ground_Height(region.hi.x, c.y, nullptr);
    } else if (index == 2) {
        c.y = region.hi.y;
        c.z = Get_Ground_Height(c.x, region.hi.y, nullptr);
    } else if (index == 3) {
        c.x = region.lo.x;
        c.z = Get_Ground_Height(region.lo.x, c.y, nullptr);
    }

    return c;
}

Coord3D TerrainLogic::Find_Farthest_Edge_Point(const Coord3D *pos) const
{
    Region3D region;
    Get_Extent(&region);
    Coord3D c;

    if (region.Width() / 2.0f <= pos->x) {
        c.x = region.lo.x;
    } else {
        c.x = region.hi.x;
    }

    if (region.Height() / 2.0f <= pos->y) {
        c.y = region.lo.y;
    } else {
        c.y = region.hi.y;
    }

    c.z = Get_Ground_Height(c.x, c.y, nullptr);
    return c;
}

void Make_Align_To_Normal_Matrix(float angle, const Coord3D &pos, const Coord3D &n, Matrix3D &tm)
{
    Coord3D axis = n;
    Coord3D rotated_axis;
    rotated_axis.x = GameMath::Cos(angle);
    rotated_axis.y = GameMath::Sin(angle);
    rotated_axis.z = 0.0f;

    if (axis.z != 0.0f) {
        rotated_axis.z = -(rotated_axis.x * axis.x + rotated_axis.y * axis.y) / axis.z;
        rotated_axis.Normalize();
    }

    captainslog_dbgassert(
        GameMath::Fabs(rotated_axis.x * axis.x + rotated_axis.y * axis.y + rotated_axis.z * axis.z) < 0.0001f,
        "dot is not zero (%f)",
        GameMath::Fabs(rotated_axis.x * axis.x + rotated_axis.y * axis.y + rotated_axis.z * axis.z));
    Coord3D perpendicular_axis;
    Coord3D::Cross_Product(&axis, &rotated_axis, &perpendicular_axis);
    perpendicular_axis.Normalize();
    tm.Set(rotated_axis.x,
        perpendicular_axis.x,
        axis.x,
        pos.x,
        rotated_axis.y,
        perpendicular_axis.y,
        axis.y,
        pos.y,
        rotated_axis.z,
        perpendicular_axis.z,
        axis.z,
        pos.z);
}

PathfindLayerEnum TerrainLogic::Align_On_Terrain(float angle, const Coord3D &pos, bool stick_to_ground, Matrix3D &mtx)
{
    PathfindLayerEnum layer = Get_Layer_For_Destination(&pos);
    Coord3D c;
    float height = Get_Layer_Height(pos.x, pos.y, layer, &c, true);

    if (layer != LAYER_GROUND) {
        height += 2.5f;
    }

    Make_Align_To_Normal_Matrix(angle, pos, c, mtx);

    if (stick_to_ground) {
        mtx.Set_Z_Translation(height);
    }

    return layer;
}

bool TerrainLogic::Is_Underwater(float x, float y, float *waterz, float *groundz)
{
    const WaterHandle *handle = Get_Water_Handle(x, y);

    if (handle != nullptr) {
        float water_height = 0.0f;

        if (handle == &m_gridWaterHandle) {
            g_theTerrainVisual->Get_Water_Grid_Height(x, y, &water_height);
        } else {
            water_height = Get_Water_Height(handle);
        }

        if (waterz != nullptr) {
            *waterz = water_height;
        }

        float ground_height = Get_Ground_Height(x, y, nullptr);

        if (groundz != nullptr) {
            *groundz = ground_height;
        }

        return ground_height < water_height;
    } else {
        if (groundz != nullptr) {
            *groundz = Get_Ground_Height(x, y, nullptr);
        }

        return false;
    }
}

PathfindLayerEnum TerrainLogic::Get_Layer_For_Destination(const Coord3D *pos)
{
    Bridge *bridge = Get_First_Bridge();
    PathfindLayerEnum layer = LAYER_GROUND;
    float height = GameMath::Fabs(pos->z - Get_Ground_Height(pos->x, pos->y, nullptr));
    if (g_theAI->Get_Pathfinder()->Get_Wall_Height() / 2.0f < height) {
        if (g_theAI->Get_Pathfinder()->Is_Point_On_Wall(pos)) {
            float wall_height = GameMath::Fabs(pos->z - g_theAI->Get_Pathfinder()->Get_Wall_Height());

            if (wall_height < height) {
                layer = LAYER_WALLS;
                height = wall_height;
            }
        }
    }

    while (bridge != nullptr) {
        if (bridge->Is_Point_On_Bridge(pos)) {
            float bridge_height = GameMath::Fabs(pos->z - bridge->Get_Bridge_Height(pos, nullptr));

            if (bridge_height < height) {
                layer = bridge->Get_Layer();
                height = bridge_height;
            }
        }

        bridge = bridge->Get_Next();
    }

    return layer;
}

const WaterHandle *TerrainLogic::Get_Water_Handle(float x, float y)
{
    const WaterHandle *handle = nullptr;
    float z = 0.0f;
    ICoord3D point;
    point.x = GameMath::Fast_To_Int_Floor(x + 0.5f);
    point.y = GameMath::Fast_To_Int_Floor(y + 0.5f);
    point.z = 0.0f;

    for (PolygonTrigger *trigger = PolygonTrigger::Get_First_Polygon_Trigger(); trigger != nullptr;
         trigger = trigger->Get_Next()) {
        if (trigger->Is_Water_Area() && trigger->Point_In_Trigger(point) && trigger->Get_Point(0)->z >= z) {
            z = trigger->Get_Point(0)->z;
            handle = trigger->Get_Water_Handle();
        }
    }

    float height;

    if (g_theTerrainVisual->Get_Water_Grid_Height(x, y, &height) && height >= z) {
        return &m_gridWaterHandle;
    }

    return handle;
}

const WaterHandle *TerrainLogic::Get_Water_Handle_By_Name(Utf8String name)
{
    if (!name.Compare("Water Grid")) {
        return &m_gridWaterHandle;
    } else {
        for (PolygonTrigger *trigger = PolygonTrigger::Get_First_Polygon_Trigger(); trigger != nullptr;
             trigger = trigger->Get_Next()) {
            if (!trigger->Get_Trigger_Name().Compare(name) && trigger->Is_Water_Area()) {
                return trigger->Get_Water_Handle();
            }
        }
    }

    return nullptr;
}

float TerrainLogic::Get_Water_Height(const WaterHandle *water)
{
    if (water == nullptr) {
        return 0.0f;
    }

    if (water == &m_gridWaterHandle) {
        captainslog_dbgassert(
            false, "TerrainLogic::Get_Water_Height( WaterHandle *water ) - water is a grid handle, cannot make this query");
        return 0.0f;
    } else {
        captainslog_dbgassert(water->m_polygon != nullptr, "Get_Water_Height: polygon trigger in water handle is NULL");
        return water->m_polygon->Get_Point(0)->z;
    }
}

void TerrainLogic::Set_Water_Height(const WaterHandle *water, float height, float damage_amount, bool force_pathfind_update)
{
    if (water != nullptr) {
        float z = 0.0f;

        if (water == &m_gridWaterHandle) {
            Matrix3D tm;
            g_theTerrainVisual->Get_Water_Transform(water, &tm);
            z = tm.Get_Z_Translation();
            tm.Set_Z_Translation(height);
            g_theTerrainVisual->Set_Water_Transform(&tm);
        } else {
            z = Get_Water_Height(water);
            int count = water->m_polygon->Get_Num_Points();

            for (int i = 0; i < count; i++) {
                ICoord3D point = *water->m_polygon->Get_Point(i);
                point.z = height;
                water->m_polygon->Set_Point(point, i);
            }

            height = Get_Water_Height(water);
        }

        Region3D region;
        Find_Axis_Aligned_Bounding_Rect(water, &region);

        if (force_pathfind_update || z != height) {
            g_theAI->Get_Pathfinder()->Force_Map_Recalculation();
        }

        if (damage_amount > 0.0f && height > z) {
            Coord3D pos;
            pos.x = region.Width() / 2.0f + region.lo.x;
            pos.y = region.Height() / 2.0f + region.lo.y;
            pos.z = 0.0f;
            float f = GameMath::Sqrt(region.Height() * region.Height() + region.Width() * region.Width());
            SimpleObjectIterator *iter =
                g_thePartitionManager->Iterate_Objects_In_Range(&pos, f, FROM_CENTER_2D, nullptr, ITER_FASTEST);
            MemoryPoolObjectHolder holder(iter);

            for (Object *obj = iter->First(); obj != nullptr; obj = iter->Next()) {
                const Coord3D *c = obj->Get_Position();
                if (Is_Underwater(c->x, c->y, nullptr, nullptr)) {
                    DamageInfo info;
                    info.m_in.m_damageType = DAMAGE_WATER;
                    info.m_in.m_deathType = DEATH_NORMAL;
                    info.m_in.m_sourceID = INVALID_OBJECT_ID;
                    info.m_in.m_amount = damage_amount;
                    obj->Attempt_Damage(&info);
                }
            }
        }
    }
}

void TerrainLogic::Find_Axis_Aligned_Bounding_Rect(const WaterHandle *water, Region3D *region)
{
    if (water != nullptr && region != nullptr) {
        Region3D extent;
        Get_Extent(&extent);
        region->lo.x = extent.hi.x + 99999.898f;
        region->lo.y = extent.hi.y + 99999.898f;
        region->hi.x = extent.lo.x - 99999.898f;
        region->hi.y = extent.lo.y - 99999.898f;

        if (water == &m_gridWaterHandle) {
            float grid_cells_x;
            float grid_cells_y;
            float cell_size;
            g_theTerrainVisual->Get_Water_Grid_Resolution(water, &grid_cells_x, &grid_cells_y, &cell_size);
            ICoord3D points[4];
            points[0].x = 0;
            points[0].y = 0;
            points[1].x = grid_cells_x * cell_size;
            points[1].y = 0;
            points[2].x = points[1].x;
            points[2].y = grid_cells_y * cell_size;
            points[3].x = 0;
            points[3].y = points[2].y;
            Vector3 tp;
            Matrix3D tm;
            g_theTerrainVisual->Get_Water_Transform(water, &tm);

            for (int i = 0; i < 4; i++) {
                tp.Set(points[i].x, points[i].y, points[i].z);
                Matrix3D::Transform_Vector(tm, tp, &tp);

                if (tp.X < region->lo.x) {
                    region->lo.x = tp.X;
                }

                if (tp.X > region->hi.x) {
                    region->hi.x = tp.X;
                }

                if (tp.Y < region->lo.y) {
                    region->lo.y = tp.Y;
                }

                if (tp.Y > region->hi.y) {
                    region->hi.y = tp.Y;
                }
            }
        } else {
            int count = water->m_polygon->Get_Num_Points();

            for (int i = 0; i < count; i++) {
                ICoord3D *point = water->m_polygon->Get_Point(i);

                if (point->x < region->lo.x) {
                    region->lo.x = point->x;
                }
                if (point->x > region->hi.x) {
                    region->hi.x = point->x;
                }
                if (point->y < region->lo.y) {
                    region->lo.y = point->y;
                }
                if (point->y > region->hi.y) {
                    region->hi.y = point->y;
                }
                if (point->z < region->lo.z) {
                    region->lo.z = point->z;
                }
                if (point->z > region->hi.z) {
                    region->hi.z = point->z;
                }
            }
        }
    }
}

void TerrainLogic::Change_Water_Height_Over_Time(
    const WaterHandle *water, float final_height, float transition_time_in_seconds, float damage_amount)
{
    if (m_numWaterToUpdate < MAX_DYNAMIC_WATER) {
        if (water != nullptr) {
            for (int i = 0; i < m_numWaterToUpdate; i++) {
                if (m_waterToUpdate[i].water_table == water) {
                    m_numWaterToUpdate--;
                    m_waterToUpdate[i] = m_waterToUpdate[m_numWaterToUpdate];
                    i--;
                }
            }

            float height = Get_Water_Height(water);
            m_waterToUpdate[m_numWaterToUpdate].water_table = water;
            m_waterToUpdate[m_numWaterToUpdate].change_per_frame =
                (final_height - height) / (30.0f * transition_time_in_seconds);
            m_waterToUpdate[m_numWaterToUpdate].target_height = final_height;
            m_waterToUpdate[m_numWaterToUpdate].damage_amount = damage_amount;
            m_waterToUpdate[m_numWaterToUpdate].current_height = height;
            m_numWaterToUpdate++;
        }
    } else {
        captainslog_dbgassert(false, "Only '%d' simultaneous water table changes are supported", MAX_DYNAMIC_WATER);
    }
}

PolygonTrigger *TerrainLogic::Get_Trigger_Area_By_Name(Utf8String name)
{
    for (PolygonTrigger *trigger = PolygonTrigger::Get_First_Polygon_Trigger(); trigger != nullptr;
         trigger = trigger->Get_Next()) {
        if (name == trigger->Get_Trigger_Name()) {
            return trigger;
        }
    }

    return nullptr;
}

void TerrainLogic::Flatten_Terrain(Object *obj)
{
    if (!obj->Get_Geometry_Info().Is_Small()) {
        const Coord3D *pos = obj->Get_Position();
        GeometryType geometry_type = obj->Get_Geometry_Info().Get_Type();

        if (geometry_type >= GEOMETRY_SPHERE) {
            if (geometry_type <= GEOMETRY_CYLINDER) {
                float major_radius = obj->Get_Geometry_Info().Get_Major_Radius();
                float major_radius_square = GameMath::Square(major_radius);
                int x_min = GameMath::Fast_To_Int_Floor((pos->x - major_radius) / 10.0f);
                int y_min = GameMath::Fast_To_Int_Floor((pos->y - major_radius) / 10.0f);
                int x_max = GameMath::Fast_To_Int_Floor((pos->x + major_radius) / 10.0f);
                int y_max = GameMath::Fast_To_Int_Floor((pos->y + major_radius) / 10.0f);
                float ground_height_sum = 0.0f;
                int ground_height_sample_count = 0;

                for (int x = x_min; x <= x_max; x++) {
                    for (int y = 0; y <= y_max; y++) {
                        Vector3 v(x * 10.0f, y * 10.0f, 0.0f);
                        bool inside = false;

                        if (GameMath::Square(v.X - pos->x) + GameMath::Square(v.Y - pos->y) < major_radius_square) {
                            inside = true;
                        }

                        if (inside) {
                            ground_height_sum += g_theTerrainLogic->Get_Ground_Height(v.X, v.Y, nullptr);
                            ground_height_sample_count++;
                        }
                    }
                }

                if (ground_height_sample_count != 0) {
                    int flattened_ground_height = GameMath::Fast_To_Int_Floor(
                        (ground_height_sum / ground_height_sample_count) / HEIGHTMAP_SCALE + 0.5f);

                    for (int x = x_min; x <= x_max; x++) {
                        for (int y = 0; y <= y_max; y++) {
                            Vector3 v(x * 10.0f, y * 10.0f, 0.0f);
                            bool inside = false;

                            if (GameMath::Square(v.X - pos->x) + GameMath::Square(v.Y - pos->y) < major_radius_square) {
                                inside = true;
                            }

                            if (inside) {
                                ICoord2D c;

                                c.x = x;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);
                            }
                        }
                    }
                }
            } else if (geometry_type == GEOMETRY_BOX) {
                float orientation = obj->Get_Orientation();
                float major_radius = obj->Get_Geometry_Info().Get_Major_Radius();
                float minor_radius = obj->Get_Geometry_Info().Get_Minor_Radius();
                float cos = GameMath::Cos(orientation);
                float sin = GameMath::Sin(orientation);

                Vector3 v1(pos->x - major_radius * cos - minor_radius * sin,
                    minor_radius * cos + pos->y - major_radius * sin,
                    0.0f);
                Vector3 v2(major_radius * cos + pos->x - minor_radius * sin,
                    minor_radius * cos + pos->y + major_radius * sin,
                    0.0f);
                Vector3 v3(major_radius * cos + pos->x + minor_radius * sin,
                    pos->y - minor_radius * cos + major_radius * sin,
                    0.0f);
                Vector3 v4(pos->x - major_radius * cos + minor_radius * sin,
                    pos->y - minor_radius * cos - major_radius * sin,
                    0.0f);

                float min_x = v1.X;

                if (v1.X > v2.X) {
                    min_x = v2.X;
                }

                if (min_x > v3.X) {
                    min_x = v3.X;
                }

                if (min_x > v4.X) {
                    min_x = v4.X;
                }

                float max_x = v1.X;

                if (v1.X < v2.X) {
                    max_x = v2.X;
                }
                if (max_x < v3.X) {
                    max_x = v3.X;
                }
                if (max_x < v4.X) {
                    max_x = v4.X;
                }

                float min_y = v1.Y;

                if (v1.Y > v2.Y) {
                    min_y = v2.Y;
                }

                if (min_y > v3.Y) {
                    min_y = v3.Y;
                }

                if (min_y > v4.Y) {
                    min_y = v4.Y;
                }

                float max_y = v1.Y;

                if (v1.Y < v2.Y) {
                    max_y = v2.Y;
                }

                if (max_y < v3.Y) {
                    max_y = v3.Y;
                }

                if (max_y < v4.Y) {
                    max_y = v4.Y;
                }

                int x_min = GameMath::Fast_To_Int_Floor(min_x / 10.0f);
                int y_min = GameMath::Fast_To_Int_Floor(min_y / 10.0f);
                int x_max = GameMath::Fast_To_Int_Floor(max_x / 10.0f);
                int y_max = GameMath::Fast_To_Int_Floor(max_y / 10.0f);
                float ground_height_sum = 0.0f;
                int ground_height_sample_count = 0;

                for (int x = x_min; x <= x_max; x++) {
                    for (int y = 0; y <= y_max; y++) {
                        Vector3 v(x * 10.0f, y * 10.0f, 0.0f);
                        unsigned char flags;
                        bool inside = Point_In_Triangle_2D(v1, v2, v4, v, 0, 1, flags);

                        if (Point_In_Triangle_2D(v2, v3, v4, v, 0, 1, flags)) {
                            inside = true;
                        }

                        if (inside) {
                            ground_height_sum += g_theTerrainLogic->Get_Ground_Height(v.X, v.Y, nullptr);
                            ground_height_sample_count++;
                        }
                    }
                }

                if (ground_height_sample_count != 0) {
                    int flattened_ground_height = GameMath::Fast_To_Int_Floor(
                        (ground_height_sum / ground_height_sample_count) / HEIGHTMAP_SCALE + 0.5f);
                    int height2 = GameMath::Fast_To_Int_Floor(
                        g_theTerrainLogic->Get_Ground_Height(pos->x, pos->y, nullptr) / HEIGHTMAP_SCALE);

                    if (flattened_ground_height > height2) {
                        flattened_ground_height = height2;
                    }

                    for (int x = x_min; x <= x_max; x++) {
                        for (int y = 0; y <= y_max; y++) {
                            Vector3 v(x * 10.0f, y * 10.0f, 0.0f);
                            unsigned char flags;
                            bool inside = Point_In_Triangle_2D(v1, v2, v4, v, 0, 1, flags);

                            if (Point_In_Triangle_2D(v2, v3, v4, v, 0, 1, flags)) {
                                inside = true;
                            }

                            if (inside) {
                                ICoord2D c;

                                c.x = x;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x + 1;
                                c.y = y - 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);

                                c.x = x - 1;
                                c.y = y + 1;
                                g_theTerrainVisual->Set_Raw_Map_Height(&c, flattened_ground_height);
                            }
                        }
                    }
                }
            }
        }
    }
}

void TerrainLogic::Create_Crater_In_Terrain(Object *obj)
{
    if (!obj->Get_Geometry_Info().Is_Small()) {
        const Coord3D *pos = obj->Get_Position();
        float major_radius = obj->Get_Geometry_Info().Get_Major_Radius();

        if (major_radius > 0.0f) {
            int x_min = GameMath::Fast_To_Int_Floor((pos->x - major_radius) / 10.0f);
            int y_min = GameMath::Fast_To_Int_Floor((pos->y - major_radius) / 10.0f);
            int x_max = GameMath::Fast_To_Int_Floor((pos->x + major_radius) / 10.0f);
            int y_max = GameMath::Fast_To_Int_Floor((pos->y + major_radius) / 10.0f);

            for (int x = x_min; x <= x_max; x++) {
                for (int y = 0; y <= y_max; y++) {
                    float crater_radius = GameMath::Sqrt(GameMath::Square(x * 10.0f) + GameMath::Square(y * 10.0f));

                    if (crater_radius < major_radius) {
                        ICoord2D c;
                        c.x = x;
                        c.y = y;
                        float crater_depth = (1.0f - crater_radius / major_radius) * major_radius;
                        float new_height;

                        if (g_theTerrainVisual->Get_Raw_Map_Height(&c) - crater_depth >= 1.0f) {
                            new_height = g_theTerrainVisual->Get_Raw_Map_Height(&c) - crater_depth;
                        } else {
                            new_height = 1.0f;
                        }

                        g_theTerrainVisual->Set_Raw_Map_Height(&c, new_height);
                    }
                }
            }
        }
    }
}

void TerrainLogic::Update()
{
    m_bridgeDamageStatesChanged = false;

    if (m_numWaterToUpdate != 0) {
        bool do_damage = g_theGameLogic->Get_Frame() % 30 == 0;

        for (int i = m_numWaterToUpdate - 1; i >= 0; i--) {
            const WaterHandle *water_table = m_waterToUpdate[i].water_table;
            float change_per_frame = m_waterToUpdate[i].change_per_frame;
            float target_height = m_waterToUpdate[i].target_height;
            float damage_amount = m_waterToUpdate[i].damage_amount;
            float current_height = m_waterToUpdate[i].current_height;

            bool update_pathfind = false;

            if (change_per_frame <= 0.0f) {
                if (current_height + change_per_frame <= target_height) {
                    update_pathfind = true;
                }
            } else if (current_height + change_per_frame >= target_height) {
                update_pathfind = true;
            }

            if (update_pathfind) {
                Set_Water_Height(water_table, target_height, damage_amount, true);

                for (int j = i; j < m_numWaterToUpdate; j++) {
                    m_waterToUpdate[i] = m_waterToUpdate[j];
                }

                m_numWaterToUpdate--;
            } else {
                if (!do_damage) {
                    damage_amount = 0.0f;
                }

                float new_height = current_height + change_per_frame;
                m_waterToUpdate[i].current_height = new_height;
                Set_Water_Height(water_table, new_height, damage_amount, false);
            }
        }
    }
}
