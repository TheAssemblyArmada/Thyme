/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Squad
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "squad.h"
#include "ai.h"
#include "gamelogic.h"
#include "team.h"
#include <algorithm>

void Squad::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short object_id_count = static_cast<unsigned short>(m_objectIDs.size());
    xfer->xferUnsignedShort(&object_id_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_objectIDs.begin(); i != m_objectIDs.end(); i++) {
            ObjectID id = *i;
            xfer->xferObjectID(&id);
        }
    } else {
        captainslog_relassert(m_objectsCached.size() == 0, 6, "Squad::xfer - m_objectsCached should be emtpy, but is not");
        for (int i = 0; i < object_id_count; i++) {
            ObjectID id;
            xfer->xferObjectID(&id);
            m_objectIDs.push_back(id);
        }
    }
}

bool Squad::Is_On_Squad(const Object *obj) const
{
    ObjectID id = obj->Get_ID();

    for (auto i = m_objectIDs.begin(); i != m_objectIDs.end(); i++) {
        if (id == (*i)) {
            return true;
        }
    }

    return false;
}

void Squad::Clear_Squad()
{
    m_objectIDs.clear();
    m_objectsCached.clear();
}

void Squad::Add_Object(Object *obj)
{
    if (obj != nullptr) {
        m_objectIDs.push_back(obj->Get_ID());
    }
}

void Squad::Add_Object_ID(ObjectID id)
{
    m_objectIDs.push_back(id);
}

void Squad::Remove_Object(Object *obj)
{
    if (obj != nullptr) {
        auto it = std::find(m_objectIDs.begin(), m_objectIDs.end(), obj->Get_ID());

        if (it != m_objectIDs.end()) {
            m_objectIDs.erase(it);
        }
    }
}

std::vector<Object *> &Squad::Get_All_Objects()
{
    m_objectsCached.clear();

    for (auto it = m_objectIDs.begin(); it != m_objectIDs.end();) {
        Object *obj = g_theGameLogic->Find_Object_By_ID(*it);

        if (obj != nullptr) {
            m_objectsCached.push_back(obj);
            it++;
        } else {
            it = m_objectIDs.erase(it);
        }
    }

    return m_objectsCached;
}

std::vector<Object *> &Squad::Get_Live_Objects()
{
    Get_All_Objects();

    for (auto it = m_objectsCached.begin(); it != m_objectsCached.end();) {
        if ((*it)->Is_Selectable()) {
            it++;
        } else {
            it = m_objectsCached.erase(it);
        }
    }

    return m_objectsCached;
}

int Squad::Get_Size_Of_Group() const
{
    return static_cast<int>(m_objectIDs.size());
}

void Squad::Squad_From_Team(const Team *from_team, bool clear_squad_first)
{
    if (from_team != nullptr) {
        if (clear_squad_first) {
            m_objectIDs.clear();
        }

        DLINK_ITERATOR<Object> obj_it = from_team->Iterate_Team_Member_List();

        while (!obj_it.Done()) {
            Object *obj = obj_it.Cur();

            if (obj != nullptr) {
                m_objectIDs.push_back(obj->Get_ID());
            }

            obj_it.Advance();
        }
    }
}

void Squad::Squad_From_AI_Group(const AIGroup *from_ai_group, bool clear_squad_first)
{
    if (from_ai_group != nullptr) {
        if (clear_squad_first) {
            m_objectIDs.clear();
        }

        m_objectIDs = from_ai_group->Get_All_IDs();
    }
}

void Squad::AI_Group_From_Squad(AIGroup *ai_group_to_fill)
{
    if (ai_group_to_fill != nullptr) {
        Get_Live_Objects();

        for (auto it = m_objectsCached.begin(); it != m_objectsCached.end(); it++) {
            ai_group_to_fill->Add(*it);
        }
    }
}
