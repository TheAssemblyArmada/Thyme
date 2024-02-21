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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "gamelogic.h"

class Image;
class INI;

class GeneralPersona
{
public:
    GeneralPersona();
    ~GeneralPersona();

    Utf8String Get_Campaign_Name() const { return m_campaign; }
    Utf8String Get_General_Name() const { return m_bioNameString; }
    Utf8String Get_Player_Template_Name() const { return m_playerTemplate; }
    Utf8String Get_Preview_Sound() const { return m_previewSound; }
    Image *Get_Bio_Portrait_Small() const { return m_bioPortraitSmall; }
    Utf8String Get_Bio_Rank_String() const { return m_bioRankString; }
    Utf8String Get_Bio_Branch_String() const { return m_bioBranchString; }
    Utf8String Get_Bio_Strategy_String() const { return m_bioStrategyString; }

    bool Get_Starts_Enabled() const { return m_startsEnabled; }
    void Set_Starts_Enabled(bool state) { m_startsEnabled = state; }

public:
    bool m_startsEnabled;

    Utf8String m_bioNameString;
    Utf8String m_bioDOBString;
    Utf8String m_bioBirthplaceString;
    Utf8String m_bioStrategyString;
    Utf8String m_bioRankString;
    Utf8String m_bioBranchString;
    Utf8String m_bioClassNumberString;
    Image *m_bioPortraitSmall;
    Image *m_bioPortraitLarge;

    Utf8String m_campaign;
    Utf8String m_playerTemplate;

    Utf8String m_portraitMovieLeftName;
    Utf8String m_portraitMovieRightName;

    Image *m_defeatedImage;
    Image *m_victoriousImage;

    Utf8String m_defeatedString;
    Utf8String m_victoriousString;

    Utf8String m_selectionSound;
    Utf8String m_tauntSound1;
    Utf8String m_tauntSound2;
    Utf8String m_tauntSound3;
    Utf8String m_winSound;
    Utf8String m_lossSound;
    Utf8String m_previewSound;
    Utf8String m_nameSound;
};

class ChallengeGenerals
{
public:
    enum
    {
        NUM_GENERALS = 12
    };

public:
    ChallengeGenerals() : m_personas(), m_playerFaction(0), m_difficulty(DIFFICULTY_EASY) {}

    void Init();

    static void Parse_General_Persona(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Challenge_Mode_Definition(INI *ini);

    GeneralPersona *Get_Player_General_By_Campaign_Name(Utf8String name);
    GeneralPersona *Get_Player_General_By_General_Name(Utf8String name);
    GeneralPersona *Get_Player_General_By_Template_Name(Utf8String name);

    GeneralPersona *Get_Personas() { return m_personas; }

    int Get_Player_Faction() const { return m_playerFaction; }
    void Set_Player_Faction(int faction) { m_playerFaction = faction; }

    GameDifficulty Get_Difficulty() const { return m_difficulty; }
    void Set_Difficulty(GameDifficulty diff) { m_difficulty = diff; }

    int Number_Of_Personas() const;

private:
    GeneralPersona m_personas[NUM_GENERALS];
    int m_playerFaction;
    GameDifficulty m_difficulty;
};

ChallengeGenerals *Create_Challenge_Generals();

#ifdef GAME_DLL
extern ChallengeGenerals *&g_theChallengeGenerals;
#else
extern ChallengeGenerals *g_theChallengeGenerals;
#endif
