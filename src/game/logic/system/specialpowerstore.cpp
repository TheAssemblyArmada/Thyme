/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Store
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpowerstore.h"

#include "object.h"
#include "player.h"
#include "specialpowertemplate.h"

#ifdef GAME_DLL
// See hooked globals
#else
SpecialPowerStore *TheSpecialPowerStore = nullptr;
#endif

// wb: 00725F73
SpecialPowerStore::SpecialPowerStore() : SubsystemInterface(), m_specialPowerTemplates(), m_nextSpecialPowerID(0) {}

// wb: 00725FFB
SpecialPowerStore::~SpecialPowerStore()
{
    for (SpecialPowerTemplate *sptemplate : m_specialPowerTemplates) {
        sptemplate->Delete_Instance();
    }
    m_specialPowerTemplates.clear();
    m_nextSpecialPowerID = 0;
}

// wb: 00726790
void SpecialPowerStore::Init() {}

// wb: 007262AE
void SpecialPowerStore::Reset()
{
    SpecialPowerTemplates::iterator it = m_specialPowerTemplates.begin();

    while (it != m_specialPowerTemplates.end()) {
        if ((*it)->Delete_Overrides() != nullptr) {
            ++it;
        } else {
            it = m_specialPowerTemplates.erase(it);
        }
    }
}

// wb: 007267A0
void SpecialPowerStore::Update() {}

// wb: 00725A00
void SpecialPowerStore::Parse_Special_Power_Definition(INI *ini)
{
    const char *token = ini->Get_Next_Token();
    Utf8String name(token);
    SpecialPowerTemplate *find_template = TheSpecialPowerStore->Find_SpecialPowerTemplate_Private(name);
    SpecialPowerTemplate *new_template = nullptr;

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {

        if (find_template != nullptr) {
            SpecialPowerTemplate *override_template = find_template->Friend_Get_Final_Override();
            new_template = new SpecialPowerTemplate;
            *new_template = *override_template;
            override_template->Set_Next(new_template);
            new_template->Set_Is_Allocated();
        } else {
            new_template = new SpecialPowerTemplate;
            const SpecialPowerTemplate *default_template =
                TheSpecialPowerStore->Find_SpecialPowerTemplate_By_Name("DefaultSpecialPower");

            if (default_template != nullptr) {
                *new_template = *default_template;
            }

            new_template->Init(name, ++TheSpecialPowerStore->m_nextSpecialPowerID);
            new_template->Set_Is_Allocated();
            TheSpecialPowerStore->m_specialPowerTemplates.push_back(new_template);
        }
    } else {
        captainslog_relassert(find_template == nullptr, CODE_06, "");

        new_template = new SpecialPowerTemplate;
        const SpecialPowerTemplate *default_template =
            TheSpecialPowerStore->Find_SpecialPowerTemplate_By_Name("DefaultSpecialPower");

        if (default_template != nullptr) {
            *new_template = *default_template;
        }

        new_template->Init(name, ++TheSpecialPowerStore->m_nextSpecialPowerID);
        // #TODO No new_template->Set_Is_Allocated ???
        TheSpecialPowerStore->m_specialPowerTemplates.push_back(new_template);
    }

    if (new_template != nullptr) {
        ini->Init_From_INI(new_template, SpecialPowerTemplate::Get_Parse_Table());
    }
}

// wb: 0072622E
unsigned int SpecialPowerStore::Get_Num_Special_Powers() const
{
    return m_specialPowerTemplates.size();
}

// wb: 00726570
const SpecialPowerTemplate *SpecialPowerStore::Find_SpecialPowerTemplate_By_Name(Utf8String name) const
{
    return const_cast<SpecialPowerStore *>(this)->Find_SpecialPowerTemplate_Private(name);
}

// wb: 00726193
const SpecialPowerTemplate *SpecialPowerStore::Find_SpecialPowerTemplate_By_ID(unsigned int id) const
{
    for (const SpecialPowerTemplate *sptemplate : m_specialPowerTemplates) {
        if (sptemplate->Get_ID() == id) {
            return sptemplate;
        }
    }
    return nullptr;
}

// wb: 007261F6
const SpecialPowerTemplate *SpecialPowerStore::Get_SpecialPowerTemplate_By_Index(unsigned int index) const
{
    if (index >= m_specialPowerTemplates.size()) {
        return nullptr;
    }

    return m_specialPowerTemplates[index];
}

// wb: 00726244
bool SpecialPowerStore::Can_Use_Special_Power(Object *object, const SpecialPowerTemplate *sptemplate)
{
    if (object == nullptr || sptemplate == nullptr) {
        return false;
    }

    if (!object->Get_Special_Power_Module(sptemplate)) {
        return false;
    }

    bool can_use = true;

    ScienceType required_science = sptemplate->Get_Required_Science();

    if (required_science != SCIENCE_INVALID) {
        Player *player = object->Get_Controlling_Player();

        if (!player->Has_Science(required_science)) {
            can_use = false;
        }
    }

    return can_use;
}

// wb: 007260A4
SpecialPowerTemplate *SpecialPowerStore::Find_SpecialPowerTemplate_Private(Utf8String name)
{
    for (SpecialPowerTemplate *sptemplate : m_specialPowerTemplates) {
        if (sptemplate->Get_Name() == name) {
            return sptemplate;
        }
    }
    return nullptr;
}
