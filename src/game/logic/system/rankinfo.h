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
    RankInfoStore() {}
    virtual ~RankInfoStore() {}

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    const RankInfo *Get_Rank_Info(int level) const;
    int Get_Rank_Level_Count() const;

    static void Parse_Rank_Definition(INI *ini);

private:
    std::vector<RankInfo *> m_infoStore;
};

class RankInfo : public Overridable
{
    IMPLEMENT_POOL(RankInfo);
    friend void RankInfoStore::Parse_Rank_Definition(INI *);

protected:
    // #BUGFIX Initialize all members
    RankInfo() : m_rankName(), m_skillPointsNeeded{}, m_sciencePurchasePointsGranted{}, m_sciencesGranted() {}
    virtual ~RankInfo() override {}

private:
    Utf16String m_rankName;
    int m_skillPointsNeeded;
    unsigned m_sciencePurchasePointsGranted;
    std::vector<ScienceType> m_sciencesGranted;
    friend class Player;
};

#ifdef GAME_DLL
extern RankInfoStore *&g_theRankInfoStore;
#else
extern RankInfoStore *g_theRankInfoStore;
#endif
