/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stores information on current generals rank and purchased "sciences".
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
#include "overridable.h"
#include "science.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <vector>

class INI;
class RankInfo;

class RankInfoStore : public SubsystemInterface
{
public:
    virtual ~RankInfoStore() {}

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    RankInfo *Get_Rank_Info(int level);
    int Get_Rank_Level_Count() const;

    static void Parse_Rank_Definition(INI *ini);

#ifdef GAME_DLL
    void Hook_Init() { RankInfoStore::Init(); }
    void Hook_Reset() { RankInfoStore::Reset(); }
#endif
private:
    std::vector<RankInfo *> m_infoStore;
};

class RankInfo : public Overridable
{
    IMPLEMENT_POOL(RankInfo);
    friend void RankInfoStore::Parse_Rank_Definition(INI *);

public:
    virtual ~RankInfo() {}

    RankInfo *Get_Override()
    {
        return m_next != nullptr ? reinterpret_cast<RankInfo *>(m_next->Get_Final_Override()) : this;
    }

private:
    Utf16String m_rankName;
    int m_skillPointsNeeded;
    unsigned m_sciencePurchasePointsGranted;
    std::vector<ScienceType> m_sciencesGranted;
};

#ifdef GAME_DLL
extern RankInfoStore *&g_theRankInfoStore;
#else
extern RankInfoStore *g_theRankInfoStore;
#endif
