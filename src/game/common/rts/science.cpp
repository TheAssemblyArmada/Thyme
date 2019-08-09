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
#include <cstddef>

#ifndef GAME_DLL
ScienceStore *g_theScienceStore = nullptr;
#endif

FieldParse ScienceInfo::s_scienceFieldParseTable[] = {
    { "PrerequisiteSciences", &INI::Parse_Science_Vector, nullptr, offsetof(ScienceInfo, m_prerequisites) },
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
        ScienceInfo *info = (*it)->Get_Override();

        if (info->Check_Name_Key(key)) {
            return ScienceType(key);
        }
    }

    DEBUG_ASSERT_THROW(false, 0xDEAD0006, "Failed to find matching ScienceType in Science Store.\n");
}
