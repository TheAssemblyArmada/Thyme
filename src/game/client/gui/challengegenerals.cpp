/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "challengegenerals.h"
#include "image.h"
#include "ini.h"

#ifndef GAME_DLL
ChallengeGenerals *g_theChallengeGenerals = nullptr;
#endif

GeneralPersona::GeneralPersona() :
    m_startsEnabled(false),
    m_bioPortraitSmall(nullptr),
    m_bioPortraitLarge(nullptr),
    m_defeatedImage(nullptr),
    m_victoriousImage(nullptr)
{
}

GeneralPersona::~GeneralPersona() {}

void ChallengeGenerals::Init()
{
    INI ini;
    ini.Load("Data/INI/ChallengeMode.ini", INI_LOAD_OVERWRITE, nullptr);
}

void ChallengeGenerals::Parse_General_Persona(INI *ini, void *formal, void *store, void const *user_data)
{
    static FieldParse _parse_table[] = {
        { "StartsEnabled", &INI::Parse_Bool, nullptr, offsetof(GeneralPersona, m_startsEnabled) },
        { "BioNameString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioNameString) },
        { "BioDOBString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioDOBString) },
        { "BioBirthplaceString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioBirthplaceString) },
        { "BioStrategyString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioStrategyString) },
        { "BioRankString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioRankString) },
        { "BioBranchString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioBranchString) },
        { "BioClassNumberString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_bioClassNumberString) },
        { "BioPortraitSmall", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(GeneralPersona, m_bioPortraitSmall) },
        { "BioPortraitLarge", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(GeneralPersona, m_bioPortraitLarge) },
        { "Campaign", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_campaign) },
        { "PlayerTemplate", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_playerTemplate) },
        { "PortraitMovieLeftName", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_portraitMovieLeftName) },
        { "PortraitMovieRightName", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_portraitMovieRightName) },
        { "DefeatedImage", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(GeneralPersona, m_defeatedImage) },
        { "VictoriousImage", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(GeneralPersona, m_victoriousImage) },
        { "DefeatedString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_defeatedString) },
        { "VictoriousString", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_victoriousString) },
        { "SelectionSound", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_selectionSound) },
        { "TauntSound1", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_tauntSound1) },
        { "TauntSound2", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_tauntSound2) },
        { "TauntSound3", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_tauntSound3) },
        { "WinSound", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_winSound) },
        { "LossSound", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_lossSound) },
        { "PreviewSound", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_previewSound) },
        { "NameSound", &INI::Parse_AsciiString, nullptr, offsetof(GeneralPersona, m_nameSound) },
        { nullptr, nullptr, nullptr, 0 }
    };

    ini->Init_From_INI(store, _parse_table);
}

void ChallengeGenerals::Parse_Challenge_Mode_Definition(INI *ini)
{
    static FieldParse _parse_table[] = {
        { "GeneralPersona0", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[0]) },
        { "GeneralPersona1", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[1]) },
        { "GeneralPersona2", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[2]) },
        { "GeneralPersona3", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[3]) },
        { "GeneralPersona4", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[4]) },
        { "GeneralPersona5", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[5]) },
        { "GeneralPersona6", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[6]) },
        { "GeneralPersona7", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[7]) },
        { "GeneralPersona8", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[8]) },
        { "GeneralPersona9", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[9]) },
        { "GeneralPersona10", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[10]) },
        { "GeneralPersona11", &Parse_General_Persona, nullptr, offsetof(ChallengeGenerals, m_personas[11]) },
        { nullptr, nullptr, nullptr, 0 }
    };

    if (g_theChallengeGenerals) {
        ini->Init_From_INI(g_theChallengeGenerals, _parse_table);
    }
}

GeneralPersona *ChallengeGenerals::Get_Player_General_By_Campaign_Name(Utf8String name)
{
    for (int i = 0; i < Number_Of_Personas(); ++i) {
        Utf8String campaign_name = m_personas[i].Get_Campaign_Name();

        if (campaign_name.Compare_No_Case(name) == 0) {
            return &m_personas[i];
        }
    }

    captainslog_debug("Can't find General by Campaign Name");

    return nullptr;
}

GeneralPersona *ChallengeGenerals::Get_Player_General_By_General_Name(Utf8String name)
{
    for (int i = 0; i < Number_Of_Personas(); ++i) {
        Utf8String campaign_name = m_personas[i].Get_General_Name();

        if (campaign_name.Compare_No_Case(name) == 0) {
            return &m_personas[i];
        }
    }

    captainslog_debug("Can't find General by General Name");

    return nullptr;
}

GeneralPersona *ChallengeGenerals::Get_Player_General_By_Template_Name(Utf8String name)
{
    for (int i = 0; i < Number_Of_Personas(); ++i) {
        Utf8String campaign_name = m_personas[i].Get_Player_Template_Name();

        if (campaign_name.Compare_No_Case(name) == 0) {
            return &m_personas[i];
        }
    }

    captainslog_debug("Can't find General by Template Name");

    return nullptr;
}

int ChallengeGenerals::Number_Of_Personas() const
{
    return NUM_GENERALS;
}

ChallengeGenerals *Create_Challenge_Generals()
{
    return new ChallengeGenerals;
}
