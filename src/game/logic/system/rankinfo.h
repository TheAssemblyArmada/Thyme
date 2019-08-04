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
    static void Hook_Me();
    void Hook_Init();
    void Hook_Reset();
#endif
private:
    std::vector<RankInfo *> m_infoStore;
};

class RankInfo : public Overridable
{
    IMPLEMENT_POOL(RankInfo);
    friend void RankInfoStore::Parse_Rank_Definition(INI*);

public:
    virtual ~RankInfo() {}

    RankInfo *Get_Override() { return m_next != nullptr ? reinterpret_cast<RankInfo *>(m_next->Get_Final_Override()) : this; }
private:
    Utf16String m_rankName;
    int m_skillPointsNeeded;
    unsigned m_sciencePurchasePointsGranted;
    std::vector<ScienceType> m_sciencesGranted;
};

#ifdef GAME_DLL
#include "hooker.h"
extern RankInfoStore *&g_theRankInfoStore;

inline void RankInfoStore::Hook_Init()
{
    RankInfoStore::Init();
}

inline void RankInfoStore::Hook_Reset()
{
    RankInfoStore::Reset();
}

inline void RankInfoStore::Hook_Me()
{
    Hook_Function(0x00489520, &RankInfoStore::Parse_Rank_Definition);
    //Hook_Method(0x00489410, &RankInfoStore::Hook_Init); // Works, but shares function in binary with ScienceStore as compiles to same code.
    Hook_Method(0x00489440, &RankInfoStore::Hook_Reset);
    Hook_Method(0x004894E0, &RankInfoStore::Get_Rank_Info);
    Hook_Method(0x004894D0, &RankInfoStore::Get_Rank_Level_Count);
}
#else
extern RankInfoStore *g_theRankInfoStore;
#endif
