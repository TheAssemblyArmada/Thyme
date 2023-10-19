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
#include "controlbar.h"
#include "gadgetpushbutton.h"

void ControlBar::Reset_Common_Command_Data()
{
    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        m_commonCommands[i] = nullptr;
        Gadget_Button_Draw_Overlay_Image(m_commandWindows[i], nullptr);
    }
}

void ControlBar::Add_Common_Commands(Drawable *draw, bool first_drawable)
{
    if (draw != nullptr) {
        Object *obj = draw->Get_Object();

        if (obj != nullptr) {
            if (!obj->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
                const CommandSet *set = Find_Command_Set(obj->Get_Command_Set_String());

                if (set != nullptr) {
                    if (first_drawable) {
                        for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                            if (m_commandWindows[i] != nullptr) {
                                const CommandButton *button = set->Get_Command_Button(i);

                                if (button != nullptr) {
                                    if ((button->Get_Options() & COMMAND_OPTION_OK_FOR_MULTI_SELECT) != 0) {
                                        m_commonCommands[i] = button;
                                        m_commandWindows[i]->Win_Hide(false);
                                        m_commandWindows[i]->Win_Enable(true);
                                        Set_Control_Command(m_commandWindows[i], button);
                                    }
                                }
                            }
                        }
                    } else {
                        for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                            if (m_commandWindows[i] != nullptr) {
                                const CommandButton *button = set->Get_Command_Button(i);
                                bool is_attack_move = (button != nullptr && button->Get_Command() == GUI_COMMAND_ATTACK_MOVE)
                                    || (m_commonCommands[i] != nullptr
                                        && m_commonCommands[i]->Get_Command() == GUI_COMMAND_ATTACK_MOVE);

                                if (is_attack_move && m_commonCommands[i] == nullptr) {
                                    m_commonCommands[i] = button;
                                    m_commandWindows[i]->Win_Hide(false);
                                    m_commandWindows[i]->Win_Enable(true);
                                    Set_Control_Command(m_commandWindows[i], button);
                                } else if (button != m_commonCommands[i] && !is_attack_move) {
                                    m_commonCommands[i] = nullptr;
                                    m_commandWindows[i]->Win_Hide(true);
                                }
                            }
                        }
                    }
                } else {
                    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                        m_commonCommands[i] = nullptr;

                        if (m_commandWindows[i] != nullptr) {
                            m_commandWindows[i]->Win_Hide(true);
                        }
                    }
                }
            }
        }
    }
}

void ControlBar::Populate_Multi_Select()
{
    bool first_drawable = true;
    bool check_image = false;
    const Image *image = nullptr;
    Object *obj = nullptr;
    Reset_Common_Command_Data();

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        if (m_commandWindows[i] != nullptr) {
            m_commandWindows[i]->Win_Hide(true);
        }
    }

    captainslog_dbgassert(g_theInGameUI->Get_Select_Count() > 1,
        "Populate_Multi_Select: Can't populate multiselect context cause there are only '%d' things selected",
        g_theInGameUI->Get_Select_Count());
    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();
    captainslog_dbgassert(!drawables->empty(), "Populate_Multi_Select: Drawable list is empty");

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *drawable = *it;

        if (!drawable->Get_Object()->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
            if (drawable != nullptr) {
                if (drawable->Get_Object() != nullptr) {
                    if (!drawable->Get_Object()->Get_Status_Bits().Test(OBJECT_STATUS_SOLD)) {
                        Add_Common_Commands(drawable, first_drawable);
                        first_drawable = false;

                        if (!check_image) {
                            image = drawable->Get_Template()->Get_Selected_Portrait_Image();
                            obj = drawable->Get_Object();
                            check_image = true;
                        } else if (drawable->Get_Template()->Get_Selected_Portrait_Image() != image) {
                            image = nullptr;
                        }
                    }
                }
            }
        }
    }

    Set_Portrait_By_Object(obj);
}

void ControlBar::Update_Context_Multi_Select()
{
    int count[CommandSet::MAX_COMMAND_BUTTONS];
    memset(count, 0, sizeof(count));
    captainslog_dbgassert(g_theInGameUI->Get_Select_Count() > 1,
        "Update_Context_Multi_Select: TheInGameUI only has '%d' things selected",
        g_theInGameUI->Get_Select_Count());
    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();
    captainslog_dbgassert(!drawables->empty(), "Populate_Multi_Select: Drawable list is empty");

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *drawable = *it;

        if (!drawable->Get_Object()->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
            Object *obj = drawable->Get_Object();

            if (obj != nullptr) {
                for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                    GameWindow *window = m_commandWindows[i];

                    if (window != nullptr) {
                        if (!window->Win_Is_Hidden()) {
                            CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(window));

                            if (button != nullptr) {
                                CommandAvailability availability =
                                    Get_Command_Availability(button, obj, window, nullptr, false);
                                window->Win_Clear_Status(COMMAND_OPTION_CAN_USE_WAYPOINTS);
                                window->Win_Clear_Status(WIN_STATUS_ALWAYS_COLOR);

                                switch (availability) {
                                    case COMMAND_AVAILABILITY_DISABLED:
                                        window->Win_Enable(false);
                                        break;
                                    case COMMAND_AVAILABILITY_HIDDEN:
                                        window->Win_Hide(true);
                                        break;
                                    case COMMAND_AVAILABILITY_NOT_READY:
                                        window->Win_Enable(false);
                                        window->Win_Set_Status(WIN_STATUS_NOT_READY);
                                        break;
                                    case COMMAND_AVAILABILITY_DISABLED_PERMANENTLY:
                                        window->Win_Enable(false);
                                        window->Win_Set_Status(WIN_STATUS_ALWAYS_COLOR);
                                        break;
                                    default:
                                        window->Win_Enable(true);
                                        break;
                                }

                                if ((button->Get_Options() & COMMAND_OPTION_CHECK_LIKE) != 0) {
                                    Gadget_Check_Like_Button_Set_Visual_Check(
                                        window, availability == COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE);
                                }

                                if (availability == COMMAND_AVAILABILITY_ENABLED
                                    || availability == COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE) {
                                    count[i]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        if (m_commandWindows[i] != nullptr && !m_commandWindows[i]->Win_Is_Hidden() && m_commonCommands[i] != nullptr) {
            if (count[i] <= 0) {
                m_commandWindows[i]->Win_Enable(false);
            } else {
                m_commandWindows[i]->Win_Enable(true);
            }
        }
    }
}
