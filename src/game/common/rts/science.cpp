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
#include "ini.h"
#include "namekeygenerator.h"
#include <algorithm>
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
ScienceStore *g_theScienceStore = nullptr;
#endif

const FieldParse ScienceInfo::s_scienceFieldParseTable[] = {
    { "PrerequisiteSciences", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(ScienceInfo, m_prerequisites) },
    { "SciencePurchasePointCost", &INI::Parse_Int, nullptr, offsetof(ScienceInfo, m_purchaseCost) },
    { "IsGrantable", &INI::Parse_Bool, nullptr, offsetof(ScienceInfo, m_isGrantable) },
    { "DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(ScienceInfo, m_displayName) },
    { "Description", &INI::Parse_And_Translate_Label, nullptr, offsetof(ScienceInfo, m_description) },
    { nullptr, nullptr, nullptr, 0 }
};

ScienceInfo::ScienceInfo() :
    m_scienceType(ScienceType::SCIENCE_INVALID),
    m_displayName(),
    m_description(),
    m_rootSciences(),
    m_prerequisites(),
    m_purchaseCost(0),
    m_isGrantable(true)
{
}

// zh: 0x00488870 wb: 0x0072741C
void ScienceInfo::Add_Root_Sciences(std::vector<ScienceType> &rootScience) const
{
    if (m_prerequisites.empty()) {
        auto res = std::find(rootScience.begin(), rootScience.end(), m_scienceType);
        if (res == rootScience.end()) {
            rootScience.push_back(m_scienceType);
        }
    } else {
        for (auto science : m_prerequisites) {
            const auto *info = g_theScienceStore->Get_Science_Info(science);
            info->Add_Root_Sciences(rootScience);
        }
    }
}

void ScienceStore::Reset()
{
    for (auto it = m_infoVec.begin(); it != m_infoVec.end();) {
        if ((*it)->Delete_Overrides() == nullptr) {
            // #BUGFIX invalidated iterator dereference.
            it = m_infoVec.erase(it);
        } else {
            ++it;
        }
    }
}

ScienceType ScienceStore::Lookup_Science(const char *name)
{
    ScienceType key = static_cast<ScienceType>(g_theNameKeyGenerator->Name_To_Key(name));

    captainslog_relassert(
        Has_Science(key), 0xDEAD0006, "Science name %s not known! (Did you define it in science.ini?)", name);
    return key;
}

// zh: 0x004887F0 wb: 0x0072727E
ScienceType ScienceStore::Get_Science_From_Internal_Name(const Utf8String &name) const
{
    if (name.Is_Empty()) {
        return ScienceType::SCIENCE_INVALID;
    }
    return static_cast<ScienceType>(g_theNameKeyGenerator->Name_To_Key(name.Str()));
}

// zh: 0x00488820 wb: 0x007272BE
Utf8String ScienceStore::Get_Internal_Name_From_Science(ScienceType science) const
{
    if (science == ScienceType::SCIENCE_INVALID) {
        return Utf8String::s_emptyString;
    }
    return g_theNameKeyGenerator->Key_To_Name(static_cast<NameKeyType>(science));
}

// wb: 0x0072731A
std::vector<Utf8String> ScienceStore::Get_All_Sciences() const
{
    std::vector<Utf8String> all_science;
    for (const auto *science : m_infoVec) {
        const auto key = static_cast<const ScienceInfo *>(science->Get_Final_Override())->Get_Science_Type();
        all_science.push_back(g_theNameKeyGenerator->Key_To_Name(static_cast<NameKeyType>(key)));
    }
    return all_science;
}

// zh: 0x00488C40 wb: 0x00727753
int32_t ScienceStore::Get_Science_Purchase_Cost(ScienceType science) const
{
    const auto *info = Get_Science_Info(science);
    return info != nullptr ? info->Get_Purchase_Cost() : 0;
}

// zh: 0x00488C90 wb: 0x00727781
bool ScienceStore::Is_Science_Grantable(ScienceType science) const
{
    const auto *info = Get_Science_Info(science);
    return info != nullptr ? info->Is_Grantable() : false;
}

// zh: 0x00488CE0 wb: 0x007277AF
bool ScienceStore::Get_Name_And_Description(ScienceType science, Utf16String &name, Utf16String &description)
{
    const auto *info = Get_Science_Info(science);
    if (info == nullptr) {
        return false;
    }
    name = info->Get_Name();
    description = info->Get_Description();
    return true;
}

// zh: 0x00488D50 wb: 0x007277F7
bool ScienceStore::Player_Has_Prereqs_For_Science(const Player *player, ScienceType science)
{
    // TODO: Requires further implementation of Player
#ifdef GAME_DLL
    return Call_Method<bool, ScienceStore, const Player *, ScienceType>(
        PICK_ADDRESS(0x00488D50, 0x007277F7), this, player, science);
#else
    return false;
#endif
}

// zh: 0x00488DC0 wb: 0x00727867
bool ScienceStore::Player_Has_Root_Prereqs_For_Science(const Player *player, ScienceType science)
{
    // TODO: Requires further implementation of Player
#ifdef GAME_DLL
    return Call_Method<bool, ScienceStore, const Player *, ScienceType>(
        PICK_ADDRESS(0x00488DC0, 0x00727867), this, player, science);
#else
    return false;
#endif
}

// zh: 0x00488950 wb: 0x0072752B
void ScienceStore::Parse_Science_Definition(INI *ini)
{
    const auto *name = ini->Get_Next_Token();
    const auto key = static_cast<ScienceType>(g_theNameKeyGenerator->Name_To_Key(name));

    if (g_theScienceStore == nullptr) {
        return;
    }

    ScienceInfo *found_info = nullptr;
    for (auto *info : g_theScienceStore->m_infoVec) {
        if (info->Check_Science_Type(key)) {
            found_info = info;
            break;
        }
    }

    if (ini->Get_Load_Type() == INILoadType::INI_LOAD_CREATE_OVERRIDES) {
        auto *new_info = new ScienceInfo;
        if (found_info == nullptr) {
            new_info->Set_Is_Allocated();
            g_theScienceStore->m_infoVec.push_back(found_info);
        } else {
            found_info = static_cast<ScienceInfo *>(found_info->Friend_Get_Final_Override());
            *new_info = *found_info;
            found_info->Set_Next(new_info);
            new_info->Set_Is_Allocated();
        }
        found_info = new_info;
    } else {
        captainslog_dbgassert(found_info == nullptr, "Duplicate Science %s!", name);
        found_info = new ScienceInfo;
        g_theScienceStore->m_infoVec.push_back(found_info);
    }

    ini->Init_From_INI(found_info, ScienceInfo::Get_Field_Parse());
    found_info->Set_Science_Type(key);
    found_info->Add_Root_Sciences(found_info->Get_Root_Sciences());
}

// wb: 0x007274D2
const ScienceInfo *ScienceStore::Get_Science_Info(ScienceType science) const
{
    auto res = std::find_if(m_infoVec.begin(), m_infoVec.end(), [=](const ScienceInfo *info) {
        info = static_cast<const ScienceInfo *>(info->Get_Final_Override());
        return info->Check_Science_Type(science);
    });

    return res != m_infoVec.end() ? *res : nullptr;
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

void ScienceStore::Parse_Science(INI *ini, void *, void *store, const void *)
{
    const char *str = ini->Get_Next_Token();
    captainslog_relassert(g_theScienceStore != nullptr, CODE_01, "TheScienceStore not inited yet");
    *static_cast<ScienceType *>(store) = INI::Scan_Science(str);
}
