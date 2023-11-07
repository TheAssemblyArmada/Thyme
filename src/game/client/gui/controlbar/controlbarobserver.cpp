/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorspace.h"
#include "controlbar.h"
#include "gadgetpushbutton.h"
#include "gadgetstatictext.h"
#include "gametext.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "recorder.h"

enum
{
    OBSERVER_BUTTON_COUNT = 8
};

static NameKeyType s_buttonPlayerID[OBSERVER_BUTTON_COUNT];
static NameKeyType s_staticTextPlayerID[OBSERVER_BUTTON_COUNT];
static GameWindow *s_observerPlayerInfoWindow;
static GameWindow *s_observerPlayerListWindow;
static GameWindow *s_buttonPlayer[OBSERVER_BUTTON_COUNT];
static GameWindow *s_staticTextPlayer[OBSERVER_BUTTON_COUNT];
static NameKeyType s_buttonCancelID;
static GameWindow *s_winFlag;
static GameWindow *s_winGeneralPortrait;
static GameWindow *s_staticTextNumberOfUnits;
static GameWindow *s_staticTextNumberOfBuildings;
static GameWindow *s_staticTextNumberOfUnitsKilled;
static GameWindow *s_staticTextNumberOfUnitsLost;
static GameWindow *s_staticTextPlayerName;

void ControlBar::Init_Observer_Controls()
{
    s_observerPlayerInfoWindow = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ObserverPlayerInfoWindow"));
    s_observerPlayerListWindow = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ObserverPlayerListWindow"));

    for (int i = 0; i < OBSERVER_BUTTON_COUNT; i++) {
        Utf8String player_id;
        player_id.Format("ControlBar.wnd:ButtonPlayer%d", i);
        s_buttonPlayerID[i] = g_theNameKeyGenerator->Name_To_Key(player_id.Str());
        s_buttonPlayer[i] = g_theWindowManager->Win_Get_Window_From_Id(s_observerPlayerListWindow, s_buttonPlayerID[i]);

        player_id.Format("ControlBar.wnd:StaticTextPlayer%d", i);
        s_staticTextPlayerID[i] = g_theNameKeyGenerator->Name_To_Key(player_id.Str());
        s_staticTextPlayer[i] =
            g_theWindowManager->Win_Get_Window_From_Id(s_observerPlayerListWindow, s_staticTextPlayerID[i]);
    }

    s_staticTextNumberOfUnits = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextNumberOfUnits"));
    s_staticTextNumberOfBuildings = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextNumberOfBuildings"));
    s_staticTextNumberOfUnitsKilled = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextNumberOfUnitsKilled"));
    s_staticTextNumberOfUnitsLost = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextNumberOfUnitsLost"));
    s_staticTextPlayerName = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextPlayerName"));
    s_winFlag =
        g_theWindowManager->Win_Get_Window_From_Id(nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:WinFlag"));
    s_winGeneralPortrait = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:WinGeneralPortrait"));
    s_buttonCancelID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonCancel");
}

WindowMsgHandledType Control_Bar_Observer_System(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message > GBM_MOUSE_LEAVING) {
        if (message > GBM_SELECTED_RIGHT) {
            return MSG_IGNORED;
        }

        int id = reinterpret_cast<GameWindow *>(data_1)->Win_Get_Window_Id();

        if (id == s_buttonCancelID) {
            g_theControlBar->Set_Observer_Player(nullptr);
            s_observerPlayerInfoWindow->Win_Hide(true);
            s_observerPlayerListWindow->Win_Hide(false);
            g_theControlBar->Populate_Observer_List();
        }

        for (int i = 0; i < OBSERVER_BUTTON_COUNT; i++) {
            if (id == s_buttonPlayerID[i]) {
                s_observerPlayerInfoWindow->Win_Hide(false);
                s_observerPlayerListWindow->Win_Hide(true);
                g_theControlBar->Set_Observer_Player(reinterpret_cast<Player *>(Gadget_Button_Get_Data(s_buttonPlayer[i])));

                if (g_theControlBar->Get_Observer_Player() != nullptr) {
                    g_theControlBar->Populate_Observer_Info_Window();
                }

                return MSG_HANDLED;
            }
        }
    } else if (message < GBM_MOUSE_ENTERING && message != GWM_CREATE) {
        return MSG_IGNORED;
    }

    return MSG_HANDLED;
}

void ControlBar::Populate_Observer_List()
{
    int button_count = 0;

    if (g_theRecorder->Is_Multiplayer()) {
        for (int i = 0; i < OBSERVER_BUTTON_COUNT; i++) {
            Utf8String player_id;
            player_id.Format("player%d", i);
            Player *player = g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(player_id.Str()));

            if (player != nullptr && !player->Is_Player_Observer()) {
                captainslog_dbgassert(button_count < OBSERVER_BUTTON_COUNT,
                    "ControlBar::Populate_Observer_List trying to populate more buttons then we have");
                Gadget_Button_Set_Data(s_buttonPlayer[button_count], reinterpret_cast<void *>(player));
                Gadget_Button_Set_Enabled_Image(
                    s_buttonPlayer[button_count], player->Get_Player_Template()->Get_Enabled_Image());
                s_buttonPlayer[button_count]->Win_Set_Tooltip(player->Get_Player_Display_Name());
                s_buttonPlayer[button_count]->Win_Hide(false);
                s_buttonPlayer[button_count]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
                const GameSlot *slot = g_theGameInfo->Get_Const_Slot(button_count);
                s_staticTextPlayer[button_count]->Win_Set_Enabled_Text_Colors(player->Get_Color(), Make_Color(0, 0, 0, 255));
                s_staticTextPlayer[button_count]->Win_Hide(false);
                Utf8String team_id;
                team_id.Format("Team:%d", slot->Get_Team_Number() + 1);

                if (slot->Is_AI() && slot->Get_Team_Number() == -1) {
                    team_id = "Team:AI";
                }

                Utf16String player_label;
                player_label.Format(g_theGameText->Fetch("CONTROLBAR:ObsPlayerLabel"),
                    player->Get_Player_Display_Name().Str(),
                    g_theGameText->Fetch(team_id).Str());
                Gadget_Static_Text_Set_Text(s_staticTextPlayer[button_count], player_label);
                button_count++;
            }
        }

        while (button_count < OBSERVER_BUTTON_COUNT) {
            s_buttonPlayer[button_count]->Win_Hide(true);
            s_staticTextPlayer[button_count]->Win_Hide(true);
            button_count++;
        }
    } else {
        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            Player *player = g_thePlayerList->Get_Nth_Player(i);

            if (player != nullptr && !player->Is_Player_Observer() && player->Get_Player_Type() == Player::PLAYER_HUMAN) {
                Gadget_Button_Set_Data(s_buttonPlayer[button_count], reinterpret_cast<void *>(player));
                Gadget_Button_Set_Enabled_Image(
                    s_buttonPlayer[button_count], player->Get_Player_Template()->Get_Enabled_Image());
                s_buttonPlayer[button_count]->Win_Set_Tooltip(player->Get_Player_Display_Name());
                s_buttonPlayer[button_count]->Win_Hide(false);
                s_buttonPlayer[button_count]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
                s_staticTextPlayer[button_count]->Win_Set_Enabled_Text_Colors(player->Get_Color(), Make_Color(0, 0, 0, 255));
                s_staticTextPlayer[button_count]->Win_Hide(false);
                Gadget_Static_Text_Set_Text(s_staticTextPlayer[button_count], player->Get_Player_Display_Name());
                button_count++;
                break;
            }
        }

        while (button_count < OBSERVER_BUTTON_COUNT) {
            s_buttonPlayer[button_count]->Win_Hide(true);
            s_staticTextPlayer[button_count]->Win_Hide(true);
            button_count++;
        }
    }
}

void ControlBar::Populate_Observer_Info_Window()
{
    if (!s_observerPlayerInfoWindow->Win_Is_Hidden()) {
        if (m_observerPlayer != nullptr) {
            Utf16String str;
            BitFlags<KINDOF_COUNT> must_be_set;
            BitFlags<KINDOF_COUNT> must_be_clear;
            must_be_set.Set(KINDOF_SCORE, true);
            must_be_clear.Set(KINDOF_STRUCTURE, true);
            str.Format(U_CHAR("%d"), m_observerPlayer->Count_Objects(must_be_set, must_be_clear));
            Gadget_Static_Text_Set_Text(s_staticTextNumberOfUnits, str);

            int count = 0;
            must_be_set.Clear();
            must_be_set.Set(KINDOF_SCORE, true);
            must_be_set.Set(KINDOF_STRUCTURE, true);
            must_be_clear.Clear();
            count = m_observerPlayer->Count_Objects(must_be_set, must_be_clear);

            must_be_set.Clear();
            must_be_set.Set(KINDOF_SCORE_CREATE, true);
            must_be_set.Set(KINDOF_STRUCTURE, true);
            count += m_observerPlayer->Count_Objects(must_be_set, must_be_clear);

            must_be_set.Clear();
            must_be_set.Set(KINDOF_SCORE_DESTROY, true);
            must_be_set.Set(KINDOF_STRUCTURE, true);
            count += m_observerPlayer->Count_Objects(must_be_set, must_be_clear);

            str.Format(U_CHAR("%d"), count);
            Gadget_Static_Text_Set_Text(s_staticTextNumberOfBuildings, str);
            str.Format(U_CHAR("%d"), m_observerPlayer->Get_Score_Keeper()->Get_Total_Units_Destroyed());
            Gadget_Static_Text_Set_Text(s_staticTextNumberOfUnitsKilled, str);
            str.Format(U_CHAR("%d"), m_observerPlayer->Get_Score_Keeper()->Get_Total_Units_Lost());
            Gadget_Static_Text_Set_Text(s_staticTextNumberOfUnitsLost, str);
            Gadget_Static_Text_Set_Text(s_staticTextPlayerName, m_observerPlayer->Get_Player_Display_Name());
            s_staticTextPlayerName->Win_Set_Enabled_Text_Colors(m_observerPlayer->Get_Color(), Make_Color(0, 0, 0, 255));
            s_winFlag->Win_Set_Enabled_Image(0, m_observerPlayer->Get_Player_Template()->Get_Flag_Watermark_Image());
            s_winGeneralPortrait->Win_Hide(false);
        } else {
            s_observerPlayerInfoWindow->Win_Hide(true);
            s_observerPlayerListWindow->Win_Hide(false);
            Populate_Observer_List();
        }
    }
}
