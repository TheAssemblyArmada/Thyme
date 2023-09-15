/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Power
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpower.h"
#include "object.h"
#include "player.h"

#ifndef GAME_DLL
SpecialPowerStore *g_theSpecialPowerStore;
#endif
SpecialPowerStore::~SpecialPowerStore()
{
    for (unsigned int i = 0; i < m_specialPowerTemplates.size(); i++) {
        m_specialPowerTemplates[i]->Delete_Instance();
    }

    m_specialPowerTemplates.clear();
    m_nextSpecialPowerID = 0;
}

void SpecialPowerStore::Reset()
{
    for (auto it = m_specialPowerTemplates.begin(); it != m_specialPowerTemplates.end();) {
        if ((*it)->Delete_Overrides() == nullptr) {
            // #BUGFIX invalidated iterator dereference.
            it = m_specialPowerTemplates.erase(it);
        } else {
            ++it;
        }
    }
}

bool SpecialPowerStore::Can_Use_Special_Power(Object *obj, const SpecialPowerTemplate *special_power_template)
{
    if (obj == nullptr || special_power_template == nullptr) {
        return false;
    }

    if (obj->Get_Special_Power_Module(special_power_template) == nullptr) {
        return false;
    }

    ScienceType science = special_power_template->Get_Required_Science();

    if (science != SCIENCE_INVALID) {
        if (!obj->Get_Controlling_Player()->Has_Science(science)) {
            return false;
        }
    }

    return true;
}

SpecialPowerTemplate *SpecialPowerStore::Find_Special_Power_Template_Private(Utf8String name)
{
    for (unsigned int i = 0; i < m_specialPowerTemplates.size(); i++) {
        if (m_specialPowerTemplates[i]->Get_Name() == name) {
            return m_specialPowerTemplates[i];
        }
    }

    return nullptr;
}

const SpecialPowerTemplate *SpecialPowerStore::Find_Special_Power_Template(Utf8String name)
{
    return Find_Special_Power_Template_Private(name);
}

const SpecialPowerTemplate *SpecialPowerStore::Get_Special_Power_Template_By_Index(unsigned int index)
{
    if (index < m_specialPowerTemplates.size()) {
        return m_specialPowerTemplates[index];
    } else {
        return nullptr;
    }
}

int SpecialPowerStore::Get_Num_Special_Powers()
{
    return m_specialPowerTemplates.size();
}

void SpecialPowerStore::Parse_Special_Power_Definition(INI *ini)
{
    Utf8String str = ini->Get_Next_Token();
    SpecialPowerTemplate *power = g_theSpecialPowerStore->Find_Special_Power_Template_Private(str);

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        if (power != nullptr) {
            SpecialPowerTemplate *final_override = static_cast<SpecialPowerTemplate *>(power->Friend_Get_Final_Override());
            power = new SpecialPowerTemplate();
            *power = *final_override;
            final_override->Set_Next(power);
            power->Set_Is_Allocated();
        } else {
            power = new SpecialPowerTemplate();
            const SpecialPowerTemplate *default_power =
                g_theSpecialPowerStore->Find_Special_Power_Template("DefaultSpecialPower");

            if (default_power != nullptr) {
                *power = *default_power;
            }

            power->Set_Name_ID(str, ++g_theSpecialPowerStore->m_nextSpecialPowerID);
            power->Set_Is_Allocated();
            g_theSpecialPowerStore->m_specialPowerTemplates.push_back(power);
        }
    } else {
        if (power != nullptr) {
            throw CODE_06;
        }

        power = new SpecialPowerTemplate();
        const SpecialPowerTemplate *default_power =
            g_theSpecialPowerStore->Find_Special_Power_Template("DefaultSpecialPower");

        if (default_power != nullptr) {
            *power = *default_power;
        }

        power->Set_Name_ID(str, ++g_theSpecialPowerStore->m_nextSpecialPowerID);
        g_theSpecialPowerStore->m_specialPowerTemplates.push_back(power);
    }

    if (power != nullptr) {
        ini->Init_From_INI(power, SpecialPowerTemplate::Get_Field_Parse());
    }
}

SpecialPowerTemplate::SpecialPowerTemplate() :
    m_id(0),
    m_type(SPECIAL_INVALID),
    m_reloadTime(0),
    m_requiredScience(SCIENCE_INVALID),
    m_academyClassify(ACT_NONE),
    m_detectionTime(300),
    m_viewObjectDuration(0),
    m_viewObjectRange(0.0f),
    m_radiusCursorRadius(0.0f),
    m_publicTimer(false),
    m_sharedSyncedTimer(false),
    m_shortcutPower(false)
{
}

const SpecialPowerTemplate *SpecialPowerTemplate::Get_FO() const
{
    return static_cast<const SpecialPowerTemplate *>(Friend_Get_Final_Override());
}

const SpecialPowerTemplate *SpecialPowerStore::Find_Special_Power_Template_By_ID(unsigned int id)
{
    for (size_t i = 0; i < m_specialPowerTemplates.size(); i++) {
        if (m_specialPowerTemplates[i]->Get_ID() == id) {
            return m_specialPowerTemplates[i];
        }
    }

    return nullptr;
}

static const char *s_academyClassificationTypeNames[] = { "ACT_NONE", "ACT_UPGRADE_RADAR", "ACT_SUPERPOWER", nullptr };

// clang-format off
const FieldParse SpecialPowerTemplate::s_specialPowerFieldParse[] = {
    { "ReloadTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_reloadTime) },
    { "RequiredScience", &ScienceStore::Parse_Science, nullptr, offsetof(SpecialPowerTemplate, m_requiredScience) },
    { "InitiateSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(SpecialPowerTemplate, m_initiateSound) },
    { "InitiateAtLocationSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(SpecialPowerTemplate, m_initiateAtLocationSound) },
    { "PublicTimer", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_publicTimer) },
    { "Enum", &INI::Parse_Index_List, BitFlags<SPECIAL_POWER_COUNT>::Get_Bit_Names_List(), offsetof(SpecialPowerTemplate, m_type) },
    { "DetectionTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_detectionTime) },
    { "SharedSyncedTimer", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_sharedSyncedTimer) },
    { "ViewObjectDuration", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_viewObjectDuration) },
    { "ViewObjectRange", &INI::Parse_Real, nullptr, offsetof(SpecialPowerTemplate, m_viewObjectRange) },
    { "RadiusCursorRadius", &INI::Parse_Real, nullptr, offsetof(SpecialPowerTemplate, m_radiusCursorRadius) },
    { "ShortcutPower", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_shortcutPower) },
    { "AcademyClassify", &INI::Parse_Index_List, s_academyClassificationTypeNames, offsetof(SpecialPowerTemplate, m_academyClassify) },
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on
