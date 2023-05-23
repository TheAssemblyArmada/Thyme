/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for keeping track of teams.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "teamsinfo.h"
#include "staticnamekey.h"

TeamsInfoRec::TeamsInfoRec(const TeamsInfoRec &that) : m_numTeams(0), m_numTeamsAllocated(0), m_teams(nullptr)
{
    *this = that;
}

TeamsInfoRec::~TeamsInfoRec()
{
    Clear();
}

/**
 * @brief Adds a team from a Dict.
 */
void TeamsInfoRec::Add_Team(const Dict *team)
{
    // If we have too many teams for our current allocation to handle, reallocate more.
    if (m_numTeams >= m_numTeamsAllocated) {
        TeamsInfo *ti = new TeamsInfo[m_numTeamsAllocated + TEAMINFO_GROWTH_STEP];

        // Copy existing data across to new array.
        int i;
        for (i = 0; i < m_numTeams; ++i) {
            ti[i] = m_teams[i];
        }

        for (; i < m_numTeamsAllocated + TEAMINFO_GROWTH_STEP; ++i) {
            ti[i].Clear();
        }

        // Delete old array and replace with new one.
        if (m_teams != nullptr) {
            delete[] m_teams;
        }

        m_teams = ti;
        m_numTeamsAllocated += TEAMINFO_GROWTH_STEP;
    }

    m_teams[m_numTeams].Clear();

    if (team != nullptr) {
        m_teams[m_numTeams].Init(team);
    }

    ++m_numTeams;
}

/**
 * @brief Removes a team by id.
 */
void TeamsInfoRec::Remove_Team(int id)
{
    // Check if ID is actually within our array.
    if (id < 0 || id >= m_numTeams || m_numTeams <= 0) {
        return;
    }

    // Shuffle remaining data down.
    for (; id < m_numTeams - 1; ++id) {
        m_teams[id] = m_teams[id + 1];
    }

    // Clear the rest of the array.
    for (; id < m_numTeamsAllocated; ++id) {
        m_teams[id].Clear();
    }

    --m_numTeams;
}

/**
 * @brief Finds team info by name.
 */
TeamsInfo *TeamsInfoRec::Find_Team_Info(Utf8String name, int *id)
{
    if (m_numTeams <= 0) {
        return nullptr;
    }

    for (int i = 0; i < m_numTeams; ++i) {
        NameKeyType key = g_teamNameKey.Key();
        Utf8String string = m_teams[i].Get_Dict()->Get_AsciiString(key);

        if (string == name) {
            if (id != nullptr) {
                *id = i;
            }

            return &m_teams[i];
        }
    }

    return nullptr;
}

/**
 * @brief Clears all team records and frees allocated memory.
 */
void TeamsInfoRec::Clear()
{
    // Clear all the dicts
    for (int i = 0; i < m_numTeamsAllocated; ++i) {
        m_teams[i].Clear();
    }

    m_numTeams = 0;
    m_numTeamsAllocated = 0;

    if (m_teams != nullptr) {
        delete[] m_teams;
        m_teams = nullptr;
    }
}
