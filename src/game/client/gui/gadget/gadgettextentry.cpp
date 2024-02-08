/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Text Entry
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgettextentry.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "gamewindow.h"
#include "imemanager.h"
#include "keyboard.h"

void Gadget_Text_Entry_Set_Font(GameWindow *text_entry, GameFont *font)
{
    _EntryData *data = static_cast<_EntryData *>(text_entry->Win_Get_User_Data());
    DisplayString *string = text_entry->Win_Get_Instance_Data()->Get_Text_DisplayString();

    if (string != nullptr) {
        string->Set_Font(font);
    }

    DisplayString *tooltip_string = text_entry->Win_Get_Instance_Data()->Get_Tooltip_DisplayString();

    if (tooltip_string != nullptr) {
        tooltip_string->Set_Font(font);
    }

    if (data != nullptr) {
        if (data->m_text != nullptr) {
            data->m_text->Set_Font(font);
        }

        if (data->m_sText != nullptr) {
            data->m_sText->Set_Font(font);
        }
    }
}

WindowMsgHandledType Gadget_Text_Entry_Input(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    _EntryData *data = static_cast<_EntryData *>(text_entry->Win_Get_User_Data());
    WinInstanceData *instance_data = text_entry->Win_Get_Instance_Data();

    if (g_theIMEManager != nullptr && g_theIMEManager->Is_Attached_To(text_entry) && g_theIMEManager->Is_Composing()) {
        return MSG_HANDLED;
    }

    switch (message) {
        case GWM_LEFT_DOWN:
            instance_data->m_state |= 2;
            g_theWindowManager->Win_Set_Focus(text_entry);
            return MSG_HANDLED;
        case GWM_LEFT_DRAG:
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GGM_LEFT_DRAG, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
            }

            return MSG_HANDLED;
        case GWM_MOUSE_ENTERING:
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_MOUSE_ENTERING, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
            }

            return MSG_HANDLED;
        case GWM_MOUSE_LEAVING:
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_MOUSE_LEAVING, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
            }

            return MSG_HANDLED;
        case GWM_CHAR:
            if ((data_2 & 2) != 0 && (data_2 & 0xCC) != 0) {
                return MSG_IGNORED;
            }

            switch (data_1) {
                case Keyboard::KEY_ESCAPE:
                case Keyboard::KEY_CAPITAL:
                case Keyboard::KEY_F1:
                case Keyboard::KEY_F2:
                case Keyboard::KEY_F3:
                case Keyboard::KEY_F4:
                case Keyboard::KEY_F5:
                case Keyboard::KEY_F6:
                case Keyboard::KEY_F7:
                case Keyboard::KEY_F8:
                case Keyboard::KEY_F9:
                case Keyboard::KEY_F10:
                case Keyboard::KEY_F11:
                case Keyboard::KEY_F12:
                case Keyboard::KEY_HOME:
                case Keyboard::KEY_PRIOR:
                case Keyboard::KEY_END:
                case Keyboard::KEY_NEXT:
                case Keyboard::KEY_DELETE:
                    return MSG_IGNORED;
                case Keyboard::KEY_BACK:
                    if ((data_2 & 2) != 0 && data->m_conCharPos == 0 && data->m_charPos != 0) {
                        data->m_text->Remove_Last_Char();
                        data->m_sText->Remove_Last_Char();
                        --data->m_charPos;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                        g_theWindowManager->Win_Send_System_Msg(
                            instance_data->Get_Owner(), GEM_UPDATE_TEXT, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_TAB:
                case Keyboard::KEY_RIGHT:
                case Keyboard::KEY_DOWN:
                    if ((data_2 & 2) != 0) {
                        GameWindow *parent = text_entry->Win_Get_Parent();

                        if (parent != nullptr && (parent->Win_Get_Style() & GWS_COMBO_BOX) == 0) {
                            parent = nullptr;
                        }

                        if (parent != nullptr) {
                            g_theWindowManager->Win_Next_Tab(parent);
                        } else {
                            g_theWindowManager->Win_Next_Tab(text_entry);
                        }
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_UP:
                case Keyboard::KEY_LEFT:
                    if ((data_2 & 2) != 0) {
                        GameWindow *parent = text_entry->Win_Get_Parent();

                        if (parent != nullptr && (parent->Win_Get_Style() & GWS_COMBO_BOX) == 0) {
                            parent = nullptr;
                        }

                        if (parent != nullptr) {
                            g_theWindowManager->Win_Prev_Tab(parent);
                        } else {
                            g_theWindowManager->Win_Prev_Tab(text_entry);
                        }
                    }

                    return MSG_HANDLED;
                default:
                    return MSG_HANDLED;
            }
        case GWM_IME_CHAR:
            if (static_cast<unichar_t>(data_1) == U_CHAR('\r')) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GEM_EDIT_DONE, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
                return MSG_HANDLED;
            }

            if (static_cast<unichar_t>(data_1) == U_CHAR('\0')) {
                return MSG_HANDLED;
            }

            if (!data->m_numericalOnly || g_theWindowManager->Win_Is_Digit(static_cast<unichar_t>(data_1))) {
                if (!data->m_alphaNumericalOnly || g_theWindowManager->Win_Is_Al_Num(static_cast<unichar_t>(data_1))) {
                    if (!data->m_asciiOnly || g_theWindowManager->Win_Is_Ascii(static_cast<unichar_t>(data_1))) {
                        if (data->m_charPos < data->m_maxTextLen - 1) {
                            data->m_text->Add_Char(static_cast<unichar_t>(data_1));
                            data->m_sText->Add_Char(U_CHAR('*'));
                            data->m_charPos++;

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                            g_theWindowManager->Win_Send_System_Msg(
                                instance_data->Get_Owner(), GEM_UPDATE_TEXT, reinterpret_cast<unsigned int>(text_entry), 0);
#endif
                        }

                        return MSG_HANDLED;
                    } else {
                        return MSG_HANDLED;
                    }
                } else {
                    return MSG_HANDLED;
                }
            } else {
                return MSG_HANDLED;
            }
        default:
            return MSG_IGNORED;
    }
}

WindowMsgHandledType Gadget_Text_Entry_System(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    _EntryData *data = static_cast<_EntryData *>(text_entry->Win_Get_User_Data());
    WinInstanceData *instance_data = text_entry->Win_Get_Instance_Data();

    if (message > GWM_INPUT_FOCUS) {
        if (message == GEM_GET_TEXT) {
            *reinterpret_cast<Utf16String *>(data_2) = data->m_text->Get_Text();
        } else {
            if (message != GEM_SET_TEXT) {
                return MSG_IGNORED;
            }

            data->m_text->Set_Text(*reinterpret_cast<Utf16String *>(data_1));
            data->m_charPos = reinterpret_cast<Utf16String *>(data_1)->Get_Length();
            data->m_constructText->Set_Text(Utf16String::s_emptyString);
            data->m_conCharPos = 0;
            data->m_sText->Set_Text(Utf16String::s_emptyString);
            int length = reinterpret_cast<Utf16String *>(data_1)->Get_Length();

            for (int i = 0; i < length; i++) {
                data->m_sText->Add_Char(U_CHAR('*'));
            }
        }
    } else if (message == GWM_INPUT_FOCUS) {
        if (data_1 != 0) {
            if (g_theIMEManager != nullptr) {
                g_theIMEManager->Attach(text_entry);
            }

            instance_data->m_state |= 4;
            instance_data->m_state |= 2;
        } else {
            instance_data->m_state &= ~4;
            instance_data->m_state &= ~2;

            if (data->m_constructList != nullptr) {
                data->m_constructList->Win_Hide(true);
            }

            data->m_constructText->Set_Text(Utf16String::s_emptyString);
            data->m_conCharPos = 0;

            if (g_theIMEManager != nullptr && g_theIMEManager->Is_Attached_To(text_entry)) {
                g_theIMEManager->Attach(nullptr);
            }
        }

        g_theWindowManager->Win_Send_System_Msg(
            instance_data->Get_Owner(), GGM_FOCUS_CHANGE, data_1, text_entry->Win_Get_Window_Id());
        *reinterpret_cast<bool *>(data_2) = true;
    } else if (message != GWM_CREATE) {
        if (message != GWM_DESTROY) {
            return MSG_IGNORED;
        }

        g_theDisplayStringManager->Free_Display_String(data->m_text);
        g_theDisplayStringManager->Free_Display_String(data->m_sText);
        g_theDisplayStringManager->Free_Display_String(data->m_constructText);

        if (data->m_constructList != nullptr) {
            g_theWindowManager->Win_Destroy(data->m_constructList);
        }

        delete data;
    }

    return MSG_HANDLED;
}
