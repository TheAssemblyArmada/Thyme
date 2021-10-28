/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Handles abilities granted by gaining experience points.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "science.h"
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
ScienceStore *g_theScienceStore = nullptr;
#endif

FieldParse ScienceInfo::s_scienceFieldParseTable[] = {
    { "PrerequisiteSciences", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(ScienceInfo, m_prerequisites) },
    { "SciencePurchasePointCost", &INI::Parse_Int, nullptr, offsetof(ScienceInfo, m_purchaseCost) },
    { "IsGrantable", &INI::Parse_Bool, nullptr, offsetof(ScienceInfo, m_isGrantable) },
    { "DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(ScienceInfo, m_displayName) },
    { "Description", &INI::Parse_And_Translate_Label, nullptr, offsetof(ScienceInfo, m_description) },
    { nullptr, nullptr, nullptr, 0 }
};

ScienceInfo::ScienceInfo() :
    m_nameKey((NameKeyType)-1),
    m_displayName(),
    m_description(),
    m_unkVec1(),
    m_prerequisites(),
    m_purchaseCost(0),
    m_isGrantable(true)
{
}

void ScienceStore::Reset()
{
    for (auto it = m_infoVec.begin(); it != m_infoVec.end(); ++it) {
        if (*it != nullptr) {
            *it = reinterpret_cast<ScienceInfo *>((*it)->Delete_Overrides());
        }

        if (*it == nullptr) {
            m_infoVec.erase(it);
        }
    }
}

ScienceType ScienceStore::Lookup_Science(const char *name)
{
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(name);

    for (auto it = m_infoVec.begin(); it != m_infoVec.end(); ++it) {
        ScienceInfo *info = (ScienceInfo *)((*it)->Get_Final_Override());

        if (info->Check_Name_Key(key)) {
            return ScienceType(key);
        }
    }

    captainslog_relassert(false, 0xDEAD0006, "Failed to find matching ScienceType in Science Store.");
    return SCIENCE_INVALID;
}

// zh: 0x0041B2C0 wb: 0x007A28ED
// Previously INI::Parse_Science_Vector
void ScienceStore::Parse_Science_Vector(INI *ini, void *, void *store, const void *)
{
    std::vector<ScienceType> *sci_vec = static_cast<std::vector<ScienceType> *>(store);
    sci_vec->clear();

    for (const char *token = ini->Get_Next_Token_Or_Null(); token != nullptr; token = ini->Get_Next_Token_Or_Null()) {
        if (strcasecmp(token, "None") == 0) {
            sci_vec->clear();

            return;
        }

        sci_vec->push_back(g_theScienceStore->Lookup_Science(token));
    }
}
