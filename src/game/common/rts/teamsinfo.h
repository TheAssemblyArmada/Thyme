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
#pragma once

#ifndef TEAMSINFO_H
#define TEAMSINFO_H

#include "always.h"
#include "dict.h"
#include "namekeygenerator.h"

// Wrapper around dict with a default ctor.
struct TeamsInfo
{
    TeamsInfo() : dict(0) {}
    Dict dict;
};

class TeamsInfoRec
{
    enum
    {
        TEAMINFO_GROWTH_STEP = 8,
    };

public:
    TeamsInfoRec() : m_numTeams(0), m_numTeamsAllocated(0), m_teams(nullptr) {}
    TeamsInfoRec(const TeamsInfoRec &that);
    ~TeamsInfoRec();

    void Add_Team(const Dict *team);
    void Remove_Team(int id);
    TeamsInfo *Find_Team(AsciiString name, int *id);
    void Clear();

    TeamsInfoRec &operator=(const TeamsInfoRec &that);

private:
    int m_numTeams;
    int m_numTeamsAllocated;
    TeamsInfo *m_teams;
};

inline TeamsInfoRec &TeamsInfoRec::operator=(const TeamsInfoRec &that)
{
    if (this != &that) {
        Clear();

        for (int i = 0; i < that.m_numTeams; ++i) {
            Add_Team(&that.m_teams[i].dict);
        }
    }

    return *this;
}

// TODO Move this if more appropriate location found.
#ifndef THYME_STANDALONE
extern StaticNameKey &g_theTeamNameKey;
#else
extern StaticNameKey g_theTeamNameKey;
#endif

#endif // TEAMSINFO_H
