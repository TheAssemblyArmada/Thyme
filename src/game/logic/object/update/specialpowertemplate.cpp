/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Template
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpowertemplate.h"
#include "object.h"
#include "science.h"

const char* const TheAcademyClassTypeNames[ACT_COUNT + 1] = {
    "ACT_NONE",
    "ACT_UPGRADE_RADAR",
    "ACT_SUPERPOWER",
    nullptr,
};

// clang-format off
// wb: 00D09390
const FieldParse SpecialPowerTemplate::s_parseTable[] = {
    { "ReloadTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_reloadTime) },
    { "RequiredScience", &INI::Parse_Science, nullptr, offsetof(SpecialPowerTemplate, m_requiredScience) },
    { "InitiateSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(SpecialPowerTemplate, m_initiateSound) },
    { "InitiateAtLocationSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(SpecialPowerTemplate, m_initiateAtLocationSound) },
    { "PublicTimer", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_hasPublicTimer) },
    { "Enum", &INI::Parse_Index_List, BitFlags<SPECIAL_POWER_COUNT>::Get_Bit_Names_List(), offsetof(SpecialPowerTemplate, m_type) },
    { "DetectionTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_detectionTime) },
    { "SharedSyncedTimer", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_hasSharedSyncedTimer) },
    { "ViewObjectDuration", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(SpecialPowerTemplate, m_viewObjectDuration) },
    { "ViewObjectRange", &INI::Parse_Real, nullptr, offsetof(SpecialPowerTemplate, m_viewObjectRange) },
    { "RadiusCursorRadius", &INI::Parse_Real, nullptr, offsetof(SpecialPowerTemplate, m_cursorRadius) },
    { "ShortcutPower", &INI::Parse_Bool, nullptr, offsetof(SpecialPowerTemplate, m_hasShortcutPower) },
    { "AcademyClassify", &INI::Parse_Index_List, TheAcademyClassTypeNames, offsetof(SpecialPowerTemplate, m_academyClassType) },
    { nullptr, nullptr, nullptr, 0 }
};
// clang-format on

// wb: 00725E0C
SpecialPowerTemplate::SpecialPowerTemplate()
    : m_name()
    , m_initiateSound()
    , m_initiateAtLocationSound()
{
    m_id = 0u;
    m_type = SPECIAL_INVALID;
    m_reloadTime = 0u;
    m_requiredScience = SCIENCE_INVALID;
    m_hasPublicTimer = false;
    m_detectionTime = 300u;
    m_hasSharedSyncedTimer = false;
    m_viewObjectDuration = 0u;
    m_viewObjectRange = 0.0f;
    m_cursorRadius = 0.0f;
    m_hasShortcutPower = false;

    // #BUGFIX Initialize all members.
    m_academyClassType = ACT_NONE;
};

SpecialPowerTemplate::~SpecialPowerTemplate() {}

// wb: 00726540
void SpecialPowerTemplate::Init(const Utf8String &name, unsigned int id)
{
    m_name = name;
    m_id = id;
}

// wb: 00425160
const Utf8String& SpecialPowerTemplate::Get_Name() const
{
    return Friend_Get_Final_Override()->m_name;
}

// wb: 007267E0
unsigned int SpecialPowerTemplate::Get_ID() const
{
    return Friend_Get_Final_Override()->m_id;
}

// wb: 00726800
ScienceType SpecialPowerTemplate::Get_Required_Science() const
{
    return Friend_Get_Final_Override()->m_requiredScience;
}

// wb: 004251A0
const SpecialPowerTemplate *SpecialPowerTemplate::Friend_Get_Final_Override() const
{
    return static_cast<const SpecialPowerTemplate *>(Overridable::Friend_Get_Final_Override());
}

SpecialPowerTemplate *SpecialPowerTemplate::Friend_Get_Final_Override()
{
    return static_cast<SpecialPowerTemplate *>(Overridable::Friend_Get_Final_Override());
}

// wb: 00726530
const FieldParse *SpecialPowerTemplate::Get_Parse_Table()
{
    return s_parseTable;
}
