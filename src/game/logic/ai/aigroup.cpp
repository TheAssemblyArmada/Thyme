/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI Group
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aigroup.h"
#include "actionmanager.h"
#include "ai.h"
#include "aipathfind.h"
#include "bodymodule.h"
#include "buildassistant.h"
#include "drawable.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "line2d.h"
#include "overchargebehavior.h"
#include "player.h"
#include "simpleobjectiterator.h"
#include "spawnbehavior.h"
#include "specialpower.h"
#include "specialpowerupdatemodule.h"
#include "stealthupdate.h"
#include "terrainlogic.h"
#include <algorithm>

AIGroup::AIGroup() :
    m_memberListSize(0), m_speed(0.0f), m_dirty(false), m_id(g_theAI->Get_Next_Group_ID()), m_groundPath(nullptr)
{
    m_memberList.clear();
}

AIGroup::~AIGroup()
{
    for (auto it = m_memberList.begin(); it != m_memberList.end();) {
        Object *member_obj = *it;

        if (member_obj != nullptr) {
            member_obj->Leave_Group();
            it = m_memberList.begin();
        } else {
            it = m_memberList.erase(it);
        }
    }

    if (m_groundPath != nullptr) {
        m_groundPath->Delete_Instance();
        m_groundPath = nullptr;
    }
}

int AIGroup::Get_ID()
{
    return m_id;
}

const std::vector<ObjectID> &AIGroup::Get_All_IDs() const
{
    m_lastRequestedIDList.clear();

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (*it != nullptr) {
            m_lastRequestedIDList.push_back((*it)->Get_ID());
        }
    }

    return m_lastRequestedIDList;
}

float AIGroup::Get_Speed()
{
    if (m_dirty) {
        Recompute();
    }

    return m_speed;
}

bool AIGroup::Is_Member(Object *obj)
{
    return std::find(m_memberList.begin(), m_memberList.end(), obj) != m_memberList.end();
}

void AIGroup::Add(Object *obj)
{
    captainslog_dbgassert(obj != nullptr, "trying to add null obj to AIGroup");

    if (obj != nullptr) {
        BitFlags<KINDOF_COUNT> flags;
        flags.Set(KINDOF_STRUCTURE, true);
        flags.Set(KINDOF_ALWAYS_SELECTABLE, true);

        if (obj->Get_AI_Update_Interface() != nullptr || obj->Is_Any_KindOf(flags)) {
            m_memberList.push_back(obj);
            m_memberListSize++;
            obj->Enter_Group(this);
            m_dirty = true;
        }
    }
}

bool AIGroup::Remove(Object *obj)
{
    auto it = std::find(m_memberList.begin(), m_memberList.end(), obj);

    if (it == m_memberList.end()) {
        return false;
    }

    m_memberList.erase(it);
    m_memberListSize--;
    obj->Leave_Group();
    m_dirty = true;

    if (!Is_Empty()) {
        return false;
    }

    g_theAI->Destroy_Group(this);
    return true;
}

bool AIGroup::Contains_Any_Objects_Not_Owned_By_Player(const Player *player)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (*it != nullptr && (*it)->Get_Controlling_Player() != player) {
            return true;
        }
    }

    return false;
}

bool AIGroup::Remove_Any_Objects_Not_Owned_By_Player(const Player *player)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end();) {
        Object *obj = *it;
        if (obj != nullptr) {
            if (obj->Get_Controlling_Player() == player) {
                it++;
            } else {
                it++;

                if (Remove(obj)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool AIGroup::Get_Center(Coord3D *center)
{
    int count = 0;
    center->x = 0;
    center->y = 0;
    center->z = 0;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
            if ((*it)->Get_AI_Update_Interface() != nullptr) {
                *center += *(*it)->Get_Position();
                count++;
            }
        }
    }

    if (count == 0 && !m_memberList.empty()) {
        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                *center += *(*it)->Get_Position();
                count++;
            }
        }
    }

    *center /= count;
    return count > 0;
}

bool AIGroup::Get_Min_Max_And_Center(Coord2D *min, Coord2D *max, Coord3D *center)
{
    int count = 0;
    min->x = 10000000000.0f;
    max->x = -10000000000.0f;
    min->y = 10000000000.0f;
    max->y = -10000000000.0f;
    center->x = 0;
    center->y = 0;
    center->z = 0;
    FormationID id = INVALID_FORMATION_ID;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
            if ((*it)->Get_AI_Update_Interface() != nullptr) {
                const Coord3D *pos = (*it)->Get_Position();
                *center += *pos;
                float x;
                float y;

                if (min->x <= pos->x) {
                    x = min->x;
                } else {
                    x = pos->x;
                }

                min->x = x;

                if (max->x >= pos->x) {
                    x = max->x;
                } else {
                    x = pos->x;
                }

                max->x = x;

                if (min->y <= pos->y) {
                    y = min->y;
                } else {
                    y = pos->y;
                }

                min->y = y;

                if (max->y >= pos->y) {
                    y = max->y;
                } else {
                    y = pos->y;
                }

                max->y = y;

                if (count == 0) {
                    id = (*it)->Get_Formation_ID();
                }

                count++;
            }
        }
    }

    *center /= count;

    if (count < 2) {
        return false;
    }

    return id != INVALID_FORMATION_ID;
}

void AIGroup::Recompute()
{
    float min_distance = 1000000000.0f;
    Coord3D center;
    Get_Center(&center);

    if (m_groundPath != nullptr) {
        m_groundPath->Delete_Instance();
        m_groundPath = nullptr;
    }

    m_speed = 10000000000.0f;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (!(*it)->Is_KindOf(KINDOF_IMMOBILE)) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

                if (update != nullptr) {
                    float speed = update->Get_Cur_Locomotor_Speed();

                    if (m_speed > speed) {
                        const BodyModuleInterface *body = (*it)->Get_Body_Module();

                        if (body->Get_Damage_State() < g_theWriteableGlobalData->m_movementPenaltyDamageState) {
                            m_speed = speed;
                        }
                    }

                    const Coord3D *pos = (*it)->Get_Position();
                    float dx = pos->x - center.x;
                    float dy = pos->y - center.y;
                    float dist = dx * dx + dy * dy;

                    if (dist < min_distance) {
                        min_distance = dist;
                    }
                }
            }
        }
    }

    m_dirty = false;
}

unsigned int AIGroup::Get_Count()
{
    return m_memberListSize;
}

bool AIGroup::Is_Empty()
{
    return m_memberList.empty();
}

void AIGroup::Compute_Individual_Destination(
    Coord3D *dest, const Coord3D *group_dest, Object *obj, const Coord3D *center, bool adjust)
{
    const Coord3D *pos = obj->Get_Position();
    Coord2D offset;

    if (adjust) {
        obj->Get_Formation_Offset(&offset);
    } else {
        offset.x = pos->x - center->x;
        offset.y = pos->y - center->y;
    }

    float length = offset.Length();

    if (obj->Get_Geometry_Info().Get_Bounding_Circle_Radius() * 6.0f < length) {
        length = obj->Get_Geometry_Info().Get_Bounding_Circle_Radius() * 6.0f;
    }

    offset.Normalize();
    offset *= length;

    PathfindLayerEnum layer = g_theTerrainLogic->Get_Layer_For_Destination(group_dest);
    dest->x = offset.x + group_dest->x;
    dest->y = offset.y + group_dest->y;
    dest->z = g_theTerrainLogic->Get_Layer_Height(dest->x, dest->y, layer, nullptr, true);
    AIUpdateInterface *update = obj->Get_AI_Update_Interface();

    if (update != nullptr && update->Is_Doing_Ground_Movement()) {
        const LocomotorSet *locomotor = update->Get_Locomotor_Set();

        if (adjust) {
            g_theAI->Get_Pathfinder()->Adjust_Destination(obj, *locomotor, dest, nullptr);
        } else {
            g_theAI->Get_Pathfinder()->Adjust_Destination(obj, *locomotor, dest, group_dest);
        }

        g_theAI->Get_Pathfinder()->Update_Goal(obj, dest, LAYER_GROUND);
    }
}

bool AIGroup::Friend_Compute_Ground_Path(const Coord3D *pos, CommandSourceType cmd_source)
{
    if (m_dirty) {
        Recompute();
    }

    if (g_theWriteableGlobalData->m_debugAI == 2) {
        return false;
    }

    bool do_find_path = false;
    Coord2D min;
    Coord2D max;
    Coord3D center;
    Get_Min_Max_And_Center(&min, &max, &center);
    float goal_min_dist_sqr = GameMath::Square(g_theAI->Get_AI_Data()->m_distanceRequiresGroup) * 4.0f;
    int infantry_count = 0;
    int vehicle_count = 0;
    Object *group_center_obj = nullptr;
    float group_center_min_dist_sqr = goal_min_dist_sqr * 10.0f;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = *it;
        g_theAI->Get_Pathfinder()->Remove_Goal(member_obj);

        if (!member_obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)
            && member_obj->Get_AI_Update_Interface() != nullptr) {
            if (member_obj->Is_KindOf(KINDOF_INFANTRY)) {
                infantry_count++;
            } else {
                if (!member_obj->Is_KindOf(KINDOF_VEHICLE) || member_obj->Is_KindOf(KINDOF_AIRCRAFT)) {
                    continue;
                }

                vehicle_count++;
            }

            Coord3D pos2 = *(*it)->Get_Position();
            float dx = pos2.x - pos->x;
            float dy = pos2.y - pos->y;

            if (dx * dx + dy * dy < goal_min_dist_sqr) {
                goal_min_dist_sqr = dx * dx + dy * dy;
            }

            dx = pos2.x - center.x;
            dy = pos2.y - center.y;

            if (group_center_obj == nullptr || dx * dx + dy * dy < group_center_min_dist_sqr) {
                group_center_obj = *it;
                group_center_min_dist_sqr = dx * dx + dy * dy;
            }
        }
    }

    if (group_center_obj == nullptr) {
        return false;
    }

    center = *group_center_obj->Get_Position();
    float dx = max.x - min.x;
    float dy = max.y - min.y;

    if (GameMath::Square(g_theAI->Get_AI_Data()->m_distanceRequiresGroup) < dx * dx + dy * dy) {
        goal_min_dist_sqr = dx * dx + dy * dy;
    }

    if (GameMath::Square(g_theAI->Get_AI_Data()->m_distanceRequiresGroup) > goal_min_dist_sqr) {
        return false;
    }

    if (GameMath::Square(g_theAI->Get_AI_Data()->m_distanceRequiresGroup) < goal_min_dist_sqr) {
        do_find_path = true;
    }

    if (infantry_count > 6) {
        do_find_path = true;
    }

    if (vehicle_count > 4) {
        do_find_path = true;
    }

    if (!do_find_path) {
        bool is_line_passable = true;
        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            Object *member_obj = *it;
            if (member_obj->Is_KindOf(KINDOF_INFANTRY)) {
                AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

                if (update != nullptr) {
                    if (!g_theAI->Get_Pathfinder()->Is_Line_Passable(member_obj,
                            update->Get_Locomotor_Set()->Get_Valid_Surfaces(),
                            member_obj->Get_Layer(),
                            member_obj->Get_Position(),
                            &center,
                            false,
                            true)) {
                        is_line_passable = false;
                    }
                }
            }
        }

        if (is_line_passable) {
            do_find_path = true;
        }
    }

    if (!do_find_path) {
        return false;
    }

    m_groundPath = g_theAI->Get_Pathfinder()->Find_Ground_Path(&center, pos, 6, false);
    return m_groundPath != nullptr;
}

void Clamp_To_Map(Coord3D *pos, Player::PlayerType type)
{
    Region3D extent;

    if (type == Player::PLAYER_COMPUTER) {
        g_theTerrainLogic->Get_Maximum_Pathfind_Extent(&extent);
    } else {
        g_theTerrainLogic->Get_Extent(&extent);
    }

    extent.hi.x -= 10.0f;
    extent.hi.y -= 10.0f;
    extent.lo.x += 10.0f;
    extent.lo.y += 10.0f;

    if (!extent.Is_In_Region_No_Z(pos)) {
        if (pos->x < extent.lo.x) {
            pos->x = extent.lo.x;
        }
        if (pos->y < extent.lo.y) {
            pos->y = extent.lo.y;
        }
        if (pos->x > extent.hi.x) {
            pos->x = extent.hi.x;
        }
        if (pos->y > extent.hi.y) {
            pos->y = extent.hi.y;
        }
    }
}

bool AIGroup::Friend_Move_Infantry_To_Pos(const Coord3D *pos, CommandSourceType cmd_source)
{
    if (m_groundPath == nullptr) {
        return false;
    }

    const int i1 = 3;
    const int i2 = 1;
    Coord3D center;

    if (!Get_Center(&center)) {
        return false;
    }

    Coord3D first_node_pos = *m_groundPath->Get_First_Node()->Get_Position();
    const float max_dist_sqr = GameMath::Square(60.0f);
    PathNode *first_node = nullptr;

    for (PathNode *node = m_groundPath->Get_First_Node(); node != nullptr;
         node = node->Get_Next_Optimized(nullptr, nullptr)) {
        float dx = node->Get_Position()->x - first_node_pos.x;
        float dy = node->Get_Position()->y - first_node_pos.y;

        if (dx * dx + dy * dy > max_dist_sqr) {
            first_node = node;
            break;
        }
    }

    Coord3D last_node_pos = *m_groundPath->Get_Last_Node()->Get_Position();
    PathNode *last_node = nullptr;

    for (PathNode *node = m_groundPath->Get_First_Node(); node != nullptr;
         node = node->Get_Next_Optimized(nullptr, nullptr)) {
        float dx = node->Get_Position()->x - last_node_pos.x;
        float dy = node->Get_Position()->y - last_node_pos.y;

        if (dx * dx + dy * dy > max_dist_sqr) {
            last_node = node;
            break;
        }
    }

    if (first_node == nullptr || last_node == nullptr) {
        m_groundPath->Delete_Instance();
        m_groundPath = nullptr;
        return false;
    } else {
        Coord2D first_pos;
        first_pos.x = first_node->Get_Position()->x - first_node_pos.x;
        first_pos.y = first_node->Get_Position()->y - first_node_pos.y;
        first_pos.Normalize();

        Coord2D last_pos;
        last_pos.x = last_node_pos.x - last_node->Get_Position()->x;
        last_pos.y = last_node_pos.y - last_node->Get_Position()->y;
        last_pos.Normalize();

        Coord2D first_pos2;
        first_pos2.x = -first_pos.y;
        first_pos2.y = first_pos.x;
        first_pos2.Normalize();

        Coord2D last_pos2;
        last_pos2.x = -last_pos.y;
        last_pos2.y = last_pos.x;
        last_pos2.Normalize();

        bool has_infantry_closer_to_last = false;
        int group_infantry_count = 0;

        MemoryPoolObjectHolder hold1(nullptr);
        SimpleObjectIterator *iter1 = new SimpleObjectIterator();
        hold1.Hold(iter1);

        MemoryPoolObjectHolder hold2(nullptr);
        SimpleObjectIterator *iter2 = new SimpleObjectIterator();
        hold2.Hold(iter2);

        Player::PlayerType type = Player::PLAYER_COMPUTER;

        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                if ((*it)->Is_KindOf(KINDOF_INFANTRY)) {
                    if ((*it)->Get_AI_Update_Interface() != nullptr) {
                        if ((*it)->Is_KindOf(KINDOF_MOB_NEXUS)) {
                            return false;
                        }

                        if ((*it)->Get_Controlling_Player() != nullptr) {
                            type = (*it)->Get_Controlling_Player()->Get_Player_Type();
                        }

                        Coord3D member_pos = *(*it)->Get_Position();
                        g_theAI->Get_Pathfinder()->Remove_Goal(*it);
                        float dx = member_pos.x - center.x;
                        float dy = member_pos.y - center.y;

                        iter1->Insert(*it, dx * first_pos2.x + dy * first_pos2.y);
                        group_infantry_count++;

                        dx = member_pos.x - last_node_pos.x;
                        dy = member_pos.y - last_node_pos.y;
                        float dist_to_last_sqr = dx * dx + dy * dy;

                        dx = member_pos.x - first_node_pos.x;
                        dy = member_pos.y - first_node_pos.y;
                        float dist_to_first_sqr = dx * dx + dy * dy;

                        if (dist_to_first_sqr > dist_to_last_sqr) {
                            has_infantry_closer_to_last = true;
                        }
                    }
                }
            }
        }

        if (group_infantry_count < g_theAI->Get_AI_Data()->m_minInfantryForGroup) {
            return false;
        }

        if (has_infantry_closer_to_last) {
            first_pos = last_pos;
            first_pos2 = last_pos2;

            for (Object *iter_obj = iter1->First(); iter_obj != nullptr; iter_obj = iter1->Next()) {
                iter2->Insert(iter_obj, 0.0f);
            }

            iter1->Make_Empty();

            for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
                Coord3D iter_pos = *iter_obj->Get_Position();
                float dx = iter_pos.x - center.x;
                float dy = iter_pos.y - center.y;
                iter1->Insert(iter_obj, dx * first_pos2.x + dy * first_pos2.y);
            }

            iter2->Make_Empty();
        }

        iter1->Sort(ITER_SORTED_FAR_TO_NEAR);
        int count2 = 0;

        for (Object *iter_obj = iter1->First(); iter_obj != nullptr; iter_obj = iter1->Next()) {
            AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
            int count3 = (group_infantry_count + 1) / i1;

            if (count3 < 1) {
                count3 = 1;
            }

            int count4 = 1 - count2 / count3;

            if (count4 < -i2) {
                count4 = -i2;
            }

            count3 = (group_infantry_count + 3) / 5;

            if (count3 < 1) {
                count3 = 1;
            }

            int count5 = 2 - count2 / count3;

            if (count5 < -2) {
                count5 = -2;
            }

            if (group_infantry_count < 16) {
                count5 = count4;
            }

            update->Set_Unk3(static_cast<unsigned short>(count4) | (count5 << 16));
            float dx = iter_obj->Get_Position()->x - center.x;
            float dy = iter_obj->Get_Position()->y - center.y;
            int i4 = 0;
            LocomotorGroupMovementPriority priority = PRIORITY_MOVES_FRONT;

            if (update->Get_Cur_Locomotor() != nullptr) {
                priority = update->Get_Cur_Locomotor()->Get_Group_Movement_Priority();

                if (priority == PRIORITY_MOVES_MIDDLE) {
                    i4 = -1000;
                } else if (priority == PRIORITY_MOVES_BACK) {
                    i4 = -2000;
                }
            }

            iter2->Insert(iter_obj, i4 + dx * first_pos.x + dy * first_pos.y);
            count2++;
        }

        iter2->Sort(ITER_SORTED_FAR_TO_NEAR);
        int counts1[3];
        int counts2[5];
        memset(counts1, 0, sizeof(counts1));
        memset(counts2, 0, sizeof(counts2));

        for (int i = 2; i >= 0; i--) {
            for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
                AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
                int unk3 = update->Get_Unk3();
                LocomotorGroupMovementPriority priority = PRIORITY_MOVES_MIDDLE;

                if (update->Get_Cur_Locomotor()) {
                    priority = update->Get_Cur_Locomotor()->Get_Group_Movement_Priority();
                }

                if (i == priority) {
                    int i5 = unk3 >> 16;
                    int i6 = static_cast<short>(unk3);
                    int i7 = 10000;
                    int i8 = 10000;

                    for (int j = 0; j < ARRAY_SIZE(counts1); j++) {
                        if (counts1[j] < i7) {
                            i7 = counts1[j];
                        }
                    }

                    for (int j = 0; j < ARRAY_SIZE(counts2); j++) {
                        if (counts2[j] < i8) {
                            i8 = counts2[j];
                        }
                    }

                    int i9 = 10000;
                    int i10 = -1;

                    for (int j = 0; j < ARRAY_SIZE(counts1); j++) {
                        if (counts1[j] == i7) {
                            int i11 = i6 + 1 - j;

                            if (i11 < 0) {
                                i11 = -i11;
                            }

                            if (i11 < i9) {
                                i9 = i11;
                                i10 = j;
                            }
                        }
                    }

                    if (i10 >= 0) {
                        counts1[i10]++;
                        i6 = i10 - 1;
                    }

                    i9 = 10000;
                    i10 = -1;

                    for (int j = 0; j < ARRAY_SIZE(counts2); j++) {
                        if (counts2[j] == i8) {
                            int i12 = i5 + 2 - j;

                            if (i12 < 0) {
                                i12 = -i12;
                            }

                            if (i12 < i9) {
                                i9 = i12;
                                i10 = j;
                            }
                        }
                    }

                    if (i10 >= 0) {
                        counts2[i10]++;
                        i5 = i10 - 2;
                    }

                    if (group_infantry_count < 16) {
                        i5 = i6;
                    }

                    update->Set_Unk3(static_cast<unsigned short>(i6) | (i5 << 16));
                }
            }
        }

        int counts3[5];
        memset(counts3, 0, sizeof(counts3));
        PathfindLayerEnum layer = g_theTerrainLogic->Get_Layer_For_Destination(pos);

        for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
            AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
            int unk3 = update->Get_Unk3();
            int i13 = unk3 >> 16;
            int i14 = static_cast<short>(unk3);
            int i15 = counts3[i13 + 2];
            counts3[i13 + 2] = i15 + 1;
            std::vector<Coord3D> coords;
            PathNode *node = first_node;
            PathNode *first_node2 = m_groundPath->Get_First_Node();
            Coord3D iter_pos = *iter_obj->Get_Position();

            while (node != nullptr) {
                Coord3D node_pos = *node->Get_Position();
                PathNode *node2 = nullptr;

                for (PathNode *node3 = node->Get_Next_Optimized(nullptr, nullptr); node3 != nullptr;
                     node3 = node3->Get_Next_Optimized(nullptr, nullptr)) {
                    float dx = node3->Get_Position()->x - node_pos.x;
                    float dy = node3->Get_Position()->y - node_pos.y;

                    if (dx * dx + dy * dy > max_dist_sqr) {
                        node2 = node3;
                        break;
                    }
                }

                if (node2 == nullptr) {
                    break;
                }

                Coord2D first_node_pos2;
                first_node_pos2.y = node2->Get_Position()->x - first_node2->Get_Position()->x;
                first_node_pos2.x = -(node2->Get_Position()->y - first_node2->Get_Position()->y);
                first_node_pos2.Normalize();

                float dx = node2->Get_Position()->x - first_node2->Get_Position()->x;
                float dy = node2->Get_Position()->y - first_node2->Get_Position()->y;
                float f3 = 21.0f / i2;
                node_pos.x = i14 * f3 * first_node_pos2.x + node_pos.x;
                node_pos.y = i14 * f3 * first_node_pos2.y + node_pos.y;

                if ((i15 & 1) != 0) {
                    node_pos.x = 5.0f * first_node_pos2.x + node_pos.x;
                    node_pos.y = 5.0f * first_node_pos2.y + node_pos.y;
                } else {
                    node_pos.x = node_pos.x - 5.0f * first_node_pos2.x;
                    node_pos.y = node_pos.y - 5.0f * first_node_pos2.y;
                }

                float dx2 = node_pos.x - iter_pos.x;
                float dy2 = node_pos.y - iter_pos.y;
                Clamp_To_Map(&node_pos, type);

                if (dx * dx2 + dy * dy2 > 0.0f) {
                    coords.push_back(node_pos);
                    iter_pos = node_pos;
                }

                node = node->Get_Next_Optimized(nullptr, nullptr);

                for (PathNode *node3 = first_node2->Get_Next_Optimized(nullptr, nullptr); node3 != nullptr && node3 != node;
                     node3 = node3->Get_Next_Optimized(nullptr, nullptr)) {
                    float dx3 = node3->Get_Position()->x - node->Get_Position()->x;
                    float dy3 = node3->Get_Position()->y - node->Get_Position()->y;

                    if (dx3 * dx3 + dy3 * dy3 > max_dist_sqr) {
                        first_node2 = node3;
                    }
                }
            }

            Coord3D goal_pos = *pos;

            if (i13 < -2) {
                i13 = -2;
            }

            if (i13 > 2) {
                i13 = 2;
            }

            float f4 = 22.0f;
            goal_pos.x = i13 * 22.0f * last_pos2.x + goal_pos.x;
            goal_pos.y = i13 * 22.0f * last_pos2.y + goal_pos.y;

            if ((i15 & 1) != 0) {
                goal_pos.x = 10.0f * last_pos2.x + goal_pos.x;
                goal_pos.y = 10.0f * last_pos2.y + goal_pos.y;
            }

            LocomotorGroupMovementPriority priority = PRIORITY_MOVES_MIDDLE;

            if (update->Get_Cur_Locomotor()) {
                priority = update->Get_Cur_Locomotor()->Get_Group_Movement_Priority();
            }

            goal_pos.x = (priority - 2) * 10.0f * last_pos.x + goal_pos.x;
            goal_pos.y = (priority - 2) * 10.0f * last_pos.y + goal_pos.y;
            goal_pos.x = goal_pos.x - i15 * f4 * last_pos.x;
            goal_pos.y = goal_pos.y - i15 * f4 * last_pos.y;
            goal_pos.z = g_theTerrainLogic->Get_Layer_Height(goal_pos.x, goal_pos.y, layer, nullptr, true);

            while (coords.size()) {
                iter_pos = coords[coords.size() - 1];
                float dx = goal_pos.x - iter_pos.x;
                float dy = goal_pos.y - iter_pos.y;

                if (last_pos.x * dx + last_pos.y * dy > 0.0f) {
                    break;
                }

                coords.pop_back();
            }

            Clamp_To_Map(&goal_pos, type);
            g_theAI->Get_Pathfinder()->Adjust_Destination(iter_obj, *update->Get_Locomotor_Set(), &goal_pos, nullptr);
            g_theAI->Get_Pathfinder()->Update_Goal(iter_obj, &goal_pos, LAYER_GROUND);
            coords.push_back(goal_pos);
            update->AI_Follow_Path(&coords, nullptr, cmd_source);
        }

        return true;
    }
}

void AIGroup::Friend_Move_Formation_To_Pos(const Coord3D *pos, CommandSourceType cmd_source)
{
    Coord3D center;

    if (Get_Center(&center)) {
        PathNode *first_node = nullptr;
        PathNode *last_node = nullptr;
        Coord3D last_pos = *pos;

        if (m_groundPath != nullptr) {
            Coord3D first_pos = *m_groundPath->Get_First_Node()->Get_Position();
            const float max_dist_sqr = GameMath::Square(60.0f);

            for (PathNode *node = m_groundPath->Get_First_Node(); node != nullptr;
                 node = node->Get_Next_Optimized(nullptr, nullptr)) {
                int dx = node->Get_Position()->x - first_pos.x;
                int dy = node->Get_Position()->y - first_pos.y;

                if (dx * dx + dy * dy > max_dist_sqr) {
                    first_node = node;
                    break;
                }
            }

            last_pos = *m_groundPath->Get_Last_Node()->Get_Position();
            for (PathNode *node = m_groundPath->Get_First_Node(); node != nullptr;
                 node = node->Get_Next_Optimized(nullptr, nullptr)) {
                int dx = node->Get_Position()->x - last_pos.x;
                int dy = node->Get_Position()->y - last_pos.y;

                if (dx * dx + dy * dy > max_dist_sqr) {
                    last_node = node;
                }
            }

            for (PathNode *node = last_node; node != nullptr; node = node->Get_Next_Optimized(nullptr, nullptr)) {
                if (node == first_node) {
                    last_node = nullptr;
                }
            }

            if (first_node == nullptr || last_node == nullptr) {
                m_groundPath->Delete_Instance();
                m_groundPath = nullptr;
                first_node = nullptr;
                last_node = nullptr;
            }
        }

        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                Object *member_obj = (*it);
                AIUpdateInterface *update = member_obj->Get_AI_Update_Interface();
                Coord2D offset;
                member_obj->Get_Formation_Offset(&offset);

                if (first_node != nullptr) {
                    std::vector<Coord3D> coords;

                    for (PathNode *node = first_node; node != nullptr; node = node->Get_Next_Optimized(nullptr, nullptr)) {
                        Coord3D pos4 = *node->Get_Position();
                        pos4.x = pos4.x + offset.x;
                        pos4.y = pos4.y + offset.y;
                        coords.push_back(pos4);

                        if (node == last_node) {
                            break;
                        }
                    }

                    Coord3D target_pos;
                    target_pos.z = last_pos.z;
                    target_pos.x = last_pos.x + offset.x;
                    target_pos.y = last_pos.y + offset.y;
                    g_theAI->Get_Pathfinder()->Adjust_Destination(
                        member_obj, *update->Get_Locomotor_Set(), &target_pos, nullptr);
                    g_theAI->Get_Pathfinder()->Update_Goal(member_obj, &target_pos, LAYER_GROUND);
                    coords.push_back(target_pos);
                    update->AI_Follow_Path(&coords, nullptr, cmd_source);
                } else {
                    Coord3D target_pos;
                    target_pos.z = last_pos.z;
                    target_pos.x = last_pos.x + offset.x;
                    target_pos.y = last_pos.y + offset.y;
                    update->AI_Move_To_Position(&target_pos, cmd_source);
                }
            }
        }
    }
}

bool AIGroup::Friend_Move_Vehicle_To_Pos(const Coord3D *pos, CommandSourceType cmd_source)
{
    if (m_groundPath == nullptr) {
        return false;
    }

    Coord3D center;

    if (!Get_Center(&center)) {
        return false;
    }

    if (m_groundPath == nullptr) {
        return false;
    }

    int i1 = 2;
    Coord3D first_node_pos = *m_groundPath->Get_First_Node()->Get_Position();
    const float max_dist_sqr = GameMath::Square(60.0f);
    PathNode *first_node = nullptr;

    for (PathNode *node = m_groundPath->Get_First_Node(); node; node = node->Get_Next_Optimized(nullptr, nullptr)) {
        float dx = node->Get_Position()->x - first_node_pos.x;
        float dy = node->Get_Position()->y - first_node_pos.y;

        if (dx * dx + dy * dy > max_dist_sqr) {
            first_node = node;
            break;
        }
    }

    Coord3D last_node_pos = *m_groundPath->Get_Last_Node()->Get_Position();
    PathNode *last_node = nullptr;

    for (PathNode *node = m_groundPath->Get_First_Node(); node; node = node->Get_Next_Optimized(nullptr, nullptr)) {
        float dx = node->Get_Position()->x - last_node_pos.x;
        float dy = node->Get_Position()->y - last_node_pos.y;

        if (dx * dx + dy * dy > max_dist_sqr) {
            last_node = node;
        }
    }

    if (last_node == m_groundPath->Get_First_Node()) {
        last_node = nullptr;
    }

    if (first_node == nullptr || last_node == nullptr) {
        m_groundPath->Delete_Instance();
        m_groundPath = nullptr;
        return false;
    } else {
        Coord2D first_pos;
        first_pos.x = first_node->Get_Position()->x - first_node_pos.x;
        first_pos.y = first_node->Get_Position()->y - first_node_pos.y;
        first_pos.Normalize();

        Coord2D last_pos;
        last_pos.x = last_node_pos.x - last_node->Get_Position()->x;
        last_pos.y = last_node_pos.y - last_node->Get_Position()->y;
        last_pos.Normalize();

        Coord2D first_pos2;
        first_pos2.x = -first_pos.y;
        first_pos2.y = first_pos.x;
        first_pos2.Normalize();

        Coord2D last_pos2;
        last_pos2.x = -last_pos.y;
        last_pos2.y = last_pos.x;
        last_pos2.Normalize();

        bool has_vehicle_closer_to_last = false;
        int group_vehicle_count = 0;

        MemoryPoolObjectHolder hold1(nullptr);
        SimpleObjectIterator *iter1 = new SimpleObjectIterator();
        hold1.Hold(iter1);

        MemoryPoolObjectHolder hold2(nullptr);
        SimpleObjectIterator *iter2 = new SimpleObjectIterator();
        hold2.Hold(iter2);

        Player::PlayerType type = Player::PLAYER_COMPUTER;

        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                if ((*it)->Is_KindOf(KINDOF_VEHICLE)) {
                    if ((*it)->Get_AI_Update_Interface() != nullptr) {
                        if ((*it)->Get_AI_Update_Interface()->Is_Doing_Ground_Movement()) {
                            if ((*it)->Get_Controlling_Player() != nullptr) {
                                type = (*it)->Get_Controlling_Player()->Get_Player_Type();
                            }

                            Coord3D member_pos = *(*it)->Get_Position();
                            g_theAI->Get_Pathfinder()->Remove_Goal(*it);
                            float dx = member_pos.x - center.x;
                            float dy = member_pos.y - center.y;

                            iter1->Insert(*it, dx * first_pos2.x + dy * first_pos2.y);
                            group_vehicle_count++;

                            dx = member_pos.x - last_node_pos.x;
                            dy = member_pos.y - last_node_pos.y;
                            float dist_to_last_sqr = dx * dx + dy * dy;

                            dx = member_pos.x - first_node_pos.x;
                            dy = member_pos.y - first_node_pos.y;
                            float dist_to_first_sqr = dx * dx + dy * dy;

                            if (dist_to_first_sqr > dist_to_last_sqr) {
                                has_vehicle_closer_to_last = true;
                            }
                        }
                    }
                }
            }
        }

        if (group_vehicle_count < g_theAI->Get_AI_Data()->m_minVehiclesForGroup) {
            return false;
        } else {
            if (has_vehicle_closer_to_last) {
                first_pos = last_pos;
                first_pos2 = last_pos2;

                for (Object *iter_obj = iter1->First(); iter_obj != nullptr; iter_obj = iter1->Next()) {
                    iter2->Insert(iter_obj, 0.0f);
                }

                iter1->Make_Empty();

                for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
                    Coord3D iter_pos = *iter_obj->Get_Position();
                    float dx = iter_pos.x - center.x;
                    float dy = iter_pos.y - center.y;
                    iter1->Insert(iter_obj, dx * first_pos2.x + dy * first_pos2.y);
                }

                iter2->Make_Empty();
            }

            iter1->Sort(ITER_SORTED_FAR_TO_NEAR);
            int count2 = 0;

            for (Object *iter_obj = iter1->First(); iter_obj != nullptr; iter_obj = iter1->Next()) {
                AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
                int count3 = (group_vehicle_count + 1) / i1;

                if (count3 < 1) {
                    count3 = 1;
                }

                int count4 = 1 - count2 / count3;

                if (count2 / count3 == 1) {
                    count4 = -1;
                }

                count3 = (group_vehicle_count + 1) / 3;

                if (count3 < 1) {
                    count3 = 1;
                }

                int count5 = 1 - count2 / count3;

                if (count5 < -1) {
                    count5 = -1;
                }

                if (group_vehicle_count < 5) {
                    count5 = count4;
                }

                update->Set_Unk3(static_cast<unsigned short>(count4) | (count5 << 16));
                float dx = iter_obj->Get_Position()->x - center.x;
                float dy = iter_obj->Get_Position()->y - center.y;
                int i4 = 0;
                iter2->Insert(iter_obj, i4 + dx * first_pos.x + dy * first_pos.y);
                count2++;
            }

            iter2->Sort(ITER_SORTED_FAR_TO_NEAR);
            int counts1[3];
            int counts2[3];
            memset(counts1, 0, sizeof(counts1));
            memset(counts2, 0, sizeof(counts2));

            for (int i = 2; i >= 0; i--) {
                for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
                    AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
                    int unk3 = update->Get_Unk3();
                    int i5 = unk3 >> 16;
                    int i6 = static_cast<short>(unk3);
                    int i7 = 10000;
                    int i8 = 10000;

                    for (int j = 0; j < ARRAY_SIZE(counts1); j += 2) {
                        if (counts1[j] < i7) {
                            i7 = counts1[j];
                        }
                    }
                    for (int j = 0; j < ARRAY_SIZE(counts2); j++) {
                        if (counts2[j] < i8) {
                            i8 = counts2[j];
                        }
                    }

                    int i9 = 10000;
                    int i10 = -1;

                    for (int j = 0; j < ARRAY_SIZE(counts1); j += 2) {
                        if (counts1[j] == i7) {
                            int i11 = i6 + 1 - j;

                            if (i11 < 0) {
                                i11 = -i11;
                            }

                            if (i11 < i9) {
                                i9 = i11;
                                i10 = j;
                            }
                        }
                    }

                    if (i10 >= 0) {
                        counts1[i10]++;
                        i6 = i10 - 1;
                    }

                    i9 = 10000;
                    i10 = -1;

                    for (int j = 0; j < ARRAY_SIZE(counts2); j++) {
                        if (counts2[j] == i8) {
                            int i12 = i5 + 1 - j;

                            if (i12 < 0) {
                                i12 = -i12;
                            }

                            if (i12 < i9) {
                                i9 = i12;
                                i10 = j;
                            }
                        }
                    }

                    if (i10 >= 0) {
                        counts2[i10]++;
                        i5 = i10 - 1;
                    }

                    if (group_vehicle_count < 5) {
                        i5 = i6;
                    }

                    update->Set_Unk3(static_cast<unsigned short>(i6) | (i5 << 16));
                }
            }

            count2 = 0;
            int counts3[5];
            memset(counts3, 0, sizeof(counts3));
            PathfindLayerEnum layer = g_theTerrainLogic->Get_Layer_For_Destination(pos);

            for (Object *iter_obj = iter2->First(); iter_obj != nullptr; iter_obj = iter2->Next()) {
                AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
                int unk3 = update->Get_Unk3();
                int i13 = unk3 >> 16;
                int i14 = static_cast<short>(unk3);
                int i15 = counts3[i13 + 2];
                counts3[i13 + 2] = i15 + 1;
                std::vector<Coord3D> coords;
                PathNode *node = first_node;
                PathNode *first_node2 = m_groundPath->Get_First_Node();
                Coord3D iter_pos = *iter_obj->Get_Position();

                while (node != nullptr) {
                    Coord3D node_pos = *node->Get_Position();
                    PathNode *node2 = nullptr;

                    for (PathNode *node3 = node->Get_Next_Optimized(nullptr, nullptr); node3 != nullptr;
                         node3 = node3->Get_Next_Optimized(nullptr, nullptr)) {
                        float dx = node3->Get_Position()->x - node_pos.x;
                        float dy = node3->Get_Position()->y - node_pos.y;

                        if (dx * dx + dy * dy > max_dist_sqr) {
                            node2 = node3;
                            break;
                        }
                    }

                    if (node2 == nullptr) {
                        break;
                    }

                    Coord2D first_node_pos3;
                    first_node_pos3.y = node2->Get_Position()->x - first_node2->Get_Position()->x;
                    first_node_pos3.x = -(node2->Get_Position()->y - first_node2->Get_Position()->y);
                    first_node_pos3.Normalize();

                    float dx = node2->Get_Position()->x - first_node2->Get_Position()->x;
                    float dy = node2->Get_Position()->y - first_node2->Get_Position()->y;

                    node_pos.x = i14 * 15.0f * first_node_pos3.x + node_pos.x;
                    node_pos.y = i14 * 15.0f * first_node_pos3.y + node_pos.y;

                    if ((i15 & 1) != 0) {
                        node_pos.x = 5.0f * first_node_pos3.x + node_pos.x;
                        node_pos.y = 5.0f * first_node_pos3.y + node_pos.y;
                    } else {
                        node_pos.x = node_pos.x - 5.0f * first_node_pos3.x;
                        node_pos.y = node_pos.y - 5.0f * first_node_pos3.y;
                    }

                    float dx2 = node_pos.x - iter_pos.x;
                    float dy2 = node_pos.y - iter_pos.y;
                    Clamp_To_Map(&node_pos, type);

                    if (dx * dx2 + dy * dy2 > 0.0f) {
                        coords.push_back(node_pos);
                        iter_pos = node_pos;
                    }

                    node = node->Get_Next_Optimized(nullptr, nullptr);

                    for (PathNode *node3 = first_node2->Get_Next_Optimized(nullptr, nullptr);
                         node3 != nullptr && node3 != node;
                         node3 = node3->Get_Next_Optimized(nullptr, nullptr)) {
                        float dx3 = node3->Get_Position()->x - node->Get_Position()->x;
                        float dy3 = node3->Get_Position()->y - node->Get_Position()->y;

                        if (dx3 * dx3 + dy3 * dy3 > max_dist_sqr) {
                            first_node2 = node3;
                        }
                    }
                }

                Coord3D goal_pos = *pos;

                if (i13 < -3) {
                    i13 = -3;
                }

                if (i13 > 3) {
                    i13 = 3;
                }

                float f4 = 32.0f;

                if (group_vehicle_count < 5) {
                    f4 = 15.0f;
                }

                goal_pos.x = i13 * f4 * last_pos2.x + goal_pos.x;
                goal_pos.y = i13 * f4 * last_pos2.y + goal_pos.y;

                if ((i15 & 1) != 0) {
                    goal_pos.x = 10.0f * last_pos2.x + goal_pos.x;
                    goal_pos.y = 10.0f * last_pos2.y + goal_pos.y;
                }

                goal_pos.x = goal_pos.x - i15 * f4 * last_pos.x;
                goal_pos.y = goal_pos.y - i15 * f4 * last_pos.y;

                goal_pos.z = g_theTerrainLogic->Get_Layer_Height(goal_pos.x, goal_pos.y, layer, nullptr, true);

                while (coords.size()) {
                    iter_pos = coords[coords.size() - 1];
                    float dx = goal_pos.x - iter_pos.x;
                    float dy = goal_pos.y - iter_pos.y;

                    if (last_pos.x * dx + last_pos.y * dy > 0.0f) {
                        break;
                    }

                    coords.pop_back();
                }

                Clamp_To_Map(&goal_pos, type);
                g_theAI->Get_Pathfinder()->Adjust_Destination(iter_obj, *update->Get_Locomotor_Set(), &goal_pos, nullptr);
                g_theAI->Get_Pathfinder()->Update_Goal(iter_obj, &goal_pos, LAYER_GROUND);
                coords.push_back(goal_pos);
                update->AI_Follow_Path(&coords, nullptr, cmd_source);
            }

            return true;
        }
    }
}

void Clamp_Waypoint_Position(Coord3D &pos, int i)
{
    Region3D extent;
    g_theTerrainLogic->Get_Extent(&extent);

    extent.hi.x = extent.hi.x - i;
    extent.hi.y = extent.hi.y - i;
    extent.lo.x = i + extent.lo.x;
    extent.lo.y = i + extent.lo.y;

    if (!extent.Is_In_Region_No_Z(&pos)) {
        if (pos.x > extent.hi.x) {
            pos.x = extent.hi.x;
        } else if (pos.x < extent.lo.x) {
            pos.x = extent.lo.x;
        }

        if (pos.y > extent.hi.y) {
            pos.y = extent.hi.y;
        } else if (pos.y < extent.lo.y) {
            pos.y = extent.lo.y;
        }
    }

    pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
}

void AIGroup::Group_Move_To_Position(const Coord3D *pos, bool append, CommandSourceType cmd_source)
{
    Coord3D target_pos = *pos;
    bool has_moved_infantry = false;
    bool has_moved_vehicle = false;
    bool is_target_pos_inside_gather_area = false;
    Coord2D min;
    Coord2D max;
    Coord3D center;
    bool in_formation = Get_Min_Max_And_Center(&min, &max, &center);

    if (append) {
        in_formation = false;
    }

    if (!append && !in_formation) {
        Friend_Compute_Ground_Path(&target_pos, cmd_source);
        has_moved_infantry = Friend_Move_Infantry_To_Pos(&target_pos, cmd_source);
        has_moved_vehicle = Friend_Move_Vehicle_To_Pos(&target_pos, cmd_source);
    }

    if (m_dirty) {
        Recompute();
    }

    if (!in_formation && cmd_source == COMMANDSOURCE_PLAYER
        && g_theWriteableGlobalData->m_groupMoveClickToGatherAreaFactor > 0.0f) {
        Scale_Rect_2D(&min, &max, g_theWriteableGlobalData->m_groupMoveClickToGatherAreaFactor);

        if (Coord_3D_Inside_Rect_2D(&target_pos, &min, &max)) {
            is_target_pos_inside_gather_area = true;
        }
    }

    float max_member_radius = 0.0f;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj->Is_KindOf(KINDOF_PRODUCED_AT_HELIPAD)) {
            in_formation = false;

            if (member_obj->Get_Geometry_Info().Get_Major_Radius() >= max_member_radius) {
                max_member_radius = member_obj->Get_Geometry_Info().Get_Major_Radius();
            }
        } else if (member_obj->Is_KindOf(KINDOF_AIRCRAFT)) {
            if (member_obj->Get_AI_Update_Interface() != nullptr) {
                if (!member_obj->Get_AI_Update_Interface()->Is_Doing_Ground_Movement()) {
                    is_target_pos_inside_gather_area = false;
                    in_formation = false;
                }

                if (max_member_radius <= 100.0f) {
                    max_member_radius = 100.0f;
                }
            }
        }
    }

    Clamp_Waypoint_Position(target_pos, max_member_radius + 40.0f);
    int i1;

    if (is_target_pos_inside_gather_area && (in_formation = false, !append)
        && (i1 = ((max.x - min.x) / 10.0f), i1 * i1 < 2000)) {
        Group_Tighten_To_Position(&target_pos, false, cmd_source);
    } else if (in_formation) {
        Friend_Compute_Ground_Path(&target_pos, cmd_source);
        Friend_Move_Formation_To_Pos(&target_pos, cmd_source);
    } else {
        MemoryPoolObjectHolder holder(nullptr);
        SimpleObjectIterator *iter = new SimpleObjectIterator();
        holder.Hold(iter);

        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                if (!(*it)->Is_KindOf(KINDOF_IMMOBILE)) {
                    if ((*it)->Get_AI_Update_Interface() != nullptr) {
                        if (!(*it)->Is_KindOf(KINDOF_INFANTRY) || !has_moved_infantry) {
                            if (!(*it)->Is_KindOf(KINDOF_VEHICLE) || !has_moved_vehicle
                                || !(*it)->Get_AI_Update_Interface()->Is_Doing_Ground_Movement()
                                || (*it)->Is_KindOf(KINDOF_CLIFF_JUMPER)) {
                                Coord3D member_pos = *(*it)->Get_Position();
                                g_theAI->Get_Pathfinder()->Remove_Goal(*it);
                                float dx = member_pos.x - target_pos.x;
                                float dy = member_pos.y - target_pos.y;
                                iter->Insert(*it, dx * dx + 0.0f + dy * dy);
                            }
                        }
                    }
                }
            }
        }

        Coord3D formation_target_pos = target_pos;
        iter->Sort(ITER_SORTED_NEAR_TO_FAR);
        bool is_first_in_formation = true;

        for (Object *iter_obj = iter->First(); iter_obj != nullptr; iter_obj = iter->Next()) {
            iter_obj->Set_Formation_ID(INVALID_FORMATION_ID);
            AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();

            if (is_first_in_formation) {
                if (in_formation) {
                    Coord2D offset;
                    iter_obj->Get_Formation_Offset(&offset);
                    formation_target_pos.x = formation_target_pos.x - offset.x;
                    formation_target_pos.y = formation_target_pos.y - offset.y;
                } else {
                    center = *iter_obj->Get_Position();
                }

                is_first_in_formation = false;
            }

            Coord3D dest;
            Compute_Individual_Destination(&dest, &formation_target_pos, iter_obj, &center, in_formation);

            if (cmd_source == COMMANDSOURCE_PLAYER) {
                if (iter_obj->Get_Status_Bits().Test(OBJECT_STATUS_CAN_STEALTH)
                    && update->Is_Auto_Acquire_Enemies_When_Idle()) {
                    if (!iter_obj->Get_Status_Bits().Test(OBJECT_STATUS_STEALTHED)) {
                        if (!iter_obj->Get_Status_Bits().Test(OBJECT_STATUS_DETECTED)
                            && !update->Can_Auto_Acquire_While_Stealthed()) {
                            StealthUpdate *stealth = iter_obj->Get_Stealth_Update();

                            if (stealth != nullptr) {
                                update->Set_Next_Mood_Check_Time(Get_Logic_Random_Value(0, 30) + stealth->Get_Stealth_Delay()
                                    + g_theGameLogic->Get_Frame());
                            }
                        }
                    }
                }
            }

            if (append) {
                update->AI_Follow_Waypath_Append(&dest, cmd_source);
            } else {
                update->AI_Move_To_Position(&dest, cmd_source);
            }
        }
    }
}

void AIGroup::Group_Scatter(CommandSourceType cmd_source)
{
    if (m_dirty) {
        Recompute();
    }

    Coord2D min;
    Coord2D max;
    Coord3D center;
    Get_Min_Max_And_Center(&min, &max, &center);
    MemoryPoolObjectHolder holder(nullptr);
    SimpleObjectIterator *iter = new SimpleObjectIterator();
    holder.Hold(iter);

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
            if (!(*it)->Is_KindOf(KINDOF_IMMOBILE)) {
                if ((*it)->Get_AI_Update_Interface() != nullptr) {
                    Coord3D obj_pos = *(*it)->Get_Position();
                    g_theAI->Get_Pathfinder()->Remove_Goal(*it);
                    float dx = obj_pos.x - center.x;
                    float dy = obj_pos.y - center.y;
                    iter->Insert(*it, dx * dx + dy * dy);
                }
            }
        }
    }

    iter->Sort(ITER_SORTED_FAR_TO_NEAR);

    for (Object *iter_obj = iter->First(); iter_obj != nullptr; iter_obj = iter->Next()) {
        center.x = center.x - 0.0099999998f;
        AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();
        Coord3D iter_pos = *iter_obj->Get_Position();
        Coord2D dist_to_center;
        dist_to_center.x = iter_pos.x - center.x;
        dist_to_center.y = iter_pos.y - center.y;
        dist_to_center.Normalize();
        iter_pos.x = iter_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius() * (dist_to_center.x * 4.0f) + iter_pos.x;
        iter_pos.y = iter_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius() * (dist_to_center.y * 4.0f) + iter_pos.y;
        update->AI_Move_To_Position(&iter_pos, cmd_source);
    }
}

void Get_Helicopter_Offset(Coord3D &pos, int count)
{
    if (count != 0) {
        float radius = 70.0f;
        float circumference = 70.0f * GAMEMATH_PI2;
        float angle = 0.0f;
        float angle_step = 70.0f / circumference * GAMEMATH_PI2;

        for (int i = 1; i < count; i++) {
            angle += angle;

            if (angle > GAMEMATH_PI2) {
                float new_radius = radius + 70.0f;
                radius = new_radius;
                float new_circumference = new_radius * GAMEMATH_PI2;
                angle_step = 70.0f / new_circumference * GAMEMATH_PI2;
                angle -= GAMEMATH_PI2;
            }
        }

        float initial_x = pos.x;
        float initial_y = pos.y;
        pos.x = GameMath::Sin(angle) * radius + initial_y;
        pos.y = GameMath::Cos(angle) * radius + initial_x;
    }
}

void AIGroup::Group_Tighten_To_Position(const Coord3D *target_pos, bool append, CommandSourceType cmd_source)
{
    if (cmd_source == COMMANDSOURCE_PLAYER && g_theWriteableGlobalData->m_groupMoveClickToGatherAreaFactor > 0.0f) {
        Coord2D min;
        Coord2D max;
        Coord3D center;
        Get_Min_Max_And_Center(&min, &max, &center);
    }

    MemoryPoolObjectHolder holder(nullptr);
    SimpleObjectIterator *iter = new SimpleObjectIterator();
    holder.Hold(iter);

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Coord3D member_pos = *(*it)->Get_Position();

        if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
            if (!(*it)->Is_KindOf(KINDOF_IMMOBILE)) {
                if ((*it)->Get_AI_Update_Interface() != nullptr) {
                    float dx = member_pos.x - target_pos->x;
                    float dy = member_pos.y - target_pos->y;
                    iter->Insert(*it, dx * dx + dy * dy);
                }
            }
        }
    }

    iter->Sort(ITER_SORTED_NEAR_TO_FAR);
    int heli_count = 0;

    for (Object *iter_obj = iter->First(); iter_obj != nullptr; iter_obj = iter->Next()) {
        AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();

        if (append) {
            update->AI_Follow_Waypath_Append(target_pos, cmd_source);
        } else if (iter_obj->Is_KindOf(KINDOF_PRODUCED_AT_HELIPAD)) {
            Coord3D offset = *target_pos;
            heli_count++;
            Get_Helicopter_Offset(offset, heli_count);
            update->AI_Tighten_To_Position(&offset, COMMANDSOURCE_AI);
        } else {
            update->AI_Tighten_To_Position(target_pos, cmd_source);
        }
    }

    for (Object *iter_obj = iter->First(); iter_obj != nullptr; iter_obj = iter->Next()) {
        Coord3D iter_pos = *iter_obj->Get_Position();
        g_theAI->Get_Pathfinder()->Update_Pos(iter_obj, &iter_pos);
    }
}

void AIGroup::Group_Follow_Waypoint_Path(const Waypoint *way, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Follow_Waypoint_Path(way, cmd_source);
        }
    }
}

void AIGroup::Group_Follow_Waypoint_Path_Exact(const Waypoint *way, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Follow_Waypoint_Path_Exact(way, cmd_source);
        }
    }
}

void AIGroup::Group_Move_To_And_Evacuate(const Coord3D *pos, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Move_To_And_Evacuate(pos, cmd_source);
        }
    }
}

void AIGroup::Group_Move_To_And_Evacuate_And_Exit(const Coord3D *pos, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Move_To_And_Evacuate_And_Exit(pos, cmd_source);
        }
    }
}

void AIGroup::Group_Follow_Waypoint_Path_As_Team(const Waypoint *way, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Follow_Waypoint_Path_As_Team(way, cmd_source);
        }
    }
}

void AIGroup::Group_Follow_Waypoint_Path_As_Team_Exact(const Waypoint *way, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Follow_Waypoint_Path_Exact_As_Team(way, cmd_source);
        }
    }
}

void Make_Member_Stop(Object *obj, void *user_data)
{
    CommandSourceType source = *static_cast<CommandSourceType *>(user_data);

    if (obj != nullptr) {
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Idle(source);
        }
    }
}

void AIGroup::Group_Idle(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);
        AIUpdateInterface *update = member_obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Idle(cmd_source);

            if (cmd_source == COMMANDSOURCE_PLAYER) {
                if (member_obj->Get_Status_Bits().Test(OBJECT_STATUS_CAN_STEALTH)
                    && update->Is_Auto_Acquire_Enemies_When_Idle()) {
                    if (!member_obj->Get_Status_Bits().Test(OBJECT_STATUS_STEALTHED)) {
                        if (!member_obj->Get_Status_Bits().Test(OBJECT_STATUS_DETECTED)
                            && !update->Can_Auto_Acquire_While_Stealthed()) {
                            StealthUpdate *stealth = member_obj->Get_Stealth_Update();

                            if (stealth != nullptr) {
                                update->Set_Next_Mood_Check_Time(Get_Logic_Random_Value(0, 30) + stealth->Get_Stealth_Delay()
                                    + g_theGameLogic->Get_Frame());
                            }
                        }
                    }
                }
            }
        } else {
            ContainModuleInterface *contain = member_obj->Get_Contain();

            if (contain != nullptr) {
                contain->Iterate_Contained(Make_Member_Stop, &cmd_source, false);
            }
        }

        SpawnBehaviorInterface *spawn = member_obj->Get_Spawn_Behavior_Interface();

        if (spawn != nullptr) {
            spawn->Order_Slaves_To_Go_Idle(cmd_source);
        }
    }
}

void AIGroup::Group_Follow_Path(const std::vector<Coord3D> *path, Object *ignore_object, CommandSourceType cmd_source) {}

void AIGroup::Group_Attack_Object_Private(bool force, Object *victim, int max_shots_to_fire, CommandSourceType cmd_source)
{
    if (victim != nullptr) {
        Coord3D victim_pos = *victim->Get_Position();
        MemoryPoolObjectHolder holder(nullptr);
        SimpleObjectIterator *iter = new SimpleObjectIterator();
        holder.Hold(iter);

        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            Coord3D member_pos = *(*it)->Get_Position();
            if (!(*it)->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                float dx = member_pos.x - victim_pos.x;
                float dy = member_pos.y - victim_pos.y;
                iter->Insert(*it, dx * dx + 0.0f + dy * dy);
            }
        }

        iter->Sort(ITER_SORTED_NEAR_TO_FAR);

        for (Object *iter_obj = iter->First(); iter_obj != nullptr; iter_obj = iter->Next()) {
            ContainModuleInterface *contain = iter_obj->Get_Contain();

            if (contain != nullptr) {
                if (contain->Is_Passenger_Allowed_To_Fire(INVALID_OBJECT_ID)) {
                    const std::list<Object *> *list = contain->Get_Contained_Items_List();

                    if (list != nullptr) {
                        for (auto it = list->begin(); it != list->end(); it++) {
                            Object *member_obj = (*it);
                            CanAttackResult attack = member_obj->Get_Able_To_Attack_Specific_Object(
                                static_cast<AbleToAttackType>(force), victim, cmd_source, WEAPONSLOT_UNK);

                            if (attack == ATTACK_RESULT_CAN_ATTACK || attack == ATTACK_RESULT_OUT_OF_RANGE) {
                                AIUpdateInterface *update = member_obj->Get_AI_Update_Interface();

                                if (update != nullptr) {
                                    if (force) {
                                        update->AI_Force_Attack_Object(victim, max_shots_to_fire, cmd_source);
                                    } else {
                                        update->AI_Attack_Object(victim, max_shots_to_fire, cmd_source);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            SpawnBehaviorInterface *spawn = iter_obj->Get_Spawn_Behavior_Interface();

            if (spawn != nullptr && !spawn->Do_Slaves_Have_Freedom()) {
                spawn->Order_Slaves_To_Attack_Target(victim, max_shots_to_fire, cmd_source);
            }

            AIUpdateInterface *update = iter_obj->Get_AI_Update_Interface();

            if (update != nullptr && iter_obj != victim) {
                if (force) {
                    update->AI_Force_Attack_Object(victim, max_shots_to_fire, cmd_source);
                } else {
                    update->AI_Attack_Object(victim, max_shots_to_fire, cmd_source);
                }
            }
        }
    }
}

void AIGroup::Group_Attack_Team(const Team *team, int max_shots_to_fire, CommandSourceType cmd_source)
{
    if (team != nullptr) {
        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

            if (update != nullptr) {
                update->AI_Attack_Team(team, max_shots_to_fire, cmd_source);
            }
        }
    }
}

void AIGroup::Group_Attack_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source)
{
    Coord3D attack_pos;

    if (pos != nullptr) {
        attack_pos = *pos;
    }

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (pos == nullptr) {
            attack_pos.Set((*it)->Get_Position());
        }

        Object *member_obj = *it;
        ContainModuleInterface *contain = member_obj->Get_Contain();

        if (contain != nullptr) {
            const std::list<Object *> *list = contain->Get_Contained_Items_List();

            if (list != nullptr) {
                for (auto contain_it = list->begin(); contain_it != list->end(); contain_it++) {
                    Object *contain_obj = (*contain_it);
                    CanAttackResult attack = contain_obj->Get_Able_To_Use_Weapon_Against_Target(
                        ATTACK_TYPE_0, nullptr, &attack_pos, cmd_source, WEAPONSLOT_UNK);

                    if (attack == ATTACK_RESULT_CAN_ATTACK || attack == ATTACK_RESULT_OUT_OF_RANGE) {
                        AIUpdateInterface *contain_update = contain_obj->Get_AI_Update_Interface();

                        if (contain_update != nullptr) {
                            contain_update->AI_Attack_Position(&attack_pos, max_shots_to_fire, cmd_source);
                        }
                    }
                }
            }
        }

        SpawnBehaviorInterface *member_spawn = member_obj->Get_Spawn_Behavior_Interface();

        if (member_spawn != nullptr && !member_spawn->Do_Slaves_Have_Freedom()) {
            member_spawn->Order_Slaves_To_Attack_Position(&attack_pos, max_shots_to_fire, cmd_source);
        }

        AIUpdateInterface *member_update = member_obj->Get_AI_Update_Interface();

        if (member_update != nullptr) {
            member_update->AI_Attack_Position(&attack_pos, max_shots_to_fire, cmd_source);
        }
    }
}

void AIGroup::Group_Attack_Move_To_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            if ((*it)->Is_Able_To_Attack()) {
                update->AI_Attack_Move_To_Position(pos, max_shots_to_fire, cmd_source);
            } else {
                update->AI_Move_To_Position(pos, cmd_source);
            }
        }
    }
}

void AIGroup::Group_Hunt(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Hunt(cmd_source);
        }
    }
}

void AIGroup::Group_Repair(Object *obj, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Repair(obj, cmd_source);
        }
    }
}

void AIGroup::Group_Resume_Construction(Object *obj, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Resume_Construction(obj, cmd_source);
        }
    }
}

void AIGroup::Group_Get_Healed(Object *heal_depot, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Get_Healed(heal_depot, cmd_source);
        }
    }
}

void AIGroup::Group_Get_Repaired(Object *repair_depot, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Get_Repaired(repair_depot, cmd_source);
        }
    }
}

void AIGroup::Group_Enter(Object *obj, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Enter(obj, cmd_source);
        }
    }
}

void AIGroup::Group_Dock(Object *obj, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Dock(obj, cmd_source);
        }
    }
}

void AIGroup::Group_Exit(Object *object_to_exit, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Exit(object_to_exit, cmd_source);
        }
    }
}

void AIGroup::Group_Evacuate(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            if ((*it)->Is_KindOf(KINDOF_AIRCRAFT) && (*it)->Is_Airborne_Target()) {
                Coord3D member_pos = *(*it)->Get_Position();
                PathfindLayerEnum layer = g_theTerrainLogic->Get_Highest_Layer_For_Destination(&member_pos, false);
                member_pos.z = g_theTerrainLogic->Get_Layer_Height(member_pos.x, member_pos.y, layer, nullptr, true);
                update->AI_Move_To_And_Evacuate(&member_pos, cmd_source);
            } else {
                update->AI_Evacuate(false, cmd_source);
            }
        } else {
            if ((*it)->Is_KindOf(KINDOF_STRUCTURE)) {
                ContainModuleInterface *contain = (*it)->Get_Contain();

                if (contain != nullptr) {
                    contain->Order_All_Passengers_To_Exit(cmd_source, false);
                }
            }
        }
    }
}

void AIGroup::Group_Execute_Railed_Transport(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Execute_Railed_Transport(cmd_source);
        }
    }
}

void AIGroup::Group_Go_Prone(const DamageInfo *damage_info, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Go_Prone(damage_info, cmd_source);
        }
    }
}

void AIGroup::Group_Guard_Position(const Coord3D *pos, GuardMode mode, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Guard_Position(pos, mode, cmd_source);
        }
    }
}

void AIGroup::Group_Guard_Object(Object *obj_to_guard, GuardMode mode, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Guard_Object(obj_to_guard, mode, cmd_source);
        }
    }
}

void AIGroup::Group_Guard_Area(const PolygonTrigger *area_to_guard, GuardMode mode, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Guard_Area(area_to_guard, mode, cmd_source);
        }
    }
}

void AIGroup::Group_Attack_Area(const PolygonTrigger *area_to_guard, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Attack_Area(area_to_guard, cmd_source);
        }
    }
}

void AIGroup::Group_Hack_Internet(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Hack_Internet(cmd_source);
        }
    }
}

void AIGroup::Group_Create_Formation(CommandSourceType cmd_source)
{
    Coord2D min;
    Coord2D max;
    Coord3D center;
    bool in_formation = Get_Min_Max_And_Center(&min, &max, &center);
    FormationID next_id = static_cast<FormationID>(g_theAI->Get_Next_Formation_ID());
    int member_count = 0;
    FormationID cur_id = INVALID_FORMATION_ID;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        member_count++;
        cur_id = (*it)->Get_Formation_ID();
    }

    if (member_count == 1 && cur_id != INVALID_FORMATION_ID) {
        in_formation = true;
    }

    if (in_formation) {
        next_id = INVALID_FORMATION_ID;
    }

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Coord3D member_pos = *(*it)->Get_Position();
        Coord2D offset;
        offset.x = member_pos.x - center.x;
        offset.y = member_pos.y - center.y;
        (*it)->Set_Formation_ID(next_id);
        (*it)->Set_Formation_Offset(&offset);
    }
}

void AIGroup::Group_Do_Special_Power(unsigned int special_power_id, unsigned int options)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        const SpecialPowerTemplate *special_power =
            g_theSpecialPowerStore->Find_Special_Power_Template_By_ID(special_power_id);

        if (special_power != nullptr) {
            if (special_power->Get_Required_Science() == SCIENCE_INVALID
                || member_obj->Get_Controlling_Player()->Has_Science(special_power->Get_Required_Science())) {
                SpecialPowerModuleInterface *power = member_obj->Get_Special_Power_Module(special_power);

                if (power != nullptr) {
                    if (g_theActionManager->Can_Do_Special_Power(
                            member_obj, special_power, COMMANDSOURCE_PLAYER, options, true)) {
                        power->Do_Special_Power(options);
                        member_obj->Friend_Set_Undetected_Defector(false);
                    }
                }
            }
        }
    }
}

void AIGroup::Group_Do_Special_Power_At_Location(
    unsigned int special_power_id, const Coord3D *location, float f, const Object *object_in_way, unsigned int options)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        const SpecialPowerTemplate *special_power =
            g_theSpecialPowerStore->Find_Special_Power_Template_By_ID(special_power_id);

        if (special_power != nullptr) {
            if (special_power->Get_Required_Science() == SCIENCE_INVALID
                || member_obj->Get_Controlling_Player()->Has_Science(special_power->Get_Required_Science())) {
                SpecialPowerModuleInterface *power = member_obj->Get_Special_Power_Module(special_power);

                if (power != nullptr) {
                    if (g_theActionManager->Can_Do_Special_Power_At_Location(
                            member_obj, location, COMMANDSOURCE_PLAYER, special_power, object_in_way, options, true)) {
                        power->Do_Special_Power_At_Location(location, f, options);
                        member_obj->Friend_Set_Undetected_Defector(false);
                    }
                }
            }
        }
    }
}

void AIGroup::Group_Do_Special_Power_At_Object(unsigned int special_power_id, Object *target, unsigned int options)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        const SpecialPowerTemplate *special_power =
            g_theSpecialPowerStore->Find_Special_Power_Template_By_ID(special_power_id);

        if (special_power != nullptr) {
            if (special_power->Get_Required_Science() == SCIENCE_INVALID
                || member_obj->Get_Controlling_Player()->Has_Science(special_power->Get_Required_Science())) {
                SpecialPowerModuleInterface *power = member_obj->Get_Special_Power_Module(special_power);

                if (power != nullptr) {
                    if (g_theActionManager->Can_Do_Special_Power_At_Object(
                            member_obj, target, COMMANDSOURCE_PLAYER, special_power, options, true)) {
                        power->Do_Special_Power_At_Object(target, options);
                        member_obj->Friend_Set_Undetected_Defector(false);
                    }
                }
            }
        }
    }
}

void AIGroup::Group_Cheer(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Set_Special_Model_Condition_State(MODELCONDITION_SPECIAL_CHEERING, 90);
    }
}

void AIGroup::Group_Sell(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        g_theBuildAssistant->Sell_Object((*it));
    }
}

void AIGroup::Group_Toggle_Overcharge(CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        for (BehaviorModule **module = (*it)->Get_All_Modules(); *module != nullptr; module++) {
            OverchargeBehaviorInterface *overcharge = (*module)->Get_Overcharge_Behavior_Interface();

            if (overcharge != nullptr) {
                overcharge->Toggle();
            }
        }
    }
}

void AIGroup::Group_Combat_Drop(Object *obj, const Coord3D &pos, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->AI_Combat_Drop(obj, pos, cmd_source);
        }
    }
}

void AIGroup::Group_Do_Command_Button(const CommandButton *button, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Do_Command_Button(button, cmd_source);
    }
}

void AIGroup::Group_Do_Command_Button_At_Position(
    const CommandButton *button, const Coord3D *pos, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Do_Command_Button_At_Position(button, pos, cmd_source);
    }
}

void AIGroup::Group_Do_Command_Button_Using_Waypoints(
    const CommandButton *button, const Waypoint *way, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Do_Command_Button_Using_Waypoints(button, way, cmd_source);
    }
}

void AIGroup::Group_Do_Command_Button_At_Object(const CommandButton *button, Object *obj, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Do_Command_Button_At_Object(button, obj, cmd_source);
    }
}

void AIGroup::Set_Attitude(AttitudeType tude)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        AIUpdateInterface *update = (*it)->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->Set_Attitude(tude);
        }
    }
}

AttitudeType AIGroup::Get_Attitude() const
{
    return AI_PASSIVE;
}

void AIGroup::Set_Mine_Clearing_Detail(bool set)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (set) {
            (*it)->Set_Weapon_Set_Flag(WEAPONSET_MINE_CLEARING_DETAIL);
        } else {
            (*it)->Clear_Weapon_Set_Flag(WEAPONSET_MINE_CLEARING_DETAIL);
        }
    }
}

bool AIGroup::Set_Weapon_Lock_For_Group(WeaponSlotType wslot, WeaponLockType wlock)
{
    bool locked = false;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if ((*it)->Set_Weapon_Lock(wslot, wlock)) {
            locked = true;
        }
    }

    return locked;
}

void AIGroup::Release_Weapon_Lock_For_Group(WeaponLockType wlock)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        (*it)->Release_Weapon_Lock(wlock);
    }
}

void AIGroup::Set_Weapon_Set_Flag(WeaponSetType wst)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        BitFlags<WEAPONSET_COUNT> flags;
        flags.Set(wst, true);

        if ((*it)->Get_Template()->Find_Weapon_Template_Set(flags)) {
            (*it)->Set_Weapon_Set_Flag(wst);
        }
    }
}

void AIGroup::Queue_Upgrade(UpgradeTemplate *upgrade)
{
    if (upgrade != nullptr) {
        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            Object *member_obj = (*it);

            if (g_theUpgradeCenter->Can_Afford_Upgrade(member_obj->Get_Controlling_Player(), upgrade, false)
                && (upgrade->Get_Type() != UPGRADE_STATUS_IN_PRODUCTION
                    || !member_obj->Has_Upgrade(upgrade) && member_obj->Affected_By_Upgrade(upgrade))) {
                if (member_obj->Can_Produce_Upgrade(upgrade)) {
                    ProductionUpdateInterface *production = member_obj->Get_Production_Update_Interface();

                    if (production != nullptr) {
                        if (production->Can_Queue_Upgrade(upgrade) != CAN_MAKE_QUEUE_FULL) {
                            production->Queue_Upgrade(upgrade);
                        }
                    }
                }
            }
        }
    }
}

bool AIGroup::Is_Idle() const
{
    bool idle = true;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj != nullptr) {
            AIUpdateInterface *update = member_obj->Get_AI_Update_Interface();

            if (update != nullptr) {
                idle = update->Is_Idle() || member_obj->Is_Effectively_Dead();

                if (!idle) {
                    return false;
                }
            }
        }
    }

    return idle;
}

bool AIGroup::Is_Busy() const
{
    bool busy = true;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj != nullptr) {
            AIUpdateInterface *update = member_obj->Get_AI_Update_Interface();

            if (update != nullptr) {
                busy = update->Is_Busy() && !member_obj->Is_Effectively_Dead();

                if (!busy) {
                    return false;
                }
            }
        }
    }

    return busy;
}

bool AIGroup::Is_Group_AI_Dead() const
{
    bool dead = true;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj != nullptr) {
            dead = dead && member_obj->Is_Effectively_Dead();
        }
    }

    return dead;
}

Object *AIGroup::Get_Special_Power_Source_Object(unsigned int special_power_id)
{
    const SpecialPowerTemplate *special_power = g_theSpecialPowerStore->Find_Special_Power_Template_By_ID(special_power_id);

    if (special_power != nullptr) {
        for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
            Object *member_obj = (*it);

            if (member_obj->Get_Special_Power_Module(special_power) != nullptr) {
                return member_obj;
            }
        }
    }

    return nullptr;
}

void AIGroup::Group_Set_Emoticon(const Utf8String &emoticon, int frames)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Drawable *drawable = (*it)->Get_Drawable();

        if (drawable != nullptr) {
            drawable->Set_Emoticon(emoticon, frames);
        }
    }
}

void AIGroup::Group_Override_Special_Power_Destination(
    SpecialPowerType type, const Coord3D *destination, CommandSourceType cmd_source)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj != nullptr) {
            SpecialPowerUpdateInterface *special_power =
                member_obj->Find_Special_Power_With_Overridable_Destination_Active(type);

            if (special_power != nullptr) {
                special_power->Set_Special_Power_Overridable_Destination(destination);
            }
        }
    }
}

Object *AIGroup::Get_Command_Button_Source_Object(GUICommand command_type)
{
    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        Object *member_obj = (*it);

        if (member_obj != nullptr) {
            const CommandSet *command_set = g_theControlBar->Find_Command_Set(member_obj->Get_Command_Set_String());

            if (command_set != nullptr) {
                for (int i = 0; i < 18; i++) {
                    const CommandButton *button = command_set->Get_Command_Button(i);

                    if (button != nullptr && button->Get_Command() == command_type) {
                        return member_obj;
                    }
                }
            }
        }
    }

    return nullptr;
}

void AIGroup::CRC_Snapshot(Xfer *xfer)
{
    ObjectID id = INVALID_OBJECT_ID;

    for (auto it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if ((*it) != nullptr) {
            id = (*it)->Get_ID();
        }

        xfer->xferUser(&id, sizeof(id));
#ifdef GAME_DEBUG_STRUCTS
        // TODO CRC log stuff
#endif
    }

    xfer->xferUnsignedInt(&m_memberListSize);
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRC log stuff
#endif
    id = INVALID_OBJECT_ID;
    xfer->xferObjectID(&id);
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRC log stuff
#endif
    xfer->xferReal(&m_speed);
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRC log stuff
#endif
    xfer->xferBool(&m_dirty);
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRC log stuff
#endif
    xfer->xferUnsignedInt(&m_id);
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRC log stuff
#endif
}

void AIGroup::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

void AIGroup::Load_Post_Process() {}
