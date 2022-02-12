/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for keeping score of a player.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "always.h"
#include "scorekeeper.h"
#include "gamelogic.h"
#include "object.h"
#include "player.h"
#include "thing.h"
#include "thingfactory.h"
#include "xfer.h"

BitFlags<KINDOF_COUNT> ScoringBuildingCreateMask;
BitFlags<KINDOF_COUNT> ScoringBuildingDestroyMask;
BitFlags<KINDOF_COUNT> ScoringBuildingMask;

ScoreKeeper::ScoreKeeper()
{
    Reset(0);
}

void ScoreKeeper::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    xfer->xferInt(&m_totalMoneyEarned);
    xfer->xferInt(&m_totalMoneySpent);

    xfer->xferUser(&m_totalUnitsDestroyed, sizeof(m_totalUnitsDestroyed));
    xfer->xferInt(&m_totalUnitsBuilt);
    xfer->xferInt(&m_totalUnitsLost);

    xfer->xferUser(&m_totalBuildingsDestroyed, sizeof(m_totalBuildingsDestroyed));
    xfer->xferInt(&m_totalBuildingsBuilt);
    xfer->xferInt(&m_totalBuildingsLost);

    xfer->xferInt(&m_totalTechBuildingsCaptured);
    xfer->xferInt(&m_totalScoredBuildingsCaptured);

    xfer->xferInt(&m_currentScore);

    xfer->xferInt(&m_playerIndex);

    Xfer_Object_Count_Map(xfer, &m_objectsBuilt);

    uint16_t count = MAX_PLAYER_COUNT;

    xfer->xferUnsignedShort(&count);

    captainslog_relassert(count == MAX_PLAYER_COUNT, 6, "size of objects destroyed array has changed");

    for (uint16_t i = 0; i < count; ++i) {
        Xfer_Object_Count_Map(xfer, &m_objectsDestroyed[i]);
    }

    Xfer_Object_Count_Map(xfer, &m_objectsLost);
    Xfer_Object_Count_Map(xfer, &m_objectsCaptured);
}

void ScoreKeeper::Reset(int player_index)
{
    ScoringBuildingMask.Set(KINDOF_STRUCTURE, true);
    ScoringBuildingMask.Set(KINDOF_SCORE, true);

    ScoringBuildingCreateMask.Set(KINDOF_STRUCTURE, true);
    ScoringBuildingCreateMask.Set(KINDOF_SCORE_CREATE, true);

    ScoringBuildingDestroyMask.Set(KINDOF_STRUCTURE, true);
    ScoringBuildingDestroyMask.Set(KINDOF_SCORE_DESTROY, true);

    m_totalMoneyEarned = 0;
    m_totalMoneySpent = 0;

    m_totalUnitsBuilt = 0;
    m_totalUnitsLost = 0;

    m_totalBuildingsBuilt = 0;
    m_totalBuildingsLost = 0;

    m_totalTechBuildingsCaptured = 0;
    m_totalScoredBuildingsCaptured = 0;

    m_currentScore = 0;

    // needs confirming
    m_objectsBuilt.clear();
    m_objectsLost.clear();
    m_objectsCaptured.clear();

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i) {
        // needs confirming
        m_objectsDestroyed[i].clear();
        m_totalUnitsDestroyed[i] = 0;
        m_totalBuildingsDestroyed[i] = 0;
    }

    m_playerIndex = player_index;
}

int ScoreKeeper::Calculate_Score()
{
    int score = 0;

    score += m_totalMoneyEarned * SUPPLIES_COLLECTED_MULTIPLIER;
    score += m_totalUnitsBuilt * UNITS_BUILT_MULTIPLIER;
    score += m_totalBuildingsBuilt * BUILDINGS_BUILT_MULTIPLIER;

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i) {
        if (i != m_playerIndex) {
            score += m_totalUnitsDestroyed[i] * UNITS_DESTROYED_MULTIPLIER;
            score += m_totalBuildingsDestroyed[i] * BUILDINGS_DESTROYED_MULTIPLIER;
        }
    }

    m_currentScore = score;

    return m_currentScore;
}

void ScoreKeeper::Add_Object_Built(const Object *object)
{
    if (g_theGameLogic->Get_Enable_Scoring()) {

        bool alter_count = false;

        const ThingTemplate *tmpl = object->Get_Template();

        if (tmpl->Is_KindOf_Multi(ScoringBuildingMask, KINDOFMASK_NONE)) {

            ++m_totalBuildingsBuilt;
            alter_count = true;

        } else {
            if (tmpl->Is_KindOf_Multi(ScoringBuildingCreateMask, KINDOFMASK_NONE)) {

                ++m_totalBuildingsBuilt;
                alter_count = true;

            } else {

                if (tmpl->Is_KindOf(KINDOF_INFANTRY) || tmpl->Is_KindOf(KINDOF_VEHICLE)) {

                    if (tmpl->Is_KindOf(KINDOF_SCORE) || tmpl->Is_KindOf(KINDOF_SCORE_CREATE)) {
                        ++m_totalUnitsBuilt;
                        alter_count = true;
                    }
                }
            }
        }

        if (alter_count) {
            int32_t count = 0;

            auto iter = m_objectsBuilt.find(tmpl);

            if (iter != m_objectsBuilt.end()) {
                count = (*iter).second;
            }

            int32_t new_count = count + 1;
            m_objectsBuilt[tmpl] = new_count;
        }
    }
}

void ScoreKeeper::Remove_Object_Built(const Object *object)
{
    if (g_theGameLogic->Get_Enable_Scoring()) {

        bool alter_count = false;

        const ThingTemplate *tmpl = object->Get_Template();

        if (tmpl->Is_KindOf_Multi(ScoringBuildingMask, KINDOFMASK_NONE)) {

            --m_totalBuildingsBuilt;
            alter_count = true;

        } else {
            if (tmpl->Is_KindOf_Multi(ScoringBuildingCreateMask, KINDOFMASK_NONE)) {

                --m_totalBuildingsBuilt;
                alter_count = true;

            } else {

                if (tmpl->Is_KindOf(KINDOF_INFANTRY) || tmpl->Is_KindOf(KINDOF_VEHICLE)) {

                    if (tmpl->Is_KindOf(KINDOF_SCORE) || tmpl->Is_KindOf(KINDOF_SCORE_CREATE)) {
                        --m_totalUnitsBuilt;
                        alter_count = true;
                    }
                }
            }
        }

        if (alter_count) {
            int32_t count = 0;

            auto iter = m_objectsBuilt.find(tmpl);

            if (iter != m_objectsBuilt.end()) {
                count = (*iter).second;
            }

            int32_t new_count = count - 1;
            m_objectsBuilt[tmpl] = new_count;
        }
    }
}

int ScoreKeeper::Get_Total_Units_Built(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    int total = 0;

    for (auto &it : m_objectsBuilt) {
        if (it.first->Is_KindOf_Multi(must_be_set, must_be_clear)) {
            total += it.second;
        }
    }

    return total;
}

int ScoreKeeper::Get_Total_Objects_Built(const ThingTemplate *tmpl)
{
    int total = 0;

    for (auto &it : m_objectsBuilt) {
        if (it.first->Is_Equivalent_To(tmpl)) {
            ++total;
        }
    }

    return total;
}

void ScoreKeeper::Add_Object_Captured(const Object *object)
{
    if (g_theGameLogic->Get_Enable_Scoring()) {
        bool alter_count = false;
        const ThingTemplate *tmpl = object->Get_Template();

        if (tmpl->Is_KindOf(KINDOF_STRUCTURE)) {

            if (tmpl->Is_KindOf(KINDOF_SCORE)) {
                ++m_totalScoredBuildingsCaptured;
            } else {
                ++m_totalTechBuildingsCaptured;
            }

            alter_count = true;
        }

        if (alter_count) {
            int32_t count = 0;

            auto iter = m_objectsCaptured.find(tmpl);

            if (iter != m_objectsCaptured.end()) {
                count = (*iter).second;
            }

            int32_t new_count = count + 1;
            m_objectsCaptured[tmpl] = new_count;
        }
    }
}

void ScoreKeeper::Add_Object_Destroyed(const Object *object)
{
    if (g_theGameLogic->Get_Enable_Scoring()) {

        int player_idx = object->Get_Controlling_Player()->Get_Player_Index();
        bool alter_count = false;

        const ThingTemplate *tmpl = object->Get_Template();

        if (tmpl->Is_KindOf_Multi(ScoringBuildingMask, KINDOFMASK_NONE)) {

            if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                ++m_totalBuildingsDestroyed[player_idx];
                alter_count = true;
            }

        } else {
            if (tmpl->Is_KindOf_Multi(ScoringBuildingDestroyMask, KINDOFMASK_NONE)) {

                if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                    ++m_totalBuildingsDestroyed[player_idx];
                    alter_count = true;
                }

            } else {

                if (tmpl->Is_KindOf(KINDOF_INFANTRY) || tmpl->Is_KindOf(KINDOF_VEHICLE)) {

                    if (tmpl->Is_KindOf(KINDOF_SCORE) || tmpl->Is_KindOf(KINDOF_SCORE_DESTROY)) {

                        if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                            ++m_totalUnitsDestroyed[player_idx];
                            alter_count = true;
                        }
                    }
                }
            }
        }

        if (alter_count) {
            int32_t count = 0;

            auto iter = m_objectsDestroyed[player_idx].find(tmpl);

            if (iter != m_objectsDestroyed[player_idx].end()) {
                count = (*iter).second;
            }

            int32_t new_count = count + 1;
            m_objectsDestroyed[player_idx][tmpl] = new_count;
        }
    }
}

void ScoreKeeper::Add_Object_Lost(const Object *object)
{
    if (g_theGameLogic->Get_Enable_Scoring()) {

        bool alter_count = false;

        const ThingTemplate *tmpl = object->Get_Template();

        if (tmpl->Is_KindOf_Multi(ScoringBuildingMask, KINDOFMASK_NONE)) {

            if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                ++m_totalBuildingsLost;
                alter_count = true;
            }

        } else {
            if (tmpl->Is_KindOf_Multi(ScoringBuildingDestroyMask, KINDOFMASK_NONE)) {

                if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                    ++m_totalBuildingsLost;
                    alter_count = true;
                }

            } else {

                if (tmpl->Is_KindOf(KINDOF_INFANTRY) || tmpl->Is_KindOf(KINDOF_VEHICLE)) {

                    if (tmpl->Is_KindOf(KINDOF_SCORE) || tmpl->Is_KindOf(KINDOF_SCORE_DESTROY)) {

                        if (!object->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                            ++m_totalUnitsLost;
                            alter_count = true;
                        }
                    }
                }
            }
        }

        if (alter_count) {
            int32_t count = 0;

            auto iter = m_objectsLost.find(tmpl);

            if (iter != m_objectsLost.end()) {
                count = (*iter).second;
            }

            int32_t new_count = count + 1;
            m_objectsLost[tmpl] = new_count;
        }
    }
}

int ScoreKeeper::Get_Total_Units_Destroyed() const
{
    int total = 0;

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i) {
        total += m_totalUnitsDestroyed[i];
    }

    return total;
}

int ScoreKeeper::Get_Total_Buildings_Destroyed() const
{
    int total = 0;

    for (int i = 0; i < MAX_PLAYER_COUNT; ++i) {
        total += m_totalBuildingsDestroyed[i];
    }

    return total;
}

void ScoreKeeper::Xfer_Object_Count_Map(Xfer *xfer, ScoreKeeperCountMap *count_map)
{
    captainslog_relassert(count_map, 6, "Invalid map parameter");

    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    // i think?
    uint16_t count = static_cast<uint16_t>(count_map->size());
    xfer->xferUnsignedShort(&count);

    Utf8String name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto &pair : *count_map) {
            name = pair.first->Get_Name();
            xfer->xferAsciiString(&name);
            int32_t value = pair.second;
            xfer->xferInt(&value);
        }
    } else {
        for (uint16_t i = 0; i < count; ++i) {
            xfer->xferAsciiString(&name);
            ThingTemplate *tmpl = g_theThingFactory->Find_Template(name, true);

            captainslog_relassert(tmpl != nullptr, 6, "Unknown thing template '%s'", name.Str());

            int32_t value;
            xfer->xferInt(&value);
            (*count_map)[tmpl] = value;
        }
    }
}