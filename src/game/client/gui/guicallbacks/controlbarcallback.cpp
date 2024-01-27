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
#include "animatewindowmanager.h"
#include "commandxlat.h"
#include "controlbar.h"
#include "diplomacy.h"
#include "display.h"
#include "gadgettextentry.h"
#include "gameclient.h"
#include "globaldata.h"
#include "languagefilter.h"
#include "mouse.h"
#include "player.h"
#include "playerlist.h"
#include "quitmenu.h"
#include "radar.h"
#include "scriptengine.h"
#include "view.h"

void Hide_Control_Bar(bool immediate)
{
    Hide_Replay_Controls();

    if (g_theControlBar != nullptr) {
        g_theControlBar->Hide_Special_Power_Shortcut();
    }

    if (g_theWindowManager != nullptr) {
        GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ControlBarParent"));

        if (window != nullptr) {
            g_theTacticalView->Set_Height(g_theDisplay->Get_Height());
        }

        if (immediate) {
            window->Win_Hide(true);

            if (g_theControlBar != nullptr) {
                g_theControlBar->Hide_Special_Power_Shortcut();
            }
        } else {
            g_theControlBar->m_controlBarAnimateWindowManager->Reverse_Animate_Window();
            g_theControlBar->Animate_Special_Power_Shortcut(false);
        }

        if (g_theControlBar != nullptr) {
            g_theControlBar->Hide_Purchase_Science();
        }
    }
}

void Show_Control_Bar(bool immediate)
{
    Show_Replay_Controls();

    if (g_theControlBar != nullptr) {
        g_theControlBar->Show_Special_Power_Shortcut();
    }

    if (g_theWindowManager != nullptr) {
        GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ControlBarParent"));

        if (window != nullptr) {
            g_theControlBar->Switch_Control_Bar_Stage(CONTROL_BAR_STAGE_DEFAULT);
            g_theTacticalView->Set_Height(g_theDisplay->Get_Height() * 0.8f);

            if (g_theControlBar->m_controlBarAnimateWindowManager != nullptr && !immediate) {
                g_theControlBar->m_controlBarAnimateWindowManager->Reset();
                g_theControlBar->m_controlBarAnimateWindowManager->Register_Game_Window(
                    window, WIN_ANIMATION_SLIDE_BOTTOM, true, 500, 0);
                g_theControlBar->Animate_Special_Power_Shortcut(true);
            }

            window->Win_Hide(false);
        }
    }

    if (g_theControlBar != nullptr) {
        g_theControlBar->Mark_UI_Dirty();
    }
}

void Toggle_Control_Bar(bool immediate)
{
    Toggle_Replay_Controls();

    if (g_theWindowManager != nullptr) {
        GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ControlBarParent"));

        if (window != nullptr) {
            if (window->Win_Is_Hidden()) {
                if (g_theControlBar != nullptr) {
                    g_theControlBar->Show_Special_Power_Shortcut();
                }

                g_theTacticalView->Set_Height(g_theDisplay->Get_Height() * 0.8f);
                window->Win_Hide(!window->Win_Is_Hidden());
                g_theControlBar->Switch_Control_Bar_Stage(CONTROL_BAR_STAGE_DEFAULT);

                if (g_theControlBar->m_controlBarAnimateWindowManager != nullptr) {
                    if (!immediate) {
                        g_theControlBar->m_controlBarAnimateWindowManager->Reset();
                        g_theControlBar->m_controlBarAnimateWindowManager->Register_Game_Window(
                            window, WIN_ANIMATION_SLIDE_BOTTOM, true, 500, 0);
                        g_theControlBar->Animate_Special_Power_Shortcut(true);
                    }
                }
            } else {
                if (g_theControlBar != nullptr) {
                    g_theControlBar->Hide_Special_Power_Shortcut();
                }

                g_theTacticalView->Set_Height(g_theDisplay->Get_Height());
                window->Win_Hide(!window->Win_Is_Hidden());
            }
        }
    }
}

WindowMsgHandledType Left_HUD_Input(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    Player *player = g_thePlayerList->Get_Local_Player();

    if (!g_theRadar->Is_Radar_Forced() && (g_theRadar->Is_Radar_Hidden() || !player->Has_Radar())) {
        return MSG_HANDLED;
    }

    if (g_theMouse->Get_Mouse_Status()->middle_state == 1) {
        return MSG_IGNORED;
    }

    switch (message) {
        case GWM_NONE:
        case GWM_MOUSE_ENTERING:
        case GWM_MOUSE_LEAVING: {
            bool needs_target = false;
            const CommandButton *command = g_theInGameUI->Get_GUI_Command();

            if (command != nullptr
                && (command->Get_Command() == GUI_COMMAND_SPECIAL_POWER
                    || command->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT)
                && (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) != 0) {
                needs_target = true;
            }

            if (!needs_target) {
                const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Local_Drawables();
                MouseCursor cursor = CURSOR_ARROW;

                if (!drawables->empty() && message != GWM_MOUSE_LEAVING) {
                    if (command != nullptr && command->Get_Command() == GUI_COMMAND_ATTACK_MOVE) {
                        cursor = CURSOR_ATTACK_MOVE;
                    } else {
                        cursor = CURSOR_MOVE;
                    }
                }

                g_theMouse->Set_Cursor(cursor);
            }

            return MSG_HANDLED;
        }
        case GWM_LEFT_DOWN:
        case GWM_RIGHT_DOWN: {
            int width;
            int height;
            int screen_x;
            int screen_y;
            window->Win_Get_Size(&width, &height);
            window->Win_Get_Screen_Position(&screen_x, &screen_y);
            ICoord2D pixel;
            pixel.x = (data_1 & 0xffff) - screen_x;
            pixel.y = ((data_1 >> 16) & 0xffff) - screen_y;
            ICoord2D radar;
            Coord3D world;

            if ((!g_theRadar->Is_Radar_Hidden() || g_theRadar->Is_Radar_Forced())
                && g_theRadar->Local_Pixel_To_Radar(&pixel, &radar) && g_theRadar->Radar_To_World(&radar, &world)) {
                if (g_theInGameUI->Get_All_Selected_Local_Drawables()->empty()
                    || (!g_theWriteableGlobalData->m_alternateMouseEnabled && message == GWM_RIGHT_DOWN)
                    || (g_theWriteableGlobalData->m_alternateMouseEnabled && message == GWM_LEFT_DOWN)) {
                    g_theTacticalView->Look_At(&world);
                } else {
                    const CommandButton *command = g_theInGameUI->Get_GUI_Command();

                    if (command != nullptr
                        && (command->Get_Command() == GUI_COMMAND_SPECIAL_POWER
                            || command->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT)
                        && (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) != 0) {
                        g_theGameClient->Evaluate_Context_Command(nullptr, &world, CommandTranslator::DO_COMMAND);
                    } else if (command != nullptr && command->Get_Command() == GUI_COMMAND_ATTACK_MOVE) {
                        GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_DO_ATTACKMOVETO);
                        msg->Append_Location_Arg(world);
                        Pick_And_Play_Unit_Voice_Response(
                            g_theInGameUI->Get_All_Selected_Drawables(), GameMessage::MSG_DO_ATTACKMOVETO, nullptr);
                    } else {
                        GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_DO_MOVETO);
                        msg->Append_Location_Arg(world);
                        Pick_And_Play_Unit_Voice_Response(
                            g_theInGameUI->Get_All_Selected_Local_Drawables(), GameMessage::MSG_DO_MOVETO, nullptr);
                    }
                }
            }

            g_theInGameUI->Clear_Attack_Move_To_Mode();
            return MSG_HANDLED;
        }
        case GWM_LEFT_UP:
        case GWM_RIGHT_UP: {
            g_theInGameUI->Clear_Attack_Move_To_Mode();
            return MSG_HANDLED;
        }
        case GWM_MOUSE_POS: {
            int screen_x;
            int screen_y;
            window->Win_Get_Screen_Position(&screen_x, &screen_y);
            ICoord2D pixel;
            pixel.x = (data_1 & 0xffff) - screen_x;
            pixel.y = ((data_1 >> 16) & 0xffff) - screen_y;
            ICoord2D radar;

            if ((!g_theRadar->Is_Radar_Hidden() || g_theRadar->Is_Radar_Forced())
                && g_theRadar->Local_Pixel_To_Radar(&pixel, &radar)) {
                const CommandButton *command = g_theInGameUI->Get_GUI_Command();

                if (command != nullptr
                    && (command->Get_Command() == GUI_COMMAND_SPECIAL_POWER
                        || command->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT)
                    && (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) != 0) {
                    MouseCursor index = g_theMouse->Get_Cursor_Index(command->Get_Cursor_Name());

                    if (index == CURSOR_INVALID) {
                        g_theMouse->Set_Cursor(CURSOR_TARGET);
                    } else {
                        g_theMouse->Set_Cursor(index);
                    }
                } else {
                    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Local_Drawables();
                    MouseCursor cursor = CURSOR_ARROW;

                    if (!drawables->empty() && message != GWM_MOUSE_LEAVING) {
                        if (command != nullptr && command->Get_Command() == GUI_COMMAND_ATTACK_MOVE) {
                            cursor = CURSOR_ATTACK_MOVE;
                        } else {
                            cursor = CURSOR_MOVE;
                        }
                    }

                    g_theMouse->Set_Cursor(cursor);
                }
            }

            g_theInGameUI->Clear_Attack_Move_To_Mode();
            return MSG_HANDLED;
        }
        default: {
            return MSG_IGNORED;
        }
    }
}

WindowMsgHandledType Control_Bar_Input(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    return MSG_IGNORED;
}

WindowMsgHandledType Control_Bar_System(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    GameWindow *window2 = reinterpret_cast<GameWindow *>(data_1);
    static NameKeyType buttonCommunicator;

    if (g_theScriptEngine != nullptr && g_theScriptEngine->Is_End_Game_Timer_Running()) {
        return MSG_IGNORED;
    }

    if (message > GBM_SELECTED_RIGHT) {
        if (message == GEM_EDIT_DONE) {
            int id = window2->Win_Get_Window_Id();
            static const NameKeyType textID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:EditBeaconText");

            if (id == textID && g_theInGameUI->Get_Select_Count() == 1) {
                GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_SET_BEACON_TEXT);
                Utf16String text = Gadget_Text_Entry_Get_Text(window2);
                g_theLanguageFilter->Filter_Line(text);
                const unichar_t *str = text.Str();

                while (str != nullptr && *str != U_CHAR('\0')) {
                    msg->Append_Wide_Char_Arg(*str);
                    str++;
                }

                msg->Append_Wide_Char_Arg(U_CHAR('\0'));
            }

            return MSG_HANDLED;
        }

        return MSG_IGNORED;
    }

    if (message >= GBM_SELECTED) {
        static const NameKeyType beaconPlacementButtonID =
            g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonPlaceBeacon");
        static const NameKeyType beaconDeleteButtonID =
            g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonDeleteBeacon");
        static const NameKeyType beaconClearTextButtonID =
            g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonClearBeaconText");
        static const NameKeyType beaconGeneralButtonID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonGeneral");
        static const NameKeyType buttonLargeID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonLarge");
        static const NameKeyType buttonOptions = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonOptions");
        static const NameKeyType buttonIdleWorker = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonIdleWorker");
        int id = window2->Win_Get_Window_Id();

        if (id == buttonCommunicator) {
            Toggle_Diplomacy(false);
        } else if (id == beaconPlacementButtonID && g_theGameLogic->Is_In_Multiplayer_Game()
            && g_thePlayerList->Get_Local_Player()->Is_Player_Active()) {
            g_theInGameUI->Set_GUI_Command(g_theControlBar->Find_Command_Button("Command_PlaceBeacon"));
        } else if (id == beaconDeleteButtonID && g_theGameLogic->Is_In_Multiplayer_Game()) {
            g_theMessageStream->Append_Message(GameMessage::MSG_REMOVE_BEACON);
        } else if (id == beaconClearTextButtonID && g_theGameLogic->Is_In_Multiplayer_Game()) {
            static const NameKeyType textID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:EditBeaconText");
            GameWindow *text_window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, textID);

            if (text_window != nullptr) {
                Gadget_Text_Entry_Set_Text(text_window, Utf16String::s_emptyString);
            }
        } else if (id == beaconGeneralButtonID) {
            Hide_Quit_Menu();
            g_theControlBar->Toggle_Purchase_Science();
        } else if (id == buttonLargeID) {
            g_theControlBar->Toggle_Control_Bar_Stage();
        } else if (id == buttonOptions) {
            Toggle_Quit_Menu();
        } else if (id == buttonIdleWorker) {
            Hide_Quit_Menu();
            g_theInGameUI->Select_Next_Idle_Worker();
        } else {
            g_theControlBar->Process_Context_Sensitive_Button_Click(window2, static_cast<GadgetGameMessage>(message));
        }

        return MSG_HANDLED;
    }

    if (message == GWM_CREATE) {
        buttonCommunicator = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PopupCommunicator");
        return MSG_HANDLED;
    }

    if (message <= GGM_CLOSE) {
        return MSG_IGNORED;
    }

    g_theControlBar->Process_Context_Sensitive_Button_Transition(window2, static_cast<GadgetGameMessage>(message));
    return MSG_HANDLED;
}
