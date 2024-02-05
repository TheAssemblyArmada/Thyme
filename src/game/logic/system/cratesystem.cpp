/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Crate System
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cratesystem.h"
#include "science.h"

#ifndef GAME_DLL
CrateSystem *g_theCrateSystem;
#endif

// clang-format off
const FieldParse CrateTemplate::s_theCrateTemplateFieldParseTable[] = {
    {"CreationChance", &INI::Parse_Real, nullptr, offsetof(CrateTemplate, m_creationChance)},
    {"VeterancyLevel", &INI::Parse_Index_List, g_veterancyNames, offsetof(CrateTemplate, m_veterancyLevel)},
    {"KilledByType", &BitFlags<KINDOF_COUNT>::Parse_From_INI, nullptr, offsetof(CrateTemplate, m_killedByTypeKindof)},
    {"CrateObject", &CrateTemplate::Parse_Crate_Creation_Entry, nullptr, 0},
    {"KillerScience", &ScienceStore::Parse_Science, nullptr, offsetof(CrateTemplate, m_killerScience)},
    {"OwnedByMaker", &INI::Parse_Bool, nullptr, offsetof(CrateTemplate, m_ownedByMaker)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

CrateTemplate::CrateTemplate() :
    m_name(""),
    m_creationChance(0.0f),
    m_veterancyLevel(VETERANCY_INVALID),
    m_killerScience(SCIENCE_INVALID),
    m_ownedByMaker(false)
{
}

CrateTemplate::~CrateTemplate()
{
    m_possibleCrates.clear();
}

void CrateTemplate::Parse_Crate_Creation_Entry(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String name(ini->Get_Next_Token());
    const char *str = ini->Get_Next_Token();
    float chance;

    if (sscanf(str, "%f", &chance) != 1) {
        throw CODE_06;
    }

    CrateCreationEntry entry;
    entry.crate_name = name;
    entry.crate_chance = chance;
    static_cast<CrateTemplate *>(formal)->m_possibleCrates.push_back(entry);
}

CrateSystem::CrateSystem()
{
    m_crateTemplateVector.clear();
}

CrateSystem::~CrateSystem()
{
    for (size_t i = 0; i < m_crateTemplateVector.size(); i++) {
        CrateTemplate *tmplate = m_crateTemplateVector[i];

        if (tmplate != nullptr) {
            tmplate->Delete_Instance();
        }
    }

    m_crateTemplateVector.clear();
}

void CrateSystem::Init()
{
    Reset();
}

void CrateSystem::Reset()
{
    for (auto it = m_crateTemplateVector.begin(); it != m_crateTemplateVector.end();) {
        if (*it != nullptr && (*it)->Delete_Overrides() != nullptr) {
            it++;
        } else {
            it = m_crateTemplateVector.erase(it);
        }
    }
}

CrateTemplate *CrateSystem::New_Crate_Template(Utf8String name)
{
    if (name.Is_Empty()) {
        return nullptr;
    } else {
        CrateTemplate *crate = new CrateTemplate();
        const CrateTemplate *default_template = Find_Crate_Template("DefaultCrate");

        if (default_template != nullptr) {
            *crate = *default_template;
        }

        crate->Set_Name(name);
        m_crateTemplateVector.push_back(crate);
        return crate;
    }
}

CrateTemplate *CrateSystem::New_Crate_Template_Override(CrateTemplate *crate)
{
    CrateTemplate *new_crate = new CrateTemplate();
    *new_crate = *crate;
    new_crate->Set_Is_Allocated();
    crate->Set_Next(new_crate);
    return new_crate;
}

const CrateTemplate *CrateSystem::Find_Crate_Template(Utf8String name)
{
    for (unsigned int i = 0; i < m_crateTemplateVector.size(); i++) {
        if (m_crateTemplateVector[i]->Get_Name() == name) {
            return m_crateTemplateVector[i];
        }
    }

    return nullptr;
}

CrateTemplate *CrateSystem::Friend_Find_Crate_Template(Utf8String name)
{
    for (unsigned int i = 0; i < m_crateTemplateVector.size(); i++) {
        if (m_crateTemplateVector[i]->Get_Name() == name) {
            return m_crateTemplateVector[i];
        }
    }

    return nullptr;
}

void CrateSystem::Parse_Crate_Template_Definition(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    CrateTemplate *crate = g_theCrateSystem->Friend_Find_Crate_Template(name);

    if (crate != nullptr) {
        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            crate = g_theCrateSystem->New_Crate_Template_Override(crate);
        } else {
            captainslog_dbgassert(false,
                "[LINE: %d in '%s'] Duplicate crate %s found!",
                ini->Get_Line_Number(),
                ini->Get_Filename().Str(),
                name.Str());
        }
    } else {
        crate = g_theCrateSystem->New_Crate_Template(name);

        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            crate->Set_Is_Allocated();
        }
    }

    ini->Init_From_INI(crate, CrateTemplate::Build_Field_Parse());
}
