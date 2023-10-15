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
#include "buildassistant.h"
#include "controlbar.h"
#include "disconnectmenu.h"
#include "displaystringmanager.h"
#include "drawable.h"
#include "gadgetpushbutton.h"
#include "gadgetstatictext.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "ingameui.h"
#include "object.h"
#include "overchargebehavior.h"
#include "player.h"
#include "playerlist.h"
#include "productionprerequisite.h"
#include "scriptengine.h"
#include "thingtemplate.h"
#include "upgrade.h"
#include "windowlayout.h"

AnimateWindowManager *g_theAnimateWindowManager = nullptr;
GameWindow *g_prevWindow = nullptr;
bool g_useAnimation = false;

void Control_Bar_Popup_Description_Update_Func(WindowLayout *layout, void *user_data)
{
    if (g_theScriptEngine->Is_End_Game_Timer_Running()) {
        g_theControlBar->Hide_Build_Tooltip_Layout();
    }

    if (g_theAnimateWindowManager != nullptr && !g_theControlBar->Get_Build_Tooltip_Layout_Visible()
        && !g_theAnimateWindowManager->Is_Reversed()) {
        g_theAnimateWindowManager->Reverse_Animate_Window();
    } else if (!g_theControlBar->Get_Build_Tooltip_Layout_Visible()
        && (!g_theWriteableGlobalData->m_animateWindows || !g_useAnimation)) {
        g_theControlBar->Delete_Build_Tooltip_Layout();
    }

    if (g_useAnimation) {
        if (g_theAnimateWindowManager != nullptr) {
            if (g_theWriteableGlobalData->m_animateWindows) {
                bool finished = g_theAnimateWindowManager->Is_Finished();
                g_theAnimateWindowManager->Update();

                if (g_theAnimateWindowManager != nullptr) {
                    if (g_theAnimateWindowManager->Is_Finished() && !finished && g_theAnimateWindowManager->Is_Reversed()) {
                        if (g_theAnimateWindowManager != nullptr) {
                            delete g_theAnimateWindowManager;
                        }

                        g_theAnimateWindowManager = nullptr;
                        g_theControlBar->Delete_Build_Tooltip_Layout();
                    }
                }
            }
        }
    }
}

void ControlBar::Show_Build_Tooltip_Layout(GameWindow *window)
{
    static bool is_initialized = false;
    static unsigned int begin_wait_time = 0;

    if (g_theInGameUI->Are_Tooltips_Disabled() || g_theScriptEngine->Is_End_Game_Timer_Running()) {
        return;
    }

    bool show = false;

    if (g_prevWindow == window) {
        m_buildTooltipLayoutVisible = true;

        if (!is_initialized) {
            if (window->Get_Tooltip_Delay() + begin_wait_time < rts::Get_Time()) {
                show = true;
            }
        }

        if (!show) {
            return;
        }
    } else if (!m_controlBarPopupDescriptionLayout->Is_Hidden()) {
        if (g_useAnimation && g_theWriteableGlobalData->m_animateWindows && !g_theAnimateWindowManager->Is_Reversed()) {
            g_theAnimateWindowManager->Reverse_Animate_Window();
        } else if (!g_useAnimation || !g_theWriteableGlobalData->m_animateWindows
            || !g_theAnimateWindowManager->Is_Reversed()) {
            m_controlBarPopupDescriptionLayout->Hide(true);
            g_prevWindow = nullptr;
        }

        return;
    }

    if (!show) {
        g_prevWindow = window;
        begin_wait_time = rts::Get_Time();
        is_initialized = false;
        return;
    }

    is_initialized = true;

    if (window != nullptr) {
        if ((window->Win_Get_Style() & GWS_PUSH_BUTTON) != 0) {
            CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(window));

            if (button == nullptr || g_theGameLogic->Is_In_Replay_Game() || g_theInGameUI->Is_Quit_Menu_Visible()
                || (g_theDisconnectMenu != nullptr && g_theDisconnectMenu->Is_Screen_Visible())) {
                return;
            }

            m_buildTooltipLayoutVisible = true;
            Populate_Build_Tooltip_Layout(button, nullptr);
        } else {
            if ((window->Win_Get_Style() & GWS_USER_WINDOW) == 0 && (window->Win_Get_Style() & GWS_STATIC_TEXT) == 0) {
                return;
            }

            Populate_Build_Tooltip_Layout(nullptr, window);
        }

        m_controlBarPopupDescriptionLayout->Hide(false);

        if (g_useAnimation && g_theWriteableGlobalData->m_animateWindows) {
            g_theAnimateWindowManager = new AnimateWindowManager();
            g_theAnimateWindowManager->Reset();
            g_theAnimateWindowManager->Register_Game_Window(
                m_controlBarPopupDescriptionLayout->Get_Window_List(), WIN_ANIMATION_SLIDE_RIGHT_FAST, true, 200, 0);
        }
    }
}

void ControlBar::Repopulate_Build_Tooltip_Layout()
{
    if (g_prevWindow != nullptr && m_controlBarPopupDescriptionLayout != nullptr
        && (g_prevWindow->Win_Get_Style() & GWS_PUSH_BUTTON) != 0) {
        CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(g_prevWindow));
        Populate_Build_Tooltip_Layout(button, nullptr);
    }
}

void ControlBar::Populate_Build_Tooltip_Layout(const CommandButton *button, GameWindow *window)
{
    if (m_controlBarPopupDescriptionLayout == nullptr) {
        return;
    }

    Player *player = g_thePlayerList->Get_Local_Player();
    Utf16String name_str;
    Utf16String cost_str;
    Utf16String description_string;
    Utf16String temp_string1(Utf16String::s_emptyString);
    Utf16String temp_string2;

    bool skip_append_comma = true;
    bool skip_cost = false;
    unsigned int cost = 0;

    if (button != nullptr) {
        const ThingTemplate *thing = *button->Get_Template();
        const UpgradeTemplate *upgrade = button->Get_Upgrade_Template();
        ScienceType science = SCIENCE_INVALID;

        if (button->Get_Command() != GUI_COMMAND_PLAYER_UPGRADE && button->Get_Command() != GUI_COMMAND_OBJECT_UPGRADE) {
            if (button->Get_Sciences()->size() > 1) {
                for (unsigned int i = 0; i < button->Get_Sciences()->size(); i++) {
                    unsigned int index = i;
                    science = (*button->Get_Sciences())[index];

                    if (button->Get_Command() != GUI_COMMAND_PURCHASE_SCIENCE) {
                        if (!player->Has_Science(science) && i > 0) {
                            index = i - 1;
                            science = (*button->Get_Sciences())[index];
                        }

                        skip_cost = true;
                        break;
                    }

                    if (!player->Has_Science(science)) {
                        break;
                    }
                }
            } else if (button->Get_Sciences()->size() == 1) {
                science = (*button->Get_Sciences())[0];

                if (button->Get_Command() != GUI_COMMAND_PURCHASE_SCIENCE) {
                    skip_cost = true;
                }
            }
        }

        if (button->Get_Descript_Label().Is_Not_Empty()) {
            description_string = g_theGameText->Fetch(button->Get_Descript_Label());
            Drawable *drawable = g_theInGameUI->Get_First_Selected_Drawable();
            Object *object = drawable ? drawable->Get_Object() : nullptr;

            if (object != nullptr) {
                if (button->Get_Command() == GUI_COMMAND_TOGGLE_OVERCHARGE) {
                    for (BehaviorModule **module = object->Get_All_Modules(); *module != nullptr; module++) {
                        OverchargeBehaviorInterface *overcharge = (*module)->Get_Overcharge_Behavior_Interface();

                        if (overcharge != nullptr) {
                            description_string.Concat(U_CHAR("\n"));

                            if (overcharge->Is_Overcharge_Active()) {
                                description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipNukeReactorOverChargeIsOn"));
                            } else {
                                description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipNukeReactorOverChargeIsOff"));
                            }
                        }
                    }
                } else if (thing != nullptr) {
                    CanMakeType type = g_theBuildAssistant->Can_Make_Unit(object, *button->Get_Template());

                    switch (type) {
                        case CAN_MAKE_NOT_ENOUGH_MONEY:
                            description_string.Concat(U_CHAR("\n\n"));
                            description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipNotEnoughMoneyToBuild"));
                            break;
                        case CAN_MAKE_QUEUE_FULL:
                            description_string.Concat(U_CHAR("\n\n"));
                            description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipCannotPurchaseBecauseQueueFull"));
                            break;
                        case CAN_MAKE_PARKING_FULL:
                            description_string.Concat(U_CHAR("\n\n"));
                            description_string.Concat(
                                g_theGameText->Fetch("TOOLTIP:TooltipCannotBuildUnitBecauseParkingFull"));
                            break;
                        case CAN_MAKE_MAXIMUM_NUMBER:
                            description_string.Concat(U_CHAR("\n\n"));

                            if (thing->Is_KindOf(KINDOF_STRUCTURE)) {
                                description_string.Concat(
                                    g_theGameText->Fetch("TOOLTIP:TooltipCannotBuildBuildingBecauseMaximumNumber"));
                            } else {
                                description_string.Concat(
                                    g_theGameText->Fetch("TOOLTIP:TooltipCannotBuildUnitBecauseMaximumNumber"));
                            }
                            break;
                        default:
                            break;
                    }
                } else if (upgrade != nullptr && !player->Has_Upgrade_In_Production(upgrade)
                    && (button->Get_Command() == GUI_COMMAND_PLAYER_UPGRADE
                        || button->Get_Command() == GUI_COMMAND_OBJECT_UPGRADE)) {
                    ProductionUpdateInterface *production = object->Get_Production_Update_Interface();

                    if (production != nullptr && production->Get_Production_Count() == 9) {
                        description_string.Concat(U_CHAR("\n\n"));
                        description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipCannotPurchaseBecauseQueueFull"));
                    } else {
                        if (!g_theUpgradeCenter->Can_Afford_Upgrade(g_thePlayerList->Get_Local_Player(), upgrade, false)) {
                            description_string.Concat(g_theGameText->Fetch("TOOLTIP:TooltipNotEnoughMoneyToBuild"));
                        }
                    }
                }
            }
        }

        name_str = g_theGameText->Fetch(button->Get_Text_Label().Str());

        if (thing == nullptr || button->Get_Command() == GUI_COMMAND_PURCHASE_SCIENCE) {
            if (upgrade != nullptr) {
                bool has_upgrade = player->Has_Upgrade_Complete(upgrade);
                bool is_not_affected_by_upgrade = false;
                bool is_missing_science = false;
                bool is_player_upgrade = button->Get_Command() == GUI_COMMAND_PLAYER_UPGRADE;
                bool is_object_upgrade = button->Get_Command() == GUI_COMMAND_OBJECT_UPGRADE;

                if (!has_upgrade) {
                    const Drawable *drawable = g_theInGameUI->Get_First_Selected_Drawable();

                    if (drawable != nullptr) {
                        const Object *object = drawable->Get_Object();

                        if (object != nullptr) {
                            has_upgrade = object->Has_Upgrade(upgrade);

                            if (is_object_upgrade) {
                                is_not_affected_by_upgrade = !object->Affected_By_Upgrade(upgrade);
                            }
                        }
                    }
                }

                if (!is_not_affected_by_upgrade || has_upgrade) {
                    if (has_upgrade && (is_player_upgrade || is_object_upgrade)) {
                        if (button->Get_Purchased_Label().Is_Not_Empty()) {
                            description_string = g_theGameText->Fetch(button->Get_Purchased_Label());

                        } else {
                            description_string = g_theGameText->Fetch("TOOLTIP:AlreadyUpgradedDefault");
                        }
                    } else if (!has_upgrade) {
                        for (unsigned int i = 0; i < button->Get_Sciences()->size(); i++) {
                            science = (*button->Get_Sciences())[i];

                            if (!player->Has_Science(science)) {
                                is_missing_science = true;
                                break;
                            }
                        }

                        cost = upgrade->Calc_Cost_To_Build(player);

                        if (cost != 0) {
                            cost_str.Format(g_theGameText->Fetch("TOOLTIP:Cost"), cost);
                        }

                        if (is_missing_science) {
                            if (!description_string.Is_Empty()) {
                                description_string.Concat(U_CHAR("\n"));
                            }

                            temp_string1.Format(g_theGameText->Fetch("CONTROLBAR:Requirements").Str(),
                                g_theGameText->Fetch("CONTROLBAR:GeneralsPromotion").Str());
                            description_string.Concat(temp_string1);
                        }
                    }
                } else {
                    if (button->Get_Conflicting_Label().Is_Not_Empty()) {
                        description_string = g_theGameText->Fetch(button->Get_Conflicting_Label());
                    } else {
                        description_string = g_theGameText->Fetch("TOOLTIP:HasConflictingUpgradeDefault");
                    }
                }
            } else if (science != SCIENCE_INVALID && !skip_cost) {
                g_theScienceStore->Get_Name_And_Description(science, name_str, description_string);
                cost = g_theScienceStore->Get_Science_Purchase_Cost(science);

                if (cost != 0) {
                    cost_str.Format(g_theGameText->Fetch("TOOLTIP:ScienceCost"), cost);
                }

                if (thing != nullptr) {
                    for (int j = 0; j < thing->Get_Prereq_Count(); j++) {
                        const ProductionPrerequisite *prerequisite = thing->Get_Nth_Prereq(j);
                        temp_string2 = prerequisite->Get_Requires_List(player);

                        if (temp_string2 != Utf16String::s_emptyString) {
                            if (skip_append_comma) {
                                skip_append_comma = false;
                            } else {
                                temp_string1.Concat(U_CHAR(", "));
                            }
                        }

                        temp_string1.Concat(temp_string2);
                    }

                    if (!temp_string1.Is_Empty()) {
                        temp_string1.Format(g_theGameText->Fetch("CONTROLBAR:Requirements"), temp_string1.Str());

                        if (!description_string.Is_Empty()) {
                            description_string.Concat(U_CHAR("\n"));
                        }

                        description_string.Concat(temp_string1);
                    }
                }
            }
        } else {
            cost = thing->Calc_Cost_To_Build(player);

            if (cost != 0) {
                cost_str.Format(g_theGameText->Fetch("TOOLTIP:Cost"), cost);
            }

            for (int j = 0; j < thing->Get_Prereq_Count(); j++) {
                const ProductionPrerequisite *prerequisite = thing->Get_Nth_Prereq(j);
                temp_string2 = prerequisite->Get_Requires_List(player);

                if (temp_string2 != Utf16String::s_emptyString) {
                    if (skip_append_comma) {
                        skip_append_comma = false;
                    } else {
                        temp_string1.Concat(U_CHAR(", "));
                    }
                }

                temp_string1.Concat(temp_string2);
            }

            if (!temp_string1.Is_Empty()) {
                temp_string1.Format(g_theGameText->Fetch("CONTROLBAR:Requirements"), temp_string1.Str());

                if (!description_string.Is_Empty()) {
                    description_string.Concat(U_CHAR("\n"));
                }

                description_string.Concat(temp_string1);
            }
        }
    } else if (window != nullptr) {
        if (window
            == g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
                g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:MoneyDisplay"))) {
            name_str = g_theGameText->Fetch("CONTROLBAR:Money");
            description_string = g_theGameText->Fetch("CONTROLBAR:MoneyDescription");
        } else if (window
            == g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
                g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PowerWindow"))) {
            name_str = g_theGameText->Fetch("CONTROLBAR:Power");
            description_string = g_theGameText->Fetch("CONTROLBAR:PowerDescription");

            player = nullptr;

            if (g_theControlBar->Is_Observer()) {
                player = g_theControlBar->Get_Observer_Player();
            } else {
                player = g_thePlayerList->Get_Local_Player();
            }

            if (player != nullptr && player->Get_Energy() != nullptr) {
                const Energy *energy = player->Get_Energy();
                description_string.Format(description_string, energy->Get_Production(), energy->Get_Consumption());
            } else {
                description_string.Format(description_string, 0, 0);
            }
        } else {
            if (window
                != g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
                    g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:GeneralsExp"))) {
                captainslog_dbgassert(false,
                    "ControlBar::Populate_Build_Tooltip_Layout We attempted to call the popup tooltip on a game window "
                    "that has yet to be hand coded in as this fuction was/is designed for only buttons but has been "
                    "hacked to work with GameWindows.");
                return;
            }

            name_str = g_theGameText->Fetch("CONTROLBAR:GeneralsExp");
            description_string = g_theGameText->Fetch("CONTROLBAR:GeneralsExpDescription");
        }
    }

    GameWindow *win = g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
        g_theNameKeyGenerator->Name_To_Key("ControlBarPopupDescription.wnd:StaticTextName"));

    if (win != nullptr) {
        Gadget_Static_Text_Set_Text(win, name_str);
    }

    win = g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
        g_theNameKeyGenerator->Name_To_Key("ControlBarPopupDescription.wnd:StaticTextCost"));

    if (win != nullptr) {
        if (cost != 0) {
            win->Win_Hide(false);
            Gadget_Static_Text_Set_Text(win, cost_str);
        } else {
            win->Win_Hide(true);
        }
    }

    win = g_theWindowManager->Win_Get_Window_From_Id(m_controlBarPopupDescriptionLayout->Get_Window_List(),
        g_theNameKeyGenerator->Name_To_Key("ControlBarPopupDescription.wnd:StaticTextDescription"));

    if (win != nullptr) {
        static NameKeyType win_name_key = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BackgroundMarker");
        DisplayString *display_string = g_theDisplayStringManager->New_Display_String();

        int width;
        int height;
        win->Win_Get_Size(&width, &height);
        display_string->Set_Font(win->Win_Get_Font());
        display_string->Set_Word_Wrap(width - 10);
        display_string->Set_Text(description_string);

        int text_w;
        int text_h;
        display_string->Get_Size(&text_w, &text_h);
        g_theDisplayStringManager->Free_Display_String(display_string);
        display_string = nullptr;
        int add_text_h = text_h - height;

        GameWindow *win_list = m_controlBarPopupDescriptionLayout->Get_Window_List();

        if (win_list == nullptr) {
            return;
        }

        win_list->Win_Get_Size(&width, &height);

        if (add_text_h + height < 102) {
            add_text_h = 102 - height;
        }

        win_list->Win_Set_Size(width, add_text_h + height);

        int x_pos;
        int y_pos;
        win_list->Win_Get_Position(&x_pos, &y_pos);
        GameWindow *background_win = g_theWindowManager->Win_Get_Window_From_Id(nullptr, win_name_key);

        if (background_win == nullptr) {
            return;
        }

        static ICoord2D base_pos;
        static ICoord2D last_offset;
        g_theControlBar->Get_Background_Marker_Pos(&base_pos.x, &base_pos.y);

        int screen_x;
        int screen_y;
        background_win->Win_Get_Screen_Position(&screen_x, &screen_y);

        ICoord2D c;
        c.x = screen_x - base_pos.x;
        c.y = screen_y - base_pos.y;

        win_list->Win_Set_Position(x_pos, screen_y - base_pos.y - last_offset.y + y_pos - add_text_h);
        last_offset = c;
        win->Win_Get_Size(&width, &height);
        win->Win_Set_Size(width, add_text_h + height);
        Gadget_Static_Text_Set_Text(win, description_string);
    }

    m_controlBarPopupDescriptionLayout->Hide(false);
}

void ControlBar::Hide_Build_Tooltip_Layout()
{
    if (g_theAnimateWindowManager == nullptr || !g_theAnimateWindowManager->Is_Reversed()) {
        if (g_useAnimation && g_theAnimateWindowManager != nullptr && g_theWriteableGlobalData->m_animateWindows) {
            g_theAnimateWindowManager->Reverse_Animate_Window();
        } else {
            Delete_Build_Tooltip_Layout();
        }
    }
}

void ControlBar::Delete_Build_Tooltip_Layout()
{
    m_buildTooltipLayoutVisible = false;
    g_prevWindow = nullptr;
    m_controlBarPopupDescriptionLayout->Hide(true);

    if (g_theAnimateWindowManager != nullptr) {
        delete g_theAnimateWindowManager;
    }

    g_theAnimateWindowManager = nullptr;
}
