/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Skirmish Battle Honors
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "skirmishbattlehonors.h"

#include "display.h"
#include "gadgetlistbox.h"
#include "gadgetstatictext.h"
#include "gametext.h"
#include "gametype.h"
#include "gamewindowmanager.h"
#include "image.h"
#include "maputil.h"
#include "mouse.h"
#include "namekeygenerator.h"
#include "registryget.h"

#ifndef GAME_DLL
int g_rowsToSkip;
#endif

SkirmishBattleHonors::SkirmishBattleHonors()
{
    Load("SkirmishStats.ini");
}

SkirmishBattleHonors::~SkirmishBattleHonors() {}

int SkirmishBattleHonors::Get_Endurance_Medal(Utf8String name, int level) const
{
    Utf8String str;
    str.Format("%s_%d", name.Str(), level);
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Wins() const
{
    Utf8String str = "Wins";
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Win_Streak() const
{
    Utf8String str = "WinStreak";
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Best_Win_Streak() const
{
    Utf8String str = "BestWinStreak";
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Losses() const
{
    Utf8String str = "Losses";
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_China_Campaign_Complete(int level) const
{
    Utf8String str;
    str.Format("CHINACampaign_%d", level);
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_USA_Campaign_Complete(int level) const
{
    Utf8String str;
    str.Format("USACampaign_%d", level);
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_GLA_Campaign_Complete(int level) const
{
    Utf8String str;
    str.Format("GLACampaign_%d", level);
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Challenge_Campaign_Complete(int general, int level) const
{
    Utf8String str;
    str.Format("ChallengeCampaign%d_%d", general, level);
    return Get_Int(str, 0);
}

int SkirmishBattleHonors::Get_Honors() const
{
    Utf8String str = "Honors";
    return Get_Int(str, 0);
}

void Reset_Battle_Honor_Insertion()
{
    g_rowsToSkip = 0;
}

enum SkirmishBattleHonorsTooltips : int32_t
{
    STREAK = 1 << 1,
    LOYALTY_USA = 1 << 5,
    LOYALTY_CHINA = 1 << 6,
    BATTLE_TANK = 1 << 7,
    AIR_WING = 1 << 8,
    LOYALTY_GLA = 1 << 9,
    ENDURANCE = 1 << 10,
    CAMPAIGN_USA = 1 << 11,
    CAMPAIGN_CHINA = 1 << 12,
    CAMPAIGN_GLA = 1 << 13,
    BLITZ_5 = 1 << 14,
    BLITZ_10 = 1 << 15,
    FAIR_PLAY = 1 << 16,
    APOCALYPSE = 1 << 17,
    OFFICERS_CLUB = 1 << 18,
    DOMINATION = 1 << 19,
    CAMPAIGN_CHALLENGE = 1 << 20,
    ULTIMATE = 1 << 21,
    GLOBAL_GENERAL = 1 << 22,
    DOMINATION_ONLINE = 1 << 23,
    STREAK_ONLINE = 1 << 24,
    CHALLENGE = 1 << 25,
    DISABLED = 1 << 27,
};

void Insert_Battle_Honor(
    GameWindow *listbox, Image const *image, bool earned, int tooltip, int *row, int *column, Utf16String str, int score)
{
    int height = (int)(((float)g_theDisplay->Get_Height() / 600.0f) * 41.0f);
    int width = (int)(((float)g_theDisplay->Get_Width() / 800.0f) * 40.0f);
    int color;
    if (earned) {
        color = 0xFFFFFFFF;
    } else {
        tooltip |= DISABLED;
        color = 0xFF505050;
    }
    Gadget_List_Box_Add_Entry_Image(listbox, image, *row, *column, height, width, 1, color);
    Gadget_List_Box_Set_Item_Data(listbox, reinterpret_cast<void *>(tooltip), *row, *column);
    Gadget_List_Box_Set_Item_Data(listbox, reinterpret_cast<void *>(score), *row - 1, *column);
    int next_column = *column + 1;
    *column = next_column;
    int num_columns = Gadget_List_Box_Get_Num_Columns(listbox);
    if (next_column >= num_columns) {
        *column = 0;
        *row += g_rowsToSkip + 1;
        num_columns = g_rowsToSkip - 1;
        if (num_columns >= 0) {
            g_rowsToSkip = num_columns;
        } else {
            g_rowsToSkip = 0;
        }
    }
}

void Populate_Skirmish_Battle_Honors()
{
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key("SkirmishGameOptionsMenu.wnd:ListboxInfo");
    GameWindow *listbox = g_theWindowManager->Win_Get_Window_From_Id(nullptr, key);
    if (listbox == nullptr) {
        return;
    }
    SkirmishBattleHonors *battle_honors = nullptr;
    battle_honors = new SkirmishBattleHonors();
    listbox->Win_Set_Tooltip_Func(Battle_Honor_Tooltip);
    Gadget_List_Box_Reset(listbox);

    int honors = battle_honors->Get_Honors();

    key = g_theNameKeyGenerator->Name_To_Key("SkirmishGameOptionsMenu.wnd:StaticTextStreakValue");
    GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, key);
    if (window != nullptr) {
        int win_steak = battle_honors->Get_Win_Streak();
        Utf16String str;
        str.Format(U_CHAR("%d"), win_steak);
        Gadget_Static_Text_Set_Text(window, str);
    }

    key = g_theNameKeyGenerator->Name_To_Key("SkirmishGameOptionsMenu.wnd:StaticTextBestStreakValue");
    window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, key);
    if (window != nullptr) {
        int best_win_streak = battle_honors->Get_Best_Win_Streak();
        Utf16String str;
        str.Format(U_CHAR("%d"), best_win_streak);
        Gadget_Static_Text_Set_Text(window, str);
    }

    key = g_theNameKeyGenerator->Name_To_Key("SkirmishGameOptionsMenu.wnd:StaticTextWinsValue");
    window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, key);
    if (window != nullptr) {
        int wins = battle_honors->Get_Wins();
        Utf16String str;
        str.Format(U_CHAR("%d"), wins);
        Gadget_Static_Text_Set_Text(window, str);
    }

    key = g_theNameKeyGenerator->Name_To_Key("SkirmishGameOptionsMenu.wnd:StaticTextLossesValue");
    window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, key);
    if (window != nullptr) {
        int losses = battle_honors->Get_Losses();
        Utf16String str;
        str.Format(U_CHAR("%d"), losses);
        Gadget_Static_Text_Set_Text(window, str);
    }

    Reset_Battle_Honor_Insertion();
    Gadget_List_Box_Add_Entry_Image(listbox, nullptr, 0, 0, 10, 10, 1, -1);

    int row = 1;
    int column = 0;

    const Image *image;
    if (battle_honors->Get_China_Campaign_Complete(DIFFICULTY_HARD)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("ChinaCampaign_G");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHINA, &row, &column, 0, 0);
    } else if (battle_honors->Get_China_Campaign_Complete(DIFFICULTY_NORMAL)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("ChinaCampaign_S");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHINA, &row, &column, 0, 0);
    } else if (battle_honors->Get_China_Campaign_Complete(DIFFICULTY_EASY)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("ChinaCampaign_B");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHINA, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("ChinaCampaign_B");
        Insert_Battle_Honor(listbox, image, (honors & CAMPAIGN_CHINA) != 0, CAMPAIGN_CHINA, &row, &column, 0, 0);
    }

    if (battle_honors->Get_GLA_Campaign_Complete(DIFFICULTY_HARD)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("GLACampaign_G");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_GLA, &row, &column, 0, 0);
    } else if (battle_honors->Get_GLA_Campaign_Complete(DIFFICULTY_NORMAL)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("GLACampaign_S");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_GLA, &row, &column, 0, 0);
    } else if (battle_honors->Get_GLA_Campaign_Complete(DIFFICULTY_EASY)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("GLACampaign_B");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_GLA, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("GLACampaign_B");
        Insert_Battle_Honor(listbox, image, (honors & CAMPAIGN_GLA) != 0, CAMPAIGN_GLA, &row, &column, 0, 0);
    }

    if (battle_honors->Get_USA_Campaign_Complete(DIFFICULTY_HARD)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("USACampaign_G");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_USA, &row, &column, 0, 0);
    } else if (battle_honors->Get_USA_Campaign_Complete(DIFFICULTY_NORMAL)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("USACampaign_S");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_USA, &row, &column, 0, 0);
    } else if (battle_honors->Get_USA_Campaign_Complete(DIFFICULTY_EASY)) {
        image = g_theMappedImageCollection->Find_Image_By_Name("USACampaign_B");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_USA, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("USACampaign_B");
        Insert_Battle_Honor(listbox, image, (honors & CAMPAIGN_USA) != 0, CAMPAIGN_USA, &row, &column, 0, 0);
    }

    bool bronze = false;
    bool silver = false;
    bool gold = false;
    for (int i = 0; i < 9; i++) {
        if (battle_honors->Get_Challenge_Campaign_Complete(i, DIFFICULTY_HARD)) {
            gold = true;
        }
        if (battle_honors->Get_Challenge_Campaign_Complete(i, DIFFICULTY_NORMAL)) {
            silver = true;
        }
        if (battle_honors->Get_Challenge_Campaign_Complete(i, DIFFICULTY_EASY)) {
            bronze = true;
        }
    }
    if (gold) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Challenge_Gold");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHALLENGE, &row, &column, 0, 0);
    } else if (silver) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Challenge_Silver");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHALLENGE, &row, &column, 0, 0);
    } else if (bronze) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Challenge_Bronz");
        Insert_Battle_Honor(listbox, image, true, CAMPAIGN_CHALLENGE, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("Challenge_Bronz");
        Insert_Battle_Honor(listbox, image, false, CAMPAIGN_CHALLENGE, &row, &column, 0, 0);
    }

    image = g_theMappedImageCollection->Find_Image_By_Name("HonorAirWing");
    Insert_Battle_Honor(listbox, image, (honors & AIR_WING) != 0, AIR_WING, &row, &column, 0, 0);

    image = g_theMappedImageCollection->Find_Image_By_Name("HonorBattleTank");
    Insert_Battle_Honor(listbox, image, (honors & BATTLE_TANK) != 0, BATTLE_TANK, &row, &column, 0, 0);

    Gadget_List_Box_Add_Entry_Image(listbox, nullptr, 2, 0, 10, 10, 1, -1);

    row = 3;
    column = 0;

    // Endurance: beat every map
    bronze = true;
    silver = true;
    gold = true;
    for (auto it = g_theMapCache->begin(); it != g_theMapCache->end(); it++) {
        if (it->second.m_isOfficial && it->second.m_isMultiplayer) {
            int beat_hard = battle_honors->Get_Endurance_Medal(it->first, 4);
            if (!beat_hard) {
                gold = false;
                int beat_medium = battle_honors->Get_Endurance_Medal(it->first, 3);
                if (!beat_medium) {
                    silver = false;
                    int beat_easy = battle_honors->Get_Endurance_Medal(it->first, 2);
                    if (!beat_easy) {
                        bronze = false;
                        break;
                    }
                }
            }
        }
    }
    if (gold) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Endurance_G");
        Insert_Battle_Honor(listbox, image, true, ENDURANCE, &row, &column, 0, 0);
    } else if (silver) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Endurance_S");
        Insert_Battle_Honor(listbox, image, true, ENDURANCE, &row, &column, 0, 0);
    } else if (bronze) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Endurance_B");
        Insert_Battle_Honor(listbox, image, true, ENDURANCE, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("Endurance_B");
        Insert_Battle_Honor(listbox, image, false, ENDURANCE, &row, &column, 0, 0);
    }

    // Apocalypse: build all superweapons
    image = g_theMappedImageCollection->Find_Image_By_Name("Apocalypse");
    Insert_Battle_Honor(listbox, image, (honors & APOCALYPSE) != 0, APOCALYPSE, &row, &column, 0, 0);

    // Blitz: win in (x) minutes or less
    if ((honors & BLITZ_5) != 0) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorBlitz5");
        Insert_Battle_Honor(listbox, image, true, BLITZ_5, &row, &column, 0, 0);
    } else if ((honors & BLITZ_10) != 0) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorBlitz10");
        Insert_Battle_Honor(listbox, image, true, BLITZ_10, &row, &column, 0, 0);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorBlitz10");
        Insert_Battle_Honor(listbox, image, false, BLITZ_10, &row, &column, 0, 0);
    }

    int best_win_streak = battle_honors->Get_Best_Win_Streak();
    Utf16String str;
    str.Format(U_CHAR("%10d"), best_win_streak);
    if (best_win_streak >= 1000) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_1000");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else if (best_win_streak >= 500) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_500");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else if (best_win_streak >= 100) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_100");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else if (best_win_streak >= 25) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_G");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else if (best_win_streak >= 10) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_S");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else if (best_win_streak >= 3) {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_B");
        Insert_Battle_Honor(listbox, image, true, STREAK, &row, &column, 0, best_win_streak);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("HonorStreak_B");
        Insert_Battle_Honor(listbox, image, false, STREAK, &row, &column, 0, best_win_streak);
    }

    int wins = battle_honors->Get_Wins();
    str.Format(U_CHAR("%10d"), wins);
    if (wins >= 10000) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Domination_10000");
        Insert_Battle_Honor(listbox, image, true, DOMINATION, &row, &column, 0, wins);
    } else if (wins >= 1000) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Domination_1000");
        Insert_Battle_Honor(listbox, image, true, DOMINATION, &row, &column, 0, wins);
    } else if (wins >= 500) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Domination_500");
        Insert_Battle_Honor(listbox, image, true, DOMINATION, &row, &column, 0, wins);
    } else if (wins >= 100) {
        image = g_theMappedImageCollection->Find_Image_By_Name("Domination_100");
        Insert_Battle_Honor(listbox, image, true, DOMINATION, &row, &column, 0, wins);
    } else {
        image = g_theMappedImageCollection->Find_Image_By_Name("Domination_100");
        Insert_Battle_Honor(listbox, image, false, DOMINATION, &row, &column, 0, wins);
    }

    // Ultimate: beat every map vs max number of hard opponents
    bool earned = true;
    for (auto it = g_theMapCache->begin(); it != g_theMapCache->end(); it++) {
        if (it->second.m_isOfficial && it->second.m_isMultiplayer) {
            int opponents_beat = battle_honors->Get_Endurance_Medal(it->first, 4);
            if (opponents_beat < it->second.m_numPlayers - 1) {
                earned = false;
                break;
            }
        }
    }
    image = g_theMappedImageCollection->Find_Image_By_Name("Ultimate");
    Insert_Battle_Honor(listbox, image, earned, ULTIMATE, &row, &column, 0, 0);

    unsigned int preorder = 0;
    Get_Unsigned_Int_From_Registry("", "Preorder", preorder);
    if (preorder != 0) {
        image = g_theMappedImageCollection->Find_Image_By_Name("OfficersClub");
        Insert_Battle_Honor(listbox, image, true, OFFICERS_CLUB, &row, &column, 0, 0);
    }
}

void Battle_Honor_Tooltip(GameWindow *listbox, WinInstanceData *instance, unsigned int mouse)
{
    unsigned int honor;
    int score;
    int row, column;
    Utf16String str;
    Gadget_List_Box_Get_Entry_Based_On_XY(listbox, mouse & 0xFFFF, mouse >> 0x10, row, column);
    if ((row == -1) || (column == -1)) {
        str = g_theGameText->Fetch("TOOLTIP:BattleHonors", nullptr);
        g_theMouse->Set_Cursor_Tooltip(str, -1, 0, 1.0f);
    } else {
        honor = (int)reinterpret_cast<uintptr_t>(Gadget_List_Box_Get_Item_Data(listbox, row, column));
        score = (int)reinterpret_cast<uintptr_t>(Gadget_List_Box_Get_Item_Data(listbox, row - 1, column));
        if (honor == 0) {
            str = g_theGameText->Fetch("TOOLTIP:BattleHonors", nullptr);
            g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.0f);
        } else if (honor != 0) {
            if (honor & DISABLED) {
                if (honor & LOYALTY_USA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyUSADisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & LOYALTY_CHINA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyChinaDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & LOYALTY_GLA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyGLADisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & BATTLE_TANK) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorBattleTankDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & AIR_WING) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorAirWingDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & ENDURANCE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorEnduranceDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & CAMPAIGN_USA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignUSADisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & CAMPAIGN_CHINA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignChinaDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & CAMPAIGN_GLA) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignGLADisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & BLITZ_10) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorBlitzDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & FAIR_PLAY) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorFairPlayDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & APOCALYPSE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorApocalypseDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & CAMPAIGN_CHALLENGE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignChallengeDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & ULTIMATE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorUltimateDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & GLOBAL_GENERAL) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorGlobalGeneralDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & CHALLENGE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorChallengeDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & STREAK) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreakDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & STREAK_ONLINE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreakOnlineDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & DOMINATION) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDominationDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (honor & DOMINATION_ONLINE) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDominationOnlineDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                }
            } else if (honor & LOYALTY_USA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyUSA", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & LOYALTY_CHINA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyChina", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & LOYALTY_GLA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorLoyaltyGLA", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & BATTLE_TANK) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorBattleTank", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & AIR_WING) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorAirWing", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & ENDURANCE) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorEndurance", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & CAMPAIGN_USA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignUSA", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & CAMPAIGN_CHINA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignChina", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & CAMPAIGN_GLA) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignGLA", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & BLITZ_5) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorBlitz5", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & BLITZ_10) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorBlitz10", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & FAIR_PLAY) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorFairPlay", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & APOCALYPSE) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorApocalypse", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & OFFICERS_CLUB) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorOfficersClub", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & CAMPAIGN_CHALLENGE) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorCampaignChallenge", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & ULTIMATE) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorUltimate", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & GLOBAL_GENERAL) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorGlobalGeneral", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & CHALLENGE) {
                str = g_theGameText->Fetch("TOOLTIP:BattleHonorChallenge", nullptr);
                g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
            } else if (honor & STREAK) {
                if (score >= 1000) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak1000", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 500) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak500", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 100) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak100", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 25) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak25", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 10) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak10", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 3) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak3", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreakDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                }
            } else if (honor & STREAK_ONLINE) {
                if (score >= 1000) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak1000Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 500) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak500Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 100) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak100Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 25) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak25Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 10) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak10Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 3) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreak3Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorStreakOnlineDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                }
            } else if (honor & DOMINATION) {
                if (score >= 1000) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination10000", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 100) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination1000", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 500) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination500", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 100) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination100", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDominationDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                }
            } else if (honor & DOMINATION_ONLINE) {
                if (score >= 10000) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination10000Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 1000) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination1000Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 500) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination500Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else if (score >= 100) {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDomination100Online", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                } else {
                    str = g_theGameText->Fetch("TOOLTIP:BattleHonorDominationOnlineDisabled", nullptr);
                    g_theMouse->Set_Cursor_Tooltip(str, -1, nullptr, 1.5f);
                }
            }
        }
    }
}
