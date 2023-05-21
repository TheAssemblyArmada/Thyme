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

#include "always.h"
#include "dict.h"
#include "namekeygenerator.h"
#include <captainslog.h>

// Wrapper around dict with a default ctor.
class TeamsInfo
{
public:
    TeamsInfo() : m_dict(0) {}
    ~TeamsInfo() {}
    Dict *Get_Dict() { return &m_dict; }
    void Init(const Dict *dict)
    {
        m_dict.Clear();
        if (dict) {
            m_dict = *dict;
        }
    }
    void Clear() { Init(nullptr); }

private:
    Dict m_dict;
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
    TeamsInfo *Find_Team_Info(Utf8String name, int *id);
    void Clear();
    int Get_Num_Teams() { return m_numTeams; }
    TeamsInfoRec &operator=(const TeamsInfoRec &that);
    TeamsInfo *Get_Team_Info(int index);

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
            Add_Team(that.m_teams[i].Get_Dict());
        }
    }

    return *this;
}

inline TeamsInfo *TeamsInfoRec::Get_Team_Info(int index)
{
    captainslog_relassert(index >= 0 && index < m_numTeams, 0xDEAD0003, "Out of range.");
    return &m_teams[index];
}
