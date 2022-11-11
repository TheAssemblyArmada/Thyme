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
#include "rankinfo.h"
#include "ini.h"
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
RankInfoStore *g_theRankInfoStore = nullptr;
#endif

/**
 * @brief Initialise subsystem.
 *
 * 0x00489410
 */
void RankInfoStore::Init()
{
    m_infoStore.clear();
}

/**
 * @brief Reset subsystem.
 *
 * 0x00489440
 */
void RankInfoStore::Reset()
{
    for (auto it = m_infoStore.begin(); it != m_infoStore.end();) {
        if ((*it)->Delete_Overrides() == nullptr) {
            // #BUGFIX invalidated iterator dereference.
            it = m_infoStore.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief Get rank info for a given level.
 *
 * 0x004894E0
 */
const RankInfo *RankInfoStore::Get_Rank_Info(int level) const
{
    if (level < 1 || (unsigned)level > m_infoStore.size() || m_infoStore[level - 1] == nullptr) {
        return nullptr;
    }

    return static_cast<const RankInfo *>(m_infoStore[level - 1]->Get_Final_Override());
}

/**
 * @brief Get number of levels available.
 *
 * 0x004894D0
 */
int RankInfoStore::Get_Rank_Level_Count() const
{
    return m_infoStore.size();
}

/**
 * @brief Parses rank information from an ini file.
 * Was originally RankInfoStore::friend_parseRankDefinition
 * Was originally called through INI::parseRankDefinition
 *
 * 0x00489520
 */
void RankInfoStore::Parse_Rank_Definition(INI *ini)
{
    // clang-format off
    static const FieldParse _parse_table[] = {
        { "RankName", &INI::Parse_And_Translate_Label, nullptr, offsetof(RankInfo, m_rankName) },
        { "SkillPointsNeeded", &INI::Parse_Int, nullptr, offsetof(RankInfo, m_skillPointsNeeded) },
        { "SciencesGranted", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(RankInfo, m_sciencesGranted) },
        { "SciencePurchasePointsGranted", &INI::Parse_Unsigned_Int, nullptr, offsetof(RankInfo, m_sciencePurchasePointsGranted) },
        { nullptr, nullptr, nullptr, 0 }
    };
    // clang-format on

    if (g_theRankInfoStore == nullptr) {
        return;
    }

    int rank_level = INI::Scan_Int(ini->Get_Next_Token());

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        captainslog_relassert(rank_level >= 1 && (unsigned)rank_level <= g_theRankInfoStore->m_infoStore.size(),
            0xDEAD0006,
            "%s, Line: %d - Rank is not within expected range, must be between 1 and %d.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number(),
            g_theRankInfoStore->m_infoStore.size());

        RankInfo *current_info = g_theRankInfoStore->m_infoStore[rank_level - 1];

        captainslog_relassert(current_info != nullptr,
            0xDEAD0006,
            "%s, Line: %d - Cannot create override as no rank already exists at this level.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        RankInfo *new_info = NEW_POOL_OBJ(RankInfo);
        RankInfo *override_info = static_cast<RankInfo *>(current_info->Friend_Get_Final_Override());
        *new_info = *override_info;
        override_info->Set_Next(new_info);
        override_info->Set_Is_Allocated();
        ini->Init_From_INI(new_info, _parse_table);
    } else {
        captainslog_relassert(rank_level == g_theRankInfoStore->m_infoStore.size() + 1,
            0xDEAD0006,
            "%s, Line: %d - Rank is not in sequence, check for missing Rank %d.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number(),
            rank_level - 1);

        RankInfo *new_info = NEW_POOL_OBJ(RankInfo);
        ini->Init_From_INI(new_info, _parse_table);
        g_theRankInfoStore->m_infoStore.push_back(new_info);
    }
}
