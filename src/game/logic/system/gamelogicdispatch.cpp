/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "controlbar.h"
#include "gameengine.h"
#include "gamelogic.h"
#include "gamemessage.h"
#include "gamewindowmanager.h"
#include "gamewindowtransitions.h"
#include "globaldata.h"
#include "mouse.h"
#include "scriptactions.h"
#include "scriptengine.h"
#include "shell.h"
#include "statscollector.h"
#include "windowlayout.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void Fixup_Score_Screen_Movie_Window()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x0068B7E0, 0x00ABC38F));
#endif
}

void GameLogic::Close_Windows()
{
    // needs various UI functions
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x005FA0F0, 0x009F49A0));
#endif
}

void GameLogic::Clear_Game_Data(bool b)
{
    if (Is_In_Game()) {
        Set_Clear_Game_Data(true);

        if (g_theStatsCollector != nullptr) {
            g_theStatsCollector->Write_File_End();
        }

        g_theScriptActions->Close_Windows(false);

        if ((!Is_In_Shell_Game() || !Is_In_Game()) && b) {
            g_theTransitionHandler->Set_Group("FadeWholeScreen", false);
            g_theShell->Push("Menus/ScoreScreen.wnd", false);
            g_theShell->Show_Shell(0);
            g_theTransitionHandler->Reverse("FadeWholeScreen");
            Fixup_Score_Screen_Movie_Window();
        }

        g_theGameEngine->Reset();
        Set_Game_Mode(GAME_NONE);

        if (!g_theWriteableGlobalData->m_initialFile.Is_Empty()) {
            g_theGameEngine->Set_Quitting(true);
        }

        Hide_Control_Bar(true);
        Close_Windows();
        g_theMouse->Set_Visibility(true);

        if (m_background != nullptr) {
            m_background->Destroy_Windows();
            m_background->Delete_Instance();
            m_background = nullptr;
        }

        Set_Clear_Game_Data(false);
    } else {
        captainslog_dbgassert(false, "We tried to clear the game data when we weren't in a game");
    }
}

void GameLogic::Prepare_New_Game(int mode, GameDifficulty difficulty, int rank_points)
{
    g_theScriptEngine->Set_Global_Difficulty(difficulty);

    if (m_background == nullptr) {
        m_background = g_theWindowManager->Win_Create_Layout("Menus/BlankWindow.wnd");
        captainslog_dbgassert(m_background != nullptr, "We Couldn't Load Menus/BlankWindow.wnd");
        m_background->Hide(false);
        m_background->Bring_Forward();
    }

    m_background->Get_Window_List()->Win_Clear_Status(WIN_STATUS_IMAGE);
    g_theGameLogic->Set_Game_Mode(static_cast<GameMode>(mode));

    if (!g_theWriteableGlobalData->m_pendingFile.Is_Empty()) {
        g_theWriteableGlobalData->m_mapName = g_theWriteableGlobalData->m_pendingFile;
        g_theWriteableGlobalData->m_pendingFile.Clear();
    }

    m_rankPointsToAddAtGameStart = rank_points;
    captainslog_debug("GameLogic::Prepare_New_Game() - m_rankPointsToAddAtGameStart = %d", m_rankPointsToAddAtGameStart);

    if (!g_theGameLogic->Is_In_Shell_Game()) {
        g_theShell->Hide_Shell();
    }

    m_startNewGame = false;
}

void GameLogic::Logic_Message_Dispatcher(GameMessage *msg, void *user_data)
{
#ifdef GAME_DLL
    Call_Method<void, GameLogic, GameMessage *, void *>(PICK_ADDRESS(0x005FA4E0, 0x009F4DA7), this, msg, user_data);
#endif
}
