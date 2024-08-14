/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Window Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamewindowmanager.h"
#include "display.h"
#include "displaystringmanager.h"
#include "gadgetcheckbox.h"
#include "gadgetcombobox.h"
#include "gadgetlistbox.h"
#include "gadgetprogressbar.h"
#include "gadgetpushbutton.h"
#include "gadgetradiobutton.h"
#include "gadgetslider.h"
#include "gadgetstatictext.h"
#include "gadgettabcontrol.h"
#include "gadgettextentry.h"
#include "gamewindowtransitions.h"
#include "globallanguage.h"
#include "mouse.h"
#include "windowlayout.h"

#ifndef GAME_DLL
GameWindowManager *g_theWindowManager;
#endif

bool s_sendMousePosMessages = true;

GameWindowManager::GameWindowManager() :
    m_windowList(nullptr),
    m_windowTail(nullptr),
    m_destroyList(nullptr),
    m_currMouseRgn(nullptr),
    m_mouseCaptor(nullptr),
    m_keyboardFocus(nullptr),
    m_modalHead(nullptr),
    m_grabWindow(nullptr),
    m_loneWindow(nullptr),
    m_cursorBitmap(nullptr),
    m_captureFlags(0)
{
}

GameWindowManager::~GameWindowManager()
{
    Win_Destroy_All();
    Free_Static_Strings();

    if (g_theTransitionHandler != nullptr) {
        delete g_theTransitionHandler;
    }

    g_theTransitionHandler = nullptr;
}

void GameWindowManager::Init()
{
    if (g_theTransitionHandler == nullptr) {
        g_theTransitionHandler = new GameWindowTransitionsHandler();
    }

    g_theTransitionHandler->Load();
    g_theTransitionHandler->Init();
}

void GameWindowManager::Link_Window(GameWindow *window)
{
    GameWindow *head_window = nullptr;

    for (GameWindow *list = m_windowList; list != nullptr; list = list->m_next) {
        for (ModalWindow *modal = m_modalHead; modal != nullptr; modal = modal->m_next) {
            if (modal->m_window == list && modal->m_window != window) {
                head_window = list;
            }
        }
    }

    if (head_window != nullptr) {
        window->m_prev = head_window;
        window->m_next = head_window->m_next;
        head_window->m_next = window;

        if (window->m_next != nullptr) {
            window->m_next->m_prev = window;
        }
    } else {
        window->m_prev = nullptr;
        window->m_next = m_windowList;

        if (m_windowList != nullptr) {
            m_windowList->m_prev = window;
        } else {
            m_windowTail = window;
        }

        m_windowList = window;
    }
}

void GameWindowManager::Unlink_Window(GameWindow *window)
{
    if (window->m_next != nullptr) {
        window->m_next->m_prev = window->m_prev;
    } else {
        m_windowTail = window->m_prev;
    }

    if (window->m_prev != nullptr) {
        window->m_prev->m_next = window->m_next;
    } else {
        m_windowList = window->m_next;
    }
}

void GameWindowManager::Unlink_Child_Window(GameWindow *window)
{
    if (window->m_prev != nullptr) {
        window->m_prev->m_next = window->m_next;

        if (window->m_next != nullptr) {
            window->m_next->m_prev = window->m_prev;
        }
    } else if (window->m_next != nullptr) {
        window->m_parent->m_child = window->m_next;
        window->m_next->m_prev = window->m_prev;
        window->m_next = nullptr;
    } else {
        window->m_parent->m_child = nullptr;
    }

    window->m_parent = nullptr;
}

void GameWindowManager::Process_Destroy_List()
{
    GameWindow *list = m_destroyList;
    m_destroyList = nullptr;

    while (list != nullptr) {
        GameWindow *next = list->m_next;

        if (m_mouseCaptor == list) {
            Win_Release(list);
        }

        if (m_keyboardFocus == list) {
            Win_Set_Focus(nullptr);
        }

        if (m_modalHead != nullptr && list == m_modalHead->m_window) {
            Win_Unset_Modal(m_modalHead->m_window);
        }

        if (m_currMouseRgn == list) {
            m_currMouseRgn = nullptr;
        }

        if (m_grabWindow == list) {
            m_grabWindow = nullptr;
        }

        list = next;
    }
}

WindowMsgHandledType Pass_Selected_Buttons_To_Parent_System(
    GameWindow *window, unsigned int msg, unsigned int data_1, unsigned int data_2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    if (msg == GBM_SELECTED || msg == GBM_SELECTED_RIGHT || msg == GBM_MOUSE_ENTERING || msg == GBM_MOUSE_LEAVING
        || msg == GEM_EDIT_DONE) {
        GameWindow *parent = window->Win_Get_Parent();

        if (parent != nullptr) {
            return g_theWindowManager->Win_Send_System_Msg(parent, msg, data_1, data_2);
        }
    }

    return MSG_IGNORED;
}

WindowMsgHandledType Pass_Messages_To_Parent_System(
    GameWindow *window, unsigned int msg, unsigned int data_1, unsigned int data_2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    GameWindow *parent = window->Win_Get_Parent();

    if (parent != nullptr) {
        return g_theWindowManager->Win_Send_System_Msg(parent, msg, data_1, data_2);
    } else {
        return MSG_IGNORED;
    }
}

void GameWindowManager::Reset()
{
    Win_Destroy_All();

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Reset();
    }
}

void GameWindowManager::Update()
{
    Process_Destroy_List();

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Update();
    }
}

void GameWindowManager::Insert_Window_Ahead_Of(GameWindow *window, GameWindow *ahead_of)
{
    if (window != nullptr) {
        if (ahead_of != nullptr) {
            GameWindow *parent = ahead_of->Win_Get_Parent();

            if (parent != nullptr) {
                window->m_prev = ahead_of->m_prev;

                if (ahead_of->m_prev != nullptr) {
                    ahead_of->m_prev->m_next = window;
                } else {
                    parent->m_child = window;
                }

                ahead_of->m_prev = window;
                window->m_next = ahead_of;
                window->m_parent = parent;
            } else {
                window->m_prev = ahead_of->m_prev;

                if (ahead_of->m_prev != nullptr) {
                    ahead_of->m_prev->m_next = window;
                } else {
                    m_windowList = window;
                }

                ahead_of->m_prev = window;
                window->m_next = ahead_of;
            }
        } else {
            Link_Window(window);
        }
    }
}

bool GameWindowManager::Is_Enabled(GameWindow *window)
{
    if (window == nullptr) {
        return false;
    }

    if ((window->m_status & WIN_STATUS_ENABLED) == 0) {
        return true;
    }

    while (window->m_parent != nullptr) {
        window = window->m_parent;

        if ((window->m_status & WIN_STATUS_ENABLED) == 0) {
            return true;
        }
    }

    return false;
}

bool GameWindowManager::Is_Hidden(GameWindow *window)
{
    if (window == nullptr) {
        return true;
    }

    if ((window->m_status & WIN_STATUS_HIDDEN) != 0) {
        return true;
    }

    while (window->m_parent != nullptr) {
        window = window->m_parent;

        if ((window->m_status & WIN_STATUS_HIDDEN) != 0) {
            return true;
        }
    }

    return false;
}

void GameWindowManager::Add_Window_To_Parent(GameWindow *window, GameWindow *parent)
{
    if (parent != nullptr) {
        window->m_prev = nullptr;
        window->m_next = parent->m_child;

        if (parent->m_child != nullptr) {
            parent->m_child->m_prev = window;
        }

        parent->m_child = window;
        window->m_parent = parent;
    }
}

void GameWindowManager::Add_Window_To_Parent_At_End(GameWindow *window, GameWindow *parent)
{
    if (parent != nullptr) {
        window->m_prev = nullptr;
        window->m_next = nullptr;

        if (parent->m_child != nullptr) {
            GameWindow *child;

            for (child = parent->m_child; child->m_next != nullptr; child = child->m_next) {
                ;
            }

            child->m_next = window;
            window->m_prev = child;
        } else {
            parent->m_child = window;
        }

        window->m_parent = parent;
    }
}

void GameWindowManager::Window_Hiding(GameWindow *window)
{
    if (m_keyboardFocus == window) {
        m_keyboardFocus = nullptr;
    }

    if (m_modalHead && m_modalHead->m_window == window) {
        Win_Unset_Modal(window);
    }

    if (m_mouseCaptor == window) {
        Win_Capture(nullptr);
    }

    for (GameWindow *child = window->Win_Get_Child(); child != nullptr; child = child->Win_Get_Next()) {
        Window_Hiding(child);
    }
}

void GameWindowManager::Hide_Windows_In_Range(GameWindow *base_window, int first, int last, bool hide_flag)
{
    while (first <= last) {
        GameWindow *win = Win_Get_Window_From_Id(base_window, first);

        if (win != nullptr) {
            win->Win_Hide(hide_flag);
        }

        first++;
    }
}

void GameWindowManager::Enable_Windows_In_Range(GameWindow *base_window, int first, int last, bool enable_flag)
{
    while (first <= last) {
        GameWindow *win = Win_Get_Window_From_Id(base_window, first);

        if (win != nullptr) {
            win->Win_Enable(enable_flag);
        }

        first++;
    }
}

int GameWindowManager::Win_Capture(GameWindow *window)
{
    if (m_mouseCaptor != nullptr) {
        return WIN_ERR_MOUSE_CAPTURED;
    }

    m_mouseCaptor = window;
    return WIN_ERR_OK;
}

int GameWindowManager::Win_Release(GameWindow *window)
{
    if (window == m_mouseCaptor) {
        m_mouseCaptor = nullptr;
    }

    return WIN_ERR_OK;
}

GameWindow *GameWindowManager::Get_Win_Capture()
{
    return m_mouseCaptor;
}

GameWindow *GameWindowManager::Win_Get_Window_From_Id(GameWindow *window, int id)
{
    if (window == nullptr) {
        window = m_windowList;
    }

    while (window != nullptr) {
        if (window->Win_Get_Window_Id() == id) {
            return window;
        }

        if (window->m_child != nullptr) {
            GameWindow *child = Win_Get_Window_From_Id(window->m_child, id);

            if (child != nullptr) {
                return child;
            }
        }

        window = window->m_next;
    }

    return nullptr;
}

GameWindow *GameWindowManager::Win_Get_Window_List()
{
    return m_windowList;
}

WindowMsgHandledType GameWindowManager::Win_Send_System_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    if (msg == GWM_DESTROY || (window->m_status & WIN_STATUS_DESTROYED) == 0) {
        return window->m_system(window, msg, data1, data2);
    }

    return MSG_IGNORED;
}

WindowMsgHandledType GameWindowManager::Win_Send_Input_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2)
{
    if (window == nullptr) {
        return MSG_IGNORED;
    }

    if (msg == GWM_DESTROY || (window->m_status & WIN_STATUS_DESTROYED) == 0) {
        return window->m_input(window, msg, data1, data2);
    }

    return MSG_IGNORED;
}

GameWindow *GameWindowManager::Win_Get_Focus()
{
    return m_keyboardFocus;
}

int GameWindowManager::Win_Set_Focus(GameWindow *window)
{
    bool b2;
    bool b = false;

    if (window != nullptr && (window->Win_Get_Status() & GWS_MOUSE_TRACK) != 0) {
        return WIN_ERR_OK;
    }

    if (m_keyboardFocus != nullptr && m_keyboardFocus != window) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
        // pointer to an unsigned int on 64 bit
        Win_Send_System_Msg(m_keyboardFocus, GWM_INPUT_FOCUS, 0, reinterpret_cast<unsigned int>(&b2));
#endif
    }

    m_keyboardFocus = window;

    if (m_keyboardFocus != nullptr) {
        do {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
            // pointer to an unsigned int on 64 bit
            Win_Send_System_Msg(window, GWM_INPUT_FOCUS, 1, reinterpret_cast<unsigned int>(&b));
#endif

            if (b) {
                break;
            }

            window = window->Win_Get_Parent();
        } while (window != nullptr);
    }

    if (!b) {
        m_keyboardFocus = nullptr;
    }

    return WIN_ERR_OK;
}

WinInputReturnCode GameWindowManager::Win_Process_Key(unsigned char key, unsigned char state)
{
    WinInputReturnCode ret = WIN_INPUT_NOT_USED;

    if (m_keyboardFocus != nullptr && key != 0) {
        GameWindow *focus = m_keyboardFocus;
        ret = WIN_INPUT_USED;

        while (Win_Send_Input_Msg(focus, GWM_CHAR, key, state) == MSG_IGNORED) {
            focus = focus->Win_Get_Parent();

            if (focus == nullptr) {
                return WIN_INPUT_NOT_USED;
            }
        }
    }

    return ret;
}

WinInputReturnCode GameWindowManager::Win_Process_Mouse_Event(GameWindowMessage msg, ICoord2D *mouse_pos, void *data)
{
    WinInputReturnCode ret = WIN_INPUT_NOT_USED;
    bool b = false;
    GameWindow *interacted_window = nullptr;
    GameWindow *tooltip_window = nullptr;
    bool clear_grab_window = false;
    IRegion2D new_region;
    ICoord2D grab_size;
    unsigned int packed_mouse_coords =
        (static_cast<unsigned short>(mouse_pos->y) << 16) | static_cast<unsigned short>(mouse_pos->x);
    g_theMouse->Set_Cursor_Tooltip(Utf16String::s_emptyString, -1, nullptr, 1.0f);

    if (m_mouseCaptor != nullptr) {
        m_grabWindow = nullptr;
        interacted_window = m_mouseCaptor->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, false);

        if (s_sendMousePosMessages || msg != GWM_MOUSE_POS) {
            GameWindow *window3 = interacted_window;

            if (interacted_window != nullptr) {
                while (window3 != nullptr) {
                    if (Win_Send_Input_Msg(window3, msg, packed_mouse_coords, 0) == MSG_HANDLED) {
                        ret = WIN_INPUT_USED;
                        goto l1;
                    }

                    if (window3 == m_mouseCaptor) {
                        goto l1;
                    }

                    window3 = window3->Win_Get_Parent();
                }
            } else if (Win_Send_Input_Msg(m_mouseCaptor, msg, packed_mouse_coords, 0) == MSG_HANDLED) {
                ret = WIN_INPUT_USED;
            }
        }

        goto l1;
    }

    if (m_grabWindow != nullptr) {
        switch (msg) {
            case GWM_NONE:
                goto l2;
            case GWM_LEFT_UP:
                m_grabWindow->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, true);
                m_grabWindow->m_status &= ~WIN_STATUS_ACTIVE;

                if (m_grabWindow->Win_Point_In_Window(mouse_pos->x, mouse_pos->y)
                    || (m_grabWindow->m_status & WIN_STATUS_DRAGABLE) != 0) {
                    Win_Send_Input_Msg(m_grabWindow, GWM_LEFT_UP, packed_mouse_coords, 0);
                }

                clear_grab_window = true;
                break;
            case GWM_LEFT_DRAG:
            l2:
                if ((m_grabWindow->m_status & WIN_STATUS_DRAGABLE) != 0) {
                    ICoord2D *delta_pos = static_cast<ICoord2D *>(data);
                    int delta_x = delta_pos->x;
                    int delta_y = delta_pos->y;

                    if (m_grabWindow->Win_Get_Parent() != nullptr) {
                        GameWindow *grab_window_parent = m_grabWindow->Win_Get_Parent();

                        if (delta_x + m_grabWindow->m_region.lo.x >= 0) {
                            if (delta_x + m_grabWindow->m_region.hi.x > grab_window_parent->m_size.x) {
                                delta_x = grab_window_parent->m_size.x - m_grabWindow->m_region.hi.x;
                            }
                        } else {
                            delta_x = -m_grabWindow->m_region.lo.x;
                        }
                        if (delta_y + m_grabWindow->m_region.lo.y >= 0) {
                            if (delta_y + m_grabWindow->m_region.hi.y > grab_window_parent->m_size.y) {
                                delta_y = grab_window_parent->m_size.y - m_grabWindow->m_region.hi.y;
                            }
                        } else {
                            delta_y = -m_grabWindow->m_region.lo.y;
                        }
                    }

                    m_grabWindow->Win_Get_Position(&new_region.lo.x, &new_region.lo.y);
                    m_grabWindow->Win_Get_Size(&grab_size.x, &grab_size.y);

                    new_region.lo.x += delta_x;
                    new_region.lo.y += delta_y;

                    if (new_region.lo.x < 0) {
                        new_region.lo.x = 0;
                    }

                    if (new_region.lo.y < 0) {
                        new_region.lo.y = 0;
                    }

                    new_region.hi.x = grab_size.x + new_region.lo.x;
                    new_region.hi.y = grab_size.y + new_region.lo.y;

                    if (new_region.hi.x > static_cast<int32_t>(g_theDisplay->Get_Width())) {
                        new_region.hi.x = g_theDisplay->Get_Width();
                    }

                    if (new_region.hi.y > static_cast<int32_t>(g_theDisplay->Get_Height())) {
                        new_region.hi.y = g_theDisplay->Get_Height();
                    }

                    new_region.lo.x = new_region.hi.x - grab_size.x;
                    new_region.lo.y = new_region.hi.y - grab_size.y;
                    m_grabWindow->Win_Set_Position(new_region.hi.x - grab_size.x, new_region.hi.y - grab_size.y);
                }

                Win_Send_Input_Msg(m_grabWindow, msg, packed_mouse_coords, 0);
                break;
        }

        ret = WIN_INPUT_USED;
        goto l1;
    }

    if (m_modalHead != nullptr && m_modalHead->m_window != nullptr) {
        interacted_window = m_modalHead->m_window->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, false);
    } else {
        for (interacted_window = m_windowList; interacted_window != nullptr; interacted_window = interacted_window->m_next) {
            if ((interacted_window->m_status & WIN_STATUS_ABOVE) != 0
                && (interacted_window->m_status & WIN_STATUS_HIDDEN) == 0 && mouse_pos->x >= interacted_window->m_region.lo.x
                && mouse_pos->x <= interacted_window->m_region.hi.x && mouse_pos->y >= interacted_window->m_region.lo.y
                && mouse_pos->y <= interacted_window->m_region.hi.y) {
                GameWindow *hovered_window =
                    interacted_window->Win_Point_In_Any_Child(mouse_pos->x, mouse_pos->y, true, true);

                if (tooltip_window == nullptr
                    || (hovered_window->m_tooltip != nullptr && hovered_window->m_instData.Get_Tooltip_Text_Length() != 0)) {
                    tooltip_window = hovered_window;
                }

                if ((interacted_window->m_status & WIN_STATUS_ENABLED) != 0) {
                    interacted_window = interacted_window->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, false);
                    break;
                }
            }
        }

        if (interacted_window == nullptr) {
            for (interacted_window = m_windowList; interacted_window != nullptr;
                 interacted_window = interacted_window->m_next) {
                if ((interacted_window->m_status & (WIN_STATUS_BELOW | WIN_STATUS_ABOVE | WIN_STATUS_HIDDEN)) == 0
                    && mouse_pos->x >= interacted_window->m_region.lo.x && mouse_pos->x <= interacted_window->m_region.hi.x
                    && mouse_pos->y >= interacted_window->m_region.lo.y
                    && mouse_pos->y <= interacted_window->m_region.hi.y) {
                    GameWindow *hovered_window =
                        interacted_window->Win_Point_In_Any_Child(mouse_pos->x, mouse_pos->y, true, true);

                    if (tooltip_window == nullptr
                        && (hovered_window->m_tooltip != nullptr
                            || hovered_window->m_instData.Get_Tooltip_Text_Length() != 0)) {
                        tooltip_window = hovered_window;
                    }

                    if ((interacted_window->m_status & WIN_STATUS_ENABLED) != 0) {
                        interacted_window = interacted_window->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, false);
                        break;
                    }
                }
            }
        }

        if (interacted_window == nullptr) {
            for (interacted_window = m_windowList; interacted_window != nullptr;
                 interacted_window = interacted_window->m_next) {
                if ((interacted_window->m_status & WIN_STATUS_BELOW) != 0
                    && (interacted_window->m_status & WIN_STATUS_HIDDEN) == 0
                    && mouse_pos->x >= interacted_window->m_region.lo.x && mouse_pos->x <= interacted_window->m_region.hi.x
                    && mouse_pos->y >= interacted_window->m_region.lo.y
                    && mouse_pos->y <= interacted_window->m_region.hi.y) {
                    GameWindow *hovered_window =
                        interacted_window->Win_Point_In_Any_Child(mouse_pos->x, mouse_pos->y, true, true);

                    if (tooltip_window == nullptr
                        && (hovered_window->m_tooltip != nullptr
                            || hovered_window->m_instData.Get_Tooltip_Text_Length() != 0)) {
                        tooltip_window = hovered_window;
                    }

                    if ((interacted_window->m_status & WIN_STATUS_ENABLED) != 0) {
                        interacted_window = interacted_window->Win_Point_In_Child(mouse_pos->x, mouse_pos->y, false, false);
                        break;
                    }
                }
            }
        }
    }

    if (interacted_window != nullptr && (interacted_window->m_status & WIN_STATUS_NO_INPUT) != 0) {
        if (interacted_window->Win_Get_Parent() != nullptr
            && (interacted_window->Win_Get_Parent()->Win_Get_Instance_Data()->Get_Style() & GWS_COMBO_BOX) != 0) {
            interacted_window = interacted_window->Win_Get_Parent();
        } else {
            interacted_window = nullptr;
        }
    }

    if (interacted_window != nullptr && (s_sendMousePosMessages || msg != GWM_MOUSE_POS)) {
        GameWindow *window6 = interacted_window;
        GameWindow *window7 = m_loneWindow;

        do {
            if (Win_Send_Input_Msg(window6, msg, packed_mouse_coords, 0) != MSG_IGNORED) {
                break;
            }

            window6 = window6->m_parent;
        } while (window6 != nullptr);

        if (m_loneWindow != nullptr && m_loneWindow == window7
            && (msg == GWM_LEFT_UP || msg == GWM_MIDDLE_UP || msg == GWM_RIGHT_UP || window6 != nullptr)
            && !m_loneWindow->Win_Is_Child(window6)) {
            Win_Set_Lone_Window(nullptr);
        }

        if (window6 != nullptr) {
            if (msg == GWM_LEFT_DOWN) {
                m_grabWindow = window6;
            }

            ret = WIN_INPUT_USED;
        }
    }

    if (tooltip_window == nullptr && !Is_Hidden(interacted_window)) {
        tooltip_window = interacted_window;
    }

    if (tooltip_window != nullptr) {
        if (tooltip_window->m_tooltip != nullptr) {
            tooltip_window->m_tooltip(tooltip_window, &tooltip_window->m_instData, packed_mouse_coords);
        } else if (tooltip_window->m_instData.Get_Tooltip_Text_Length() != 0) {
            g_theMouse->Set_Cursor_Tooltip(
                tooltip_window->m_instData.Get_Tooltip_Text(), tooltip_window->m_instData.m_tooltipDelay, 0, 1.0f);
        }
    } else {
        b = true;
    }
l1:
    if (m_grabWindow == nullptr && interacted_window != m_currMouseRgn) {
        if (m_mouseCaptor != nullptr) {
            if (m_mouseCaptor->Win_Is_Child(m_currMouseRgn)) {
                Win_Send_Input_Msg(m_currMouseRgn, GWM_MOUSE_LEAVING, packed_mouse_coords, 0);
            }
        } else if (m_currMouseRgn != nullptr) {
            Win_Send_Input_Msg(m_currMouseRgn, GWM_MOUSE_LEAVING, packed_mouse_coords, 0);
        }

        if (interacted_window != nullptr) {
            Win_Send_Input_Msg(interacted_window, GWM_MOUSE_ENTERING, packed_mouse_coords, 0);
        }

        m_currMouseRgn = interacted_window;
    }

    if (clear_grab_window) {
        m_grabWindow = nullptr;
        clear_grab_window = false;
    }

    return ret;
}

int GameWindowManager::Draw_Window(GameWindow *window)
{
    if (window == nullptr) {
        return WIN_ERR_INVALID_WINDOW;
    }

    if ((window->m_status & WIN_STATUS_HIDDEN) == 0) {
        if ((window->m_status & WIN_STATUS_SEE_THRU) == 0 && window->m_draw != nullptr) {
            window->m_draw(window, &window->m_instData);
        }

        if ((window->Win_Get_Style() & GWS_SCROLL_LISTBOX) != 0 && (window->m_status & WIN_STATUS_BORDER) != 0
            && (window->m_status & WIN_STATUS_SEE_THRU) == 0) {
            window->Win_Draw_Border();
        }

        GameWindow *child;

        for (child = window->m_child; child != nullptr && child->m_next != nullptr; child = child->m_next) {
            ;
        }

        while (child != nullptr) {
            Draw_Window(child);
            child = child->m_prev;
        }

        if ((window->Win_Get_Style() & GWS_SCROLL_LISTBOX) == 0 && (window->m_status & WIN_STATUS_BORDER) != 0
            && (window->m_status & WIN_STATUS_SEE_THRU) == 0) {
            window->Win_Draw_Border();
        }
    }

    return WIN_ERR_OK;
}

void GameWindowManager::Win_Repaint()
{
    GameWindow *prev;

    for (GameWindow *tail = m_windowTail; tail != nullptr; tail = prev) {
        prev = tail->m_prev;

        if ((tail->m_status & WIN_STATUS_BELOW) != 0) {
            Draw_Window(tail);
        }
    }

    for (GameWindow *tail = m_windowTail; tail != nullptr; tail = prev) {
        prev = tail->m_prev;

        if ((tail->m_status & (WIN_STATUS_BELOW | WIN_STATUS_ABOVE)) == 0) {
            Draw_Window(tail);
        }
    }

    for (GameWindow *tail = m_windowTail; tail != nullptr; tail = prev) {
        prev = tail->m_prev;

        if ((tail->m_status & WIN_STATUS_ABOVE) != 0) {
            Draw_Window(tail);
        }
    }

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Draw();
    }
}

void GameWindowManager::Dump_Window(GameWindow *window)
{
    if (window != nullptr) {
        captainslog_debug(
            "ID: %d\tRedraw: 0x%08X\tUser Data: %d", window->Win_Get_Window_Id(), window->m_draw, window->m_userData);
    }
}

GameWindow *GameWindowManager::Win_Create(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WindowCallbackFunc system,
    WinInstanceData *inst_data)
{
    GameWindow *window = Allocate_New_Window();

    if (window != nullptr) {
        if (parent != nullptr) {
            Add_Window_To_Parent(window, parent);
        } else {
            Link_Window(window);
        }

        window->m_status = status;
        window->m_size.x = width;
        window->m_size.y = height;
        window->m_region.lo.x = x;
        window->m_region.lo.y = y;
        window->m_region.hi.x = width + x;
        window->m_region.hi.y = height + y;
        window->Normalize_Window_Region();
        window->Win_Set_System_Func(system);
        Win_Send_System_Msg(window, GWM_CREATE, 0, 0);

        if (inst_data != nullptr) {
            window->Win_Set_Instance_Data(inst_data);
        }

        if (g_theGlobalLanguage != nullptr && g_theGlobalLanguage->Default_Window_Font().Name().Is_Not_Empty()) {
            window->Win_Set_Font(Win_Find_Font(g_theGlobalLanguage->Default_Window_Font().Name(),
                g_theGlobalLanguage->Default_Window_Font().Point_Size(),
                g_theGlobalLanguage->Default_Window_Font().Bold()));
        } else {
            window->Win_Set_Font(Win_Find_Font("Times New Roman", 14, false));
        }

        return window;
    } else {
        captainslog_debug("Win_Create error: Could not allocate new window");

        for (GameWindow *win = m_windowList; win != nullptr; win = win->m_next) {
            Dump_Window(win);
        }

        return nullptr;
    }
}

int GameWindowManager::Win_Destroy(GameWindow *window)
{
    if (window == nullptr) {
        return WIN_ERR_INVALID_WINDOW;
    }

    captainslog_dbgassert(window->Win_Get_Edit_Data() == nullptr, "Win_Destroy(): edit data should NOT be present!");

    if ((window->m_status & WIN_STATUS_DESTROYED) != 0) {
        return WIN_ERR_OK;
    }

    window->m_status |= WIN_STATUS_DESTROYED;

    if (m_mouseCaptor == window) {
        Win_Release(window);
    }

    if (m_keyboardFocus == window) {
        Win_Set_Focus(nullptr);
    }

    if (m_modalHead != nullptr && window == m_modalHead->m_window) {
        Win_Unset_Modal(m_modalHead->m_window);
    }

    if (m_currMouseRgn == window) {
        m_currMouseRgn = nullptr;
    }

    if (m_grabWindow == window) {
        m_grabWindow = nullptr;
    }

    GameWindow *next;
    for (GameWindow *child = window->m_child; child != nullptr; child = next) {
        next = child->m_next;
        Win_Destroy(child);
    }

    if (window->m_parent) {
        Unlink_Child_Window(window);
    } else {
        Unlink_Window(window);
    }

    window->m_prev = nullptr;
    window->m_next = m_destroyList;
    m_destroyList = window;

    if (window->m_layout != nullptr) {
        window->m_layout->Remove_Window(window);
    }

    return WIN_ERR_OK;
}

int GameWindowManager::Win_Destroy_All()
{
    GameWindow *window = m_windowList;

    for (GameWindow *win = window; win != nullptr; window = win) {
        win = window->m_next;
        Win_Destroy(window);
    }

    Process_Destroy_List();
    return WIN_ERR_OK;
}

int GameWindowManager::Win_Set_Modal(GameWindow *window)
{
    if (window == nullptr) {
        return WIN_ERR_INVALID_WINDOW;
    }

    if (window->m_parent != nullptr) {
        captainslog_debug("Win_Set_Modal: Non Root window attempted to go modal.");
        return WIN_ERR_INVALID_PARAMETER;
    } else {
        ModalWindow *modal = new ModalWindow();

        if (modal != nullptr) {
            modal->m_window = window;
            modal->m_next = m_modalHead;
            m_modalHead = modal;
            return WIN_ERR_OK;
        } else {
            captainslog_debug("Win_Set_Modal: Unable to allocate space for Modal Entry.");
            return WIN_ERR_GENERAL_FAILURE;
        }
    }
}

int GameWindowManager::Win_Unset_Modal(GameWindow *window)
{
    if (window == nullptr) {
        return WIN_ERR_INVALID_WINDOW;
    }

    if (m_modalHead != nullptr && m_modalHead->m_window == window) {
        ModalWindow *next = m_modalHead->m_next;
        m_modalHead->Delete_Instance();
        m_modalHead = next;
        return WIN_ERR_OK;
    } else {
        captainslog_debug("Win_Unset_Modal: Invalid window attempting to unset modal (%d)", window->Win_Get_Window_Id());
        return WIN_ERR_GENERAL_FAILURE;
    }
}

GameWindow *GameWindowManager::Win_Get_Grab_Window()
{
    return m_grabWindow;
}

void GameWindowManager::Win_Set_Grab_Window(GameWindow *window)
{
    m_grabWindow = window;
}

void GameWindowManager::Win_Set_Lone_Window(GameWindow *window)
{
    if (m_loneWindow != window) {
        if (m_loneWindow != nullptr) {
            g_theWindowManager->Win_Send_System_Msg(m_loneWindow, GGM_CLOSE, 0, 0);
        }

        m_loneWindow = window;
    }
}

GameWindow *GameWindowManager::Go_Go_Message_Box(int x,
    int y,
    int width,
    int height,
    unsigned short flags,
    Utf16String title,
    Utf16String body,
    WindowMsgBoxCallbackFunc yes_callback,
    WindowMsgBoxCallbackFunc no_callback,
    WindowMsgBoxCallbackFunc ok_callback,
    WindowMsgBoxCallbackFunc cancel_callback)
{
    return Go_Go_Message_Box(
        x, y, width, height, flags, title, body, yes_callback, no_callback, ok_callback, cancel_callback, false);
}

GameWindow *GameWindowManager::Go_Go_Message_Box(int x,
    int y,
    int width,
    int height,
    unsigned short flags,
    Utf16String title,
    Utf16String body,
    WindowMsgBoxCallbackFunc yes_callback,
    WindowMsgBoxCallbackFunc no_callback,
    WindowMsgBoxCallbackFunc ok_callback,
    WindowMsgBoxCallbackFunc cancel_callback,
    bool use_quit_box)
{
    if (flags != 0) {
        GameWindow *message_box = nullptr;

        if (use_quit_box) {
            message_box = Win_Create_From_Script("Menus/QuitMessageBox.wnd", nullptr);
        } else {
            message_box = Win_Create_From_Script("Menus/MessageBox.wnd", nullptr);
        }

        Utf8String str;

        if (use_quit_box) {
            str.Set("QuitMessageBox.wnd:");
        } else {
            str.Set("MessageBox.wnd:");
        }

        GameWindow *parent = nullptr;
        Utf8String window_name = str;
        window_name.Concat("MessageBoxParent");
        parent = Win_Get_Window_From_Id(message_box, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        Win_Set_Modal(message_box);
        Win_Set_Focus(nullptr);
        Win_Set_Focus(parent);

        if (width > 0 && height > 0) {
            int window_width;
            int window_height;
            parent->Win_Get_Size(&window_width, &window_height);
            float x_rescale_ratio = (float)width / (float)window_width;
            float y_rescale_ratio = (float)height / (float)window_height;
            parent->Win_Set_Size(width, height);

            for (GameWindow *child = parent->Win_Get_Child(); child != nullptr; child = child->Win_Get_Next()) {
                child->Win_Get_Size(&window_width, &window_height);
                window_width *= x_rescale_ratio;
                window_height *= y_rescale_ratio;
                child->Win_Set_Size(window_width, window_height);

                child->Win_Get_Position(&window_width, &window_height);
                window_width *= x_rescale_ratio;
                window_height *= y_rescale_ratio;
                child->Win_Set_Position(window_width, window_height);
            }
        }

        if (x >= 0 && y >= 0) {
            parent->Win_Set_Position(x, y);
        }

        window_name = str;
        window_name.Concat("ButtonOk");
        GameWindow *button_ok = Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        int ok_x;
        int ok_y;
        button_ok->Win_Get_Position(&ok_x, &ok_y);

        window_name = str;
        window_name.Concat("ButtonYes");
        GameWindow *button_yes = Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));

        window_name = str;
        window_name.Concat("ButtonNo");
        GameWindow *button_no = Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        int no_x;
        int no_y;
        button_no->Win_Get_Position(&no_x, &no_y);

        window_name = str;
        window_name.Concat("ButtonCancel");
        GameWindow *button_cancel = Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        int cancel_x;
        int cancel_y;
        button_cancel->Win_Get_Position(&cancel_x, &cancel_y);

        captainslog_dbgassert((flags & (MSG_BOX_OK | MSG_BOX_YES)) != (MSG_BOX_OK | MSG_BOX_YES),
            "Passed in MSG_BOX_OK and MSG_BOX_YES.  Big No No.");

        if ((flags & MSG_BOX_OK) == MSG_BOX_OK) {
            button_ok->Win_Set_Position(ok_x, ok_y);
            button_ok->Win_Hide(false);
        }

        if ((flags & MSG_BOX_YES) == MSG_BOX_YES) {
            button_yes->Win_Set_Position(ok_x, ok_y);
            button_yes->Win_Hide(false);
        }

        if ((flags & (MSG_BOX_CANCEL | MSG_BOX_NO)) == (MSG_BOX_CANCEL | MSG_BOX_NO)) {
            button_no->Win_Set_Position(no_x, no_y);
            button_cancel->Win_Set_Position(cancel_x, cancel_y);
            button_no->Win_Hide(false);
            button_cancel->Win_Hide(false);
        } else if ((flags & MSG_BOX_NO) == MSG_BOX_NO) {
            button_no->Win_Set_Position(cancel_x, cancel_y);
            button_no->Win_Hide(false);
        } else if ((flags & MSG_BOX_CANCEL) == MSG_BOX_CANCEL) {
            button_cancel->Win_Set_Position(cancel_x, cancel_y);
            button_cancel->Win_Hide(false);
        }

        window_name = str;
        window_name.Concat("StaticTextTitle");
        GameWindow *static_text_title =
            Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        Gadget_Static_Text_Set_Text(static_text_title, title);

        window_name = str;
        window_name.Concat("StaticTextMessage");
        GameWindow *static_text_message =
            Win_Get_Window_From_Id(parent, g_theNameKeyGenerator->Name_To_Key(window_name.Str()));
        Gadget_Static_Text_Set_Text(static_text_message, body);

        _MessageBoxData *mb_data = new _MessageBoxData;
        mb_data->cancel_callback = cancel_callback;
        mb_data->no_callback = no_callback;
        mb_data->ok_callback = ok_callback;
        mb_data->yes_callback = yes_callback;
        message_box->Win_Set_User_Data(mb_data);
        parent->Win_Hide(false);
        parent->Win_Bring_To_Top();
        return message_box;
    } else {
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Push_Button(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & GWS_PUSH_BUTTON) != 0) {
        GameWindow *button = Win_Create(parent, status, x, y, width, height, Gadget_Push_Button_System, inst_data);

        if (button != nullptr) {
            button->Win_Set_Input_Func(Gadget_Push_Button_Input);
            WindowDrawFunc draw;

            if ((button->Win_Get_Status() & WIN_STATUS_IMAGE)) {
                draw = Get_Push_Button_Image_Draw_Func();
            } else {
                draw = Get_Push_Button_Draw_Func();
            }

            button->Win_Set_Draw_Func(draw);
            button->Win_Set_Owner(parent);
            button->Win_Set_User_Data(nullptr);
            Assign_Default_Gadget_Look(button, font, assign_visuals);
            Utf16String str = Win_Text_Label_To_Text(inst_data->m_textLabelString);

            if (str.Get_Length() != 0) {
                Gadget_Button_Set_Text(button, str);
            }

            return button;
        } else {
            captainslog_debug("Unable to create button for push button gadget");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create button gadget, instance data not button type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Checkbox(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & GWS_CHECK_BOX) != 0) {
        GameWindow *check_box =
            g_theWindowManager->Win_Create(parent, status, x, y, width, height, Gadget_Check_Box_System, inst_data);

        if (check_box != nullptr) {
            check_box->Win_Set_Input_Func(Gadget_Check_Box_Input);
            WindowDrawFunc draw;

            if ((check_box->Win_Get_Status() & WIN_STATUS_IMAGE)) {
                draw = Get_Check_Box_Image_Draw_Func();
            } else {
                draw = Get_Check_Box_Draw_Func();
            }

            check_box->Win_Set_Draw_Func(draw);
            check_box->Win_Set_Owner(parent);
            Assign_Default_Gadget_Look(check_box, font, assign_visuals);
            Utf16String str = Win_Text_Label_To_Text(inst_data->m_textLabelString);

            if (str.Get_Length() != 0) {
                Gadget_Check_Box_Set_Text(check_box, str);
            }

            return check_box;
        } else {
            captainslog_debug("Unable to create checkbox window");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create checkbox gadget, instance data not checkbox type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Radio_Button(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _RadioButtonData *data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & GWS_RADIO_BUTTON) != 0) {
        GameWindow *radio_button =
            g_theWindowManager->Win_Create(parent, status, x, y, width, height, Gadget_Radio_Button_System, inst_data);

        if (radio_button != nullptr) {
            _RadioButtonData *rb_data = new _RadioButtonData;
            memcpy(rb_data, data, sizeof(_RadioButtonData));
            radio_button->Win_Set_User_Data(rb_data);
            radio_button->Win_Set_Input_Func(Gadget_Radio_Button_Input);
            WindowDrawFunc draw;

            if ((radio_button->Win_Get_Status() & WIN_STATUS_IMAGE)) {
                draw = Get_Radio_Button_Image_Draw_Func();
            } else {
                draw = Get_Radio_Button_Draw_Func();
            }

            radio_button->Win_Set_Draw_Func(draw);
            radio_button->Win_Set_Owner(parent);
            Assign_Default_Gadget_Look(radio_button, font, assign_visuals);
            Utf16String str = Win_Text_Label_To_Text(inst_data->m_textLabelString);

            if (str.Get_Length() != 0) {
                Gadget_Radio_Set_Text(radio_button, str);
            }

            return radio_button;
        } else {
            captainslog_debug("Unable to create radio button window");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create radioButton gadget, instance data not radioButton type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_List_Box(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _ListboxData *data,
    GameFont *font,
    bool assign_visuals)
{
    bool has_text = false;

    if ((inst_data->Get_Style() & GWS_SCROLL_LISTBOX) == 0) {
        captainslog_debug("Can't create listbox gadget, instance data not listbox type");
        return nullptr;
    }

    GameWindow *list_box = Win_Create(parent, status, x, y, width, height, Gadget_List_Box_System, inst_data);

    if (list_box == nullptr) {
        captainslog_debug("Unable to create listbox window");
        return nullptr;
    }

    _ListboxData *lb_data = new _ListboxData;
    captainslog_dbgassert(data != nullptr, "listboxDataTemplate not initialized");
    memcpy(lb_data, data, sizeof(_ListboxData));
    list_box->Win_Set_User_Data(lb_data);
    list_box->Win_Set_Owner(parent);

    if (inst_data->Get_Text_Length() != 0) {
        has_text = true;
    }

    WindowDrawFunc draw;

    if ((list_box->Win_Get_Status() & WIN_STATUS_IMAGE)) {
        draw = Get_List_Box_Image_Draw_Func();
    } else {
        draw = Get_List_Box_Draw_Func();
    }

    list_box->Win_Set_Draw_Func(draw);

    if (lb_data->m_multiSelect) {
        list_box->Win_Set_Input_Func(Gadget_List_Box_Multi_Input);
    } else {
        list_box->Win_Set_Input_Func(Gadget_List_Box_Input);
    }

    int length = lb_data->m_listLength;
    lb_data->m_listLength = 0;
    Gadget_List_Box_Set_List_Length(list_box, length);
    lb_data->m_displayHeight = height;

    if (has_text) {
        lb_data->m_displayHeight -= Win_Font_Height(inst_data->Get_Font());
    }

    lb_data->m_displayPos = 0;
    lb_data->m_selectPos = -1;
    lb_data->m_doubleClickTime = 0;
    lb_data->m_insertPos = 0;
    lb_data->m_endPos = 0;
    lb_data->m_totalHeight = 0;

    if (lb_data->m_scrollBar) {
        Gadget_List_Box_Create_Scroll_Bar(list_box);
    }

    if (lb_data->m_columns == 1) {
        lb_data->m_columnWidth = new int;
        *lb_data->m_columnWidth = width;

        if (lb_data->m_slider != nullptr) {
            int slider_width;
            int slider_height;
            lb_data->m_slider->Win_Get_Size(&slider_width, &slider_height);
            *lb_data->m_columnWidth -= slider_width + 2;
        }
    } else {
        if (lb_data->m_columnWidthPercentage == nullptr) {
            return nullptr;
        }

        lb_data->m_columnWidth = new int[lb_data->m_columns];

        if (lb_data->m_columnWidth == nullptr) {
            return nullptr;
        }

        int width_multiplier = width;

        if (lb_data->m_slider != nullptr) {
            int slider_width;
            int slider_height;
            lb_data->m_slider->Win_Get_Size(&slider_width, &slider_height);
            width_multiplier -= slider_width + 2;
        }

        for (int i = 0; i < lb_data->m_columns; i++) {
            lb_data->m_columnWidth[i] = width_multiplier * lb_data->m_columnWidthPercentage[i] / 100;
        }
    }

    Assign_Default_Gadget_Look(list_box, font, assign_visuals);
    return list_box;
}

GameWindow *GameWindowManager::Go_Go_Gadget_Tab_Control(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _TabControlData *data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & GWS_TAB_CONTROL) != 0) {
        GameWindow *tab_control =
            g_theWindowManager->Win_Create(parent, status, x, y, width, height, Gadget_Tab_Control_System, inst_data);

        if (tab_control != nullptr) {
            _TabControlData *tc_data = new _TabControlData;
            memcpy(tc_data, data, sizeof(_TabControlData));
            tab_control->Win_Set_User_Data(tc_data);
            Gadget_Tab_Control_Compute_Tab_Region(tab_control);
            Gadget_Tab_Control_Create_Sub_Panes(tab_control);
            Gadget_Tab_Control_Show_Sub_Pane(tab_control, 0);
            tab_control->Win_Set_Input_Func(Gadget_Tab_Control_Input);
            WindowDrawFunc draw;

            if ((tab_control->Win_Get_Status() & WIN_STATUS_IMAGE)) {
                draw = Get_Tab_Control_Image_Draw_Func();
            } else {
                draw = Get_Tab_Control_Draw_Func();
            }

            tab_control->Win_Set_Draw_Func(draw);
            tab_control->Win_Set_Owner(parent);
            Assign_Default_Gadget_Look(tab_control, font, assign_visuals);
            return tab_control;
        } else {
            captainslog_debug("Unable to create tab control window");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create tabControl gadget, instance data not tabControl type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Slider(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _SliderData *data,
    GameFont *font,
    bool assign_visuals)
{
    status |= WIN_STATUS_TAB_STOP;
    GameWindow *slider;

    if ((inst_data->Get_Style() & GWS_HORZ_SLIDER) != 0) {
        slider = Win_Create(parent, status, x, y, width, height, Gadget_Horizontal_Slider_System, inst_data);
        slider->Win_Set_Input_Func(Gadget_Horizontal_Slider_Input);
        WindowDrawFunc draw;

        if ((slider->Win_Get_Status() & WIN_STATUS_IMAGE)) {
            draw = Get_Horizontal_Slider_Image_Draw_Func();
        } else {
            draw = Get_Horizontal_Slider_Draw_Func();
        }

        slider->Win_Set_Draw_Func(draw);
    } else {
        if ((inst_data->Get_Style() & GWS_VERT_SLIDER) == 0) {
            captainslog_debug("Go_Go_Gadget_Slider warning: unrecognized slider style.");
            return nullptr;
        }

        slider = Win_Create(parent, status, x, y, width, height, Gadget_Vertical_Slider_System, inst_data);
        slider->Win_Set_Input_Func(Gadget_Vertical_Slider_Input);
        WindowDrawFunc draw;

        if ((slider->Win_Get_Status() & WIN_STATUS_IMAGE) == 0
            || parent && (parent->Win_Get_Style() & GWS_SCROLL_LISTBOX) != 0) {
            draw = Get_Horizontal_Slider_Draw_Func();
        } else {
            draw = Get_Horizontal_Slider_Image_Draw_Func();
        }

        slider->Win_Set_Draw_Func(draw);
    }

    if (slider != nullptr) {
        slider->Win_Set_Owner(parent);
        WinInstanceData instance_data;
        instance_data.Init();
        status = status & ~(WIN_STATUS_HIDDEN | WIN_STATUS_ENABLED | WIN_STATUS_DRAGABLE)
            | (WIN_STATUS_ENABLED | WIN_STATUS_DRAGABLE);
        instance_data.m_owner = slider;
        instance_data.m_style = GWS_PUSH_BUTTON;

        if ((inst_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
            instance_data.m_style |= GWS_MOUSE_TRACK;
        }

        if ((inst_data->Get_Style() & GWS_HORZ_SLIDER) != 0) {
            Go_Go_Gadget_Push_Button(slider, status, 0, 10, 13, 16, &instance_data, 0, true);
        } else {
            Go_Go_Gadget_Push_Button(slider, status, 0, 0, width, width + 1, &instance_data, 0, true);
        }

        if (data->m_maxVal == data->m_minVal) {
            data->m_maxVal = data->m_minVal + 1;
        }

        if ((inst_data->Get_Style() & GWS_HORZ_SLIDER) != 0) {
            data->m_numTicks = (float)(width - 13) / (float)(data->m_maxVal - data->m_minVal);
        } else {
            data->m_numTicks = (float)(height - 16) / (float)(data->m_maxVal - data->m_minVal);
        }

        _SliderData *sl_data = new _SliderData;
        memcpy(sl_data, data, sizeof(_SliderData));
        slider->Win_Set_User_Data(sl_data);
        Assign_Default_Gadget_Look(slider, font, assign_visuals);
        return slider;
    } else {
        captainslog_debug("Unable to create slider control window");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Combo_Box(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _ComboBoxData *data,
    GameFont *font,
    bool assign_visuals)
{
    bool has_text = false;

    if ((inst_data->Get_Style() & GWS_COMBO_BOX) != 0) {
        GameWindow *combo_box = Win_Create(parent, status, x, y, width, height, Gadget_Combo_Box_System, inst_data);

        if (combo_box != nullptr) {
            _ComboBoxData *cb_data = new _ComboBoxData;
            memcpy(cb_data, data, sizeof(_ComboBoxData));
            combo_box->Win_Set_User_Data(cb_data);
            combo_box->Win_Set_Owner(parent);

            if (inst_data->Get_Text_Length() != 0) {
                has_text = true;
            }

            WindowDrawFunc draw;

            if ((combo_box->Win_Get_Status() & WIN_STATUS_IMAGE)) {
                draw = Get_Combo_Box_Image_Draw_Func();
            } else {
                draw = Get_Combo_Box_Draw_Func();
            }

            combo_box->Win_Set_Draw_Func(draw);
            combo_box->Win_Set_Input_Func(Gadget_Combo_Box_Input);
            WinInstanceData instance_data;

            if (combo_box->Win_Get_Text_Length() != 0) {
                has_text = true;
            }

            status &= ~(WIN_STATUS_BORDER | WIN_STATUS_HIDDEN);
            int font_height = g_theWindowManager->Win_Font_Height(combo_box->Win_Get_Font());

            // Leftover code from the original and yes its meant to be unused.
            int i1;
            int i2;

            if (has_text) {
                i1 = font_height + 1;
            } else {
                i1 = 0;
            }

            if (has_text) {
                i2 = height - (font_height + 1);
            } else {
                i2 = height;
            }

            instance_data.Init();
            int i3 = 21;
            int i4 = 22;
            instance_data.m_owner = combo_box;
            instance_data.m_style = GWS_PUSH_BUTTON;

            if ((combo_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data.m_style |= GWS_MOUSE_TRACK;
            }

            cb_data->m_dropDownButton = g_theWindowManager->Go_Go_Gadget_Push_Button(combo_box,
                status | WIN_STATUS_ENABLED | WIN_STATUS_ACTIVE,
                width - i3,
                0,
                i3,
                height,
                &instance_data,
                0,
                true);
            cb_data->m_dropDownButton->Win_Set_Tooltip_Func(combo_box->Win_Get_Tooltip_Func());
            cb_data->m_dropDownButton->Win_Set_Tooltip(inst_data->Get_Tooltip_Text());
            cb_data->m_dropDownButton->Set_tooltip_Delay(combo_box->Get_Tooltip_Delay());
            instance_data.Init();
            instance_data.m_owner = combo_box;
            instance_data.m_style |= GWS_ENTRY_FIELD;
            instance_data.m_textLabelString = "Entry";

            if ((combo_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data.m_style |= GWS_MOUSE_TRACK;
            }

            unsigned int new_status;

            if (cb_data->m_isEditable) {
                new_status = status;
            } else {
                new_status = status | WIN_STATUS_NO_INPUT;
                cb_data->m_entryData->m_noInput = true;
            }

            GameWindow *text_entry = g_theWindowManager->Go_Go_Gadget_Text_Entry(combo_box,
                new_status,
                0,
                0,
                width - i3,
                height,
                &instance_data,
                cb_data->m_entryData,
                instance_data.m_font,
                0);

            cb_data->m_editBox = text_entry;
            cb_data->m_editBox->Win_Set_Tooltip_Func(combo_box->Win_Get_Tooltip_Func());
            cb_data->m_editBox->Win_Set_Tooltip(inst_data->Get_Tooltip_Text());
            cb_data->m_editBox->Set_tooltip_Delay(combo_box->Get_Tooltip_Delay());
            delete cb_data->m_entryData;
            cb_data->m_entryData = static_cast<_EntryData *>(cb_data->m_editBox->Win_Get_User_Data());
            instance_data.Init();
            instance_data.m_owner = combo_box;

            if ((combo_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data.m_style |= GWS_MOUSE_TRACK;
            }

            instance_data.m_style |= GWS_HORZ_SLIDER;
            instance_data.m_style |= GWS_SCROLL_LISTBOX;

            GameWindow *list_box = g_theWindowManager->Go_Go_Gadget_List_Box(combo_box,
                status & ~(WIN_STATUS_ONE_LINE | WIN_STATUS_IMAGE | WIN_STATUS_ABOVE) | WIN_STATUS_ONE_LINE
                    | WIN_STATUS_ABOVE,
                0,
                height,
                width,
                height,
                &instance_data,
                cb_data->m_listboxData,
                instance_data.m_font,
                0);

            cb_data->m_listBox = list_box;
            cb_data->m_listBox->Win_Hide(true);
            delete cb_data->m_listboxData;
            cb_data->m_listboxData = static_cast<_ListboxData *>(cb_data->m_listBox->Win_Get_User_Data());
            cb_data->m_listBox->Win_Set_Tooltip_Func(combo_box->Win_Get_Tooltip_Func());
            cb_data->m_listBox->Win_Set_Tooltip(inst_data->Get_Tooltip_Text());
            cb_data->m_listBox->Set_tooltip_Delay(combo_box->Get_Tooltip_Delay());
            Gadget_List_Box_Set_Audio_Feedback(cb_data->m_listBox, true);
            Gadget_Combo_Box_Set_Is_Editable(combo_box, cb_data->m_isEditable);
            Gadget_Combo_Box_Set_Max_Chars(combo_box, cb_data->m_maxChars);
            Gadget_Combo_Box_Set_Max_Display(combo_box, cb_data->m_maxDisplay);
            int text_color = combo_box->Win_Get_Enabled_Text_Color();
            int text_border_color = combo_box->Win_Get_Enabled_Text_Border_Color();

            if (cb_data->m_listBox) {
                cb_data->m_listBox->Win_Set_Enabled_Text_Colors(text_color, text_border_color);
            }

            if (cb_data->m_editBox) {
                cb_data->m_editBox->Win_Set_Enabled_Text_Colors(text_color, text_border_color);
            }

            text_color = combo_box->Win_Get_Disabled_Text_Color();
            text_border_color = combo_box->Win_Get_Disabled_Text_Border_Color();

            if (cb_data->m_listBox) {
                cb_data->m_listBox->Win_Set_Disabled_Text_Colors(text_color, text_border_color);
            }

            if (cb_data->m_editBox) {
                cb_data->m_editBox->Win_Set_Disabled_Text_Colors(text_color, text_border_color);
            }

            text_color = combo_box->Win_Get_Hilite_Text_Color();
            text_border_color = combo_box->Win_Get_Hilite_Text_Border_Color();

            if (cb_data->m_listBox) {
                cb_data->m_listBox->Win_Set_Hilite_Text_Colors(text_color, text_border_color);
            }

            if (cb_data->m_editBox) {
                cb_data->m_editBox->Win_Set_Hilite_Text_Colors(text_color, text_border_color);
            }

            cb_data->m_dontHide = false;
            Assign_Default_Gadget_Look(combo_box, font, assign_visuals);
            return combo_box;
        } else {
            captainslog_debug("Unable to create ComboBox window");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create ComboBox gadget, instance data not ComboBox type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Progress_Bar(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & GWS_PROGRESS_BAR) != 0) {
        GameWindow *progress_bar =
            g_theWindowManager->Win_Create(parent, status, x, y, width, height, Gadget_Progress_Bar_System, inst_data);

        if (progress_bar != nullptr) {
            WindowDrawFunc draw;

            if ((progress_bar->Win_Get_Status() & WIN_STATUS_IMAGE) != 0) {
                draw = Get_Progress_Bar_Image_Draw_Func();
            } else {
                draw = Get_Progress_Bar_Draw_Func();
            }

            progress_bar->Win_Set_Draw_Func(draw);
            progress_bar->Win_Set_Owner(parent);
            Assign_Default_Gadget_Look(progress_bar, font, assign_visuals);
            return progress_bar;
        } else {
            captainslog_debug("Unable to create progress bar control");
            return nullptr;
        }
    } else {
        captainslog_debug("Can't create progressBar gadget, instance data not progressBar type");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Static_Text(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _TextData *data,
    GameFont *font,
    bool assign_visuals)
{
    inst_data->m_style &= ~GWS_TAB_STOP;

    if ((inst_data->Get_Style() & GWS_STATIC_TEXT) != 0) {
        GameWindow *static_text = Win_Create(parent, status, x, y, width, height, Gadget_Static_Text_System, inst_data);

        if (static_text != nullptr) {
            static_text->Win_Set_Owner(parent);
            static_text->Win_Set_Input_Func(Gadget_Static_Text_Input);
            WindowDrawFunc draw;

            if ((static_text->Win_Get_Status() & WIN_STATUS_IMAGE) != 0) {
                draw = Get_Static_Text_Image_Draw_Func();
            } else {
                draw = Get_Static_Text_Draw_Func();
            }

            static_text->Win_Set_Draw_Func(draw);
            _TextData *st_data = new _TextData;
            memcpy(st_data, data, sizeof(_TextData));
            st_data->m_text = g_theDisplayStringManager->New_Display_String();
            st_data->m_text->Set_Word_Wrap_Centered((inst_data->Get_Status() & WIN_STATUS_WRAP_CENTERED) != 0);
            static_text->Win_Set_User_Data(st_data);
            Assign_Default_Gadget_Look(static_text, font, assign_visuals);
            Utf16String str = Win_Text_Label_To_Text(inst_data->m_textLabelString);

            if (str.Get_Length() != 0) {
                Gadget_Static_Text_Set_Text(static_text, str);
            }
        }
        return static_text;
    } else {
        captainslog_debug("Go_Go_Gadget_Static_Text warning: unrecognized text style.");
        return nullptr;
    }
}

GameWindow *GameWindowManager::Go_Go_Gadget_Text_Entry(GameWindow *parent,
    unsigned int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    _EntryData *data,
    GameFont *font,
    bool assign_visuals)
{
    if ((inst_data->Get_Style() & WIN_STATUS_BELOW) == 0) {
        captainslog_debug("Unable to create text entry, style not entry type");
        return nullptr;
    }

    GameWindow *text_entry = Win_Create(parent, status, x, y, width, height, Gadget_Text_Entry_System, inst_data);

    if (text_entry == nullptr) {
        captainslog_debug("Unable to create text entry window");
        return nullptr;
    }

    text_entry->Win_Set_Owner(parent);
    text_entry->Win_Set_Input_Func(Gadget_Text_Entry_Input);
    WindowDrawFunc draw;

    if ((text_entry->Win_Get_Status() & WIN_STATUS_IMAGE) != 0) {
        draw = Get_Text_Entry_Image_Draw_Func();
    } else {
        draw = Get_Text_Entry_Draw_Func();
    }

    text_entry->Win_Set_Draw_Func(draw);

    if (data->m_text != nullptr) {
        data->m_charPos = data->m_text->Get_Text_Length();
    } else {
        data->m_charPos = 0;
    }

    data->m_conCharPos = 0;
    data->m_receivedUnichar = 0;

    if (data->m_maxTextLen >= 256) {
        data->m_maxTextLen = 256;
    }

    _EntryData *te_data = new _EntryData;
    memcpy(te_data, data, sizeof(_EntryData));
    te_data->m_text = g_theDisplayStringManager->New_Display_String();
    te_data->m_sText = g_theDisplayStringManager->New_Display_String();
    te_data->m_constructText = g_theDisplayStringManager->New_Display_String();

    if (data->m_text != nullptr) {
        te_data->m_text->Set_Text(data->m_text->Get_Text());
    }

    if (data->m_sText != nullptr) {
        te_data->m_sText->Set_Text(data->m_sText->Get_Text());
    }

    text_entry->Win_Set_User_Data(te_data);
    te_data->m_constructList = 0;
    Assign_Default_Gadget_Look(text_entry, font, assign_visuals);
    Utf16String str = Win_Text_Label_To_Text(inst_data->m_textLabelString);

    if (str.Get_Length() != 0) {
        Gadget_Text_Entry_Set_Text(text_entry, str);
    }

    return text_entry;
}

void GameWindowManager::Assign_Default_Gadget_Look(GameWindow *gadget, GameFont *font, bool assign_visuals)
{
    static int red = g_theWindowManager->Win_Make_Color(255, 0, 0, 255);
    static int dark_red = g_theWindowManager->Win_Make_Color(128, 0, 0, 255);
    static int light_red = g_theWindowManager->Win_Make_Color(255, 128, 128, 255);
    static int green = g_theWindowManager->Win_Make_Color(0, 255, 0, 255);
    static int dark_green = g_theWindowManager->Win_Make_Color(0, 128, 0, 255);
    static int light_green = g_theWindowManager->Win_Make_Color(128, 255, 128, 255);
    static int blue = g_theWindowManager->Win_Make_Color(0, 0, 255, 255);
    static int dark_blue = g_theWindowManager->Win_Make_Color(0, 0, 128, 255);
    static int light_blue = g_theWindowManager->Win_Make_Color(128, 128, 255, 255);
    static int purple = g_theWindowManager->Win_Make_Color(255, 0, 255, 255);
    static int dark_purple = g_theWindowManager->Win_Make_Color(128, 0, 128, 255);
    static int light_purple = g_theWindowManager->Win_Make_Color(255, 128, 255, 255);
    static int yellow = g_theWindowManager->Win_Make_Color(255, 255, 0, 255);
    static int dark_yellow = g_theWindowManager->Win_Make_Color(128, 128, 0, 255);
    static int light_yellow = g_theWindowManager->Win_Make_Color(255, 255, 128, 255);
    static int cyan = g_theWindowManager->Win_Make_Color(0, 255, 255, 255);
    static int dark_cyan = g_theWindowManager->Win_Make_Color(64, 128, 128, 255);
    static int light_cyan = g_theWindowManager->Win_Make_Color(128, 255, 255, 255);
    static int gray = g_theWindowManager->Win_Make_Color(128, 128, 128, 255);
    static int dark_gray = g_theWindowManager->Win_Make_Color(64, 64, 64, 255);
    static int light_gray = g_theWindowManager->Win_Make_Color(192, 192, 192, 255);
    static int black = g_theWindowManager->Win_Make_Color(0, 0, 0, 255);
    static int white = g_theWindowManager->Win_Make_Color(254, 254, 254, 255);

    static int enabled_text = white;
    static int enabled_text_border = dark_gray;
    static int disabled_text = dark_gray;
    static int disabled_text_border = black;
    static int hilite_text = light_blue;
    static int hilite_text_border = blue;
    static int ime_composite_text = green;
    static int ime_composite_text_border = blue;

    if (gadget != nullptr) {
        WinInstanceData *instance_data = gadget->Win_Get_Instance_Data();

        if (font != nullptr) {
            gadget->Win_Set_Font(font);
        } else if (g_theGlobalLanguage != nullptr && g_theGlobalLanguage->Default_Window_Font().Name().Is_Not_Empty()) {
            gadget->Win_Set_Font(Win_Find_Font(g_theGlobalLanguage->Default_Window_Font().Name(),
                g_theGlobalLanguage->Default_Window_Font().Point_Size(),
                g_theGlobalLanguage->Default_Window_Font().Bold()));
        } else {
            gadget->Win_Set_Font(Win_Find_Font("Times New Roman", 14, false));
        }

        if (assign_visuals) {
            if ((instance_data->Get_Style() & GWS_PUSH_BUTTON) != 0) {
                Gadget_Button_Set_Enabled_Image(gadget, Win_Find_Image("PushButtonEnabled"));
                Gadget_Button_Set_Enabled_Color(gadget, red);
                Gadget_Button_Set_Enabled_Border_Color(gadget, light_red);
                Gadget_Button_Set_Enabled_Selected_Image(gadget, Win_Find_Image("PushButtonEnabledSelected"));
                Gadget_Button_Set_Enabled_Selected_Color(gadget, yellow);
                Gadget_Button_Set_Enabled_Selected_Border_Color(gadget, white);

                Gadget_Button_Set_Disabled_Image(gadget, Win_Find_Image("PushButtonDisabled"));
                Gadget_Button_Set_Disabled_Color(gadget, gray);
                Gadget_Button_Set_Disabled_Border_Color(gadget, light_gray);
                Gadget_Button_Set_Disabled_Selected_Image(gadget, Win_Find_Image("PushButtonDisabledSelected"));
                Gadget_Button_Set_Disabled_Selected_Color(gadget, light_gray);
                Gadget_Button_Set_Disabled_Selected_Border_Color(gadget, gray);

                Gadget_Button_Set_Hilite_Image(gadget, Win_Find_Image("PushButtonHilite"));
                Gadget_Button_Set_Hilite_Color(gadget, green);
                Gadget_Button_Set_Hilite_Border_Color(gadget, dark_green);
                Gadget_Button_Set_Hilite_Selected_Image(gadget, Win_Find_Image("PushButtonHiliteSelected"));
                Gadget_Button_Set_Hilite_Selected_Color(gadget, yellow);
                Gadget_Button_Set_Hilite_Selected_Border_Color(gadget, white);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);
            } else if ((instance_data->Get_Style() & GWS_CHECK_BOX) != 0) {
                Gadget_Check_Box_Set_Enabled_Image(gadget, Win_Find_Image("CheckBoxEnabled"));
                Gadget_Check_Box_Set_Enabled_Color(gadget, red);
                Gadget_Check_Box_Set_Enabled_Border_Color(gadget, light_red);
                Gadget_Check_Box_Set_Enabled_Box_Unselected_Image(gadget, Win_Find_Image("CheckBoxEnabledBoxUnselected"));
                Gadget_Check_Box_Set_Enabled_Box_Unselected_Color(gadget, 0xFFFFFF);
                Gadget_Check_Box_Set_Enabled_Box_Unselected_Border_Color(gadget, light_blue);
                Gadget_Check_Box_Set_Enabled_Box_Selected_Image(gadget, Win_Find_Image("CheckBoxEnabledBoxSelected"));
                Gadget_Check_Box_Set_Enabled_Box_Selected_Color(gadget, blue);
                Gadget_Check_Box_Set_Enabled_Box_Selected_Border_Color(gadget, light_blue);

                Gadget_Check_Box_Set_Disabled_Image(gadget, Win_Find_Image("CheckBoxDisabled"));
                Gadget_Check_Box_Set_Disabled_Color(gadget, gray);
                Gadget_Check_Box_Set_Disabled_Border_Color(gadget, light_gray);
                Gadget_Check_Box_Set_Disabled_Box_Unselected_Image(gadget, Win_Find_Image("CheckBoxDisabledBoxUnselected"));
                Gadget_Check_Box_Set_Disabled_Box_Unselected_Color(gadget, 0xFFFFFF);
                Gadget_Check_Box_Set_Disabled_Box_Unselected_Border_Color(gadget, light_gray);
                Gadget_Check_Box_Set_Disabled_Box_Selected_Image(gadget, Win_Find_Image("CheckBoxDisabledBoxSelected"));
                Gadget_Check_Box_Set_Disabled_Box_Selected_Color(gadget, dark_gray);
                Gadget_Check_Box_Set_Disabled_Box_Selected_Border_Color(gadget, white);

                Gadget_Check_Box_Set_Hilite_Image(gadget, Win_Find_Image("CheckBoxHilite"));
                Gadget_Check_Box_Set_Hilite_Color(gadget, green);
                Gadget_Check_Box_Set_Hilite_Border_Color(gadget, light_green);
                Gadget_Check_Box_Set_Hilite_Box_Unselected_Image(gadget, Win_Find_Image("CheckBoxHiliteBoxUnselected"));
                Gadget_Check_Box_Set_Hilite_Box_Unselected_Color(gadget, 0xFFFFFF);
                Gadget_Check_Box_Set_Hilite_Box_Unselected_Border_Color(gadget, light_blue);
                Gadget_Check_Box_Set_Hilite_Box_Selected_Image(gadget, Win_Find_Image("CheckBoxHiliteBoxSelected"));
                Gadget_Check_Box_Set_Hilite_Box_Selected_Color(gadget, yellow);
                Gadget_Check_Box_Set_Hilite_Box_Selected_Border_Color(gadget, white);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);
            } else if ((instance_data->Get_Style() & GWS_RADIO_BUTTON) != 0) {
                Gadget_Radio_Button_Set_Enabled_Image(gadget, Win_Find_Image("RadioButtonEnabled"));
                Gadget_Radio_Button_Set_Enabled_Color(gadget, red);
                Gadget_Radio_Button_Set_Enabled_Border_Color(gadget, light_red);
                Gadget_Radio_Button_Set_Enabled_Box_Unselected_Image(
                    gadget, Win_Find_Image("RadioButtonEnabledBoxUnselected"));
                Gadget_Radio_Button_Set_Enabled_Box_Unselected_Color(gadget, dark_red);
                Gadget_Radio_Button_Set_Enabled_Box_Unselected_Border_Color(gadget, black);
                Gadget_Radio_Button_Set_Enabled_Box_Selected_Image(gadget, Win_Find_Image("RadioButtonEnabledBoxSelected"));
                Gadget_Radio_Button_Set_Enabled_Box_Selected_Color(gadget, blue);
                Gadget_Radio_Button_Set_Enabled_Box_Selected_Border_Color(gadget, light_blue);

                Gadget_Radio_Button_Set_Disabled_Image(gadget, Win_Find_Image("RadioButtonDisabled"));
                Gadget_Radio_Button_Set_Disabled_Color(gadget, gray);
                Gadget_Radio_Button_Set_Disabled_Border_Color(gadget, light_gray);
                Gadget_Radio_Button_Set_Disabled_Box_Unselected_Image(
                    gadget, Win_Find_Image("RadioButtonDisabledBoxUnselected"));
                Gadget_Radio_Button_Set_Disabled_Box_Unselected_Color(gadget, gray);
                Gadget_Radio_Button_Set_Disabled_Box_Unselected_Border_Color(gadget, light_gray);
                Gadget_Radio_Button_Set_Disabled_Box_Selected_Image(
                    gadget, Win_Find_Image("RadioButtonDisabledBoxSelected"));
                Gadget_Radio_Button_Set_Disabled_Box_Selected_Color(gadget, dark_gray);
                Gadget_Radio_Button_Set_Disabled_Box_Selected_Border_Color(gadget, white);

                Gadget_Radio_Button_Set_Hilite_Image(gadget, Win_Find_Image("RadioButtonHilite"));
                Gadget_Radio_Button_Set_Hilite_Color(gadget, green);
                Gadget_Radio_Button_Set_Hilite_Border_Color(gadget, light_green);
                Gadget_Radio_Button_Set_Hilite_Box_Unselected_Image(
                    gadget, Win_Find_Image("RadioButtonHiliteBoxUnselected"));
                Gadget_Radio_Button_Set_Hilite_Box_Unselected_Color(gadget, dark_green);
                Gadget_Radio_Button_Set_Hilite_Box_Unselected_Border_Color(gadget, light_green);
                Gadget_Radio_Button_Set_Hilite_Box_Selected_Image(gadget, Win_Find_Image("RadioButtonHiliteBoxSelected"));
                Gadget_Radio_Button_Set_Hilite_Box_Selected_Color(gadget, yellow);
                Gadget_Radio_Button_Set_Hilite_Box_Selected_Border_Color(gadget, white);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);
            } else if ((instance_data->Get_Style() & GWS_HORZ_SLIDER) != 0) {
                Gadget_Slider_Set_Enabled_Left_End_Image(gadget, Win_Find_Image("HSliderEnabledLeftEnd"));
                Gadget_Slider_Set_Enabled_Right_End_Image(gadget, Win_Find_Image("HSliderEnabledRightEnd"));
                Gadget_Slider_Set_Enabled_Repeating_Centre_Image(gadget, Win_Find_Image("HSliderEnabledRepeatingCenter"));
                Gadget_Slider_Set_Enabled_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("HSliderEnabledSmallRepeatingCenter"));
                Gadget_Slider_Set_Enabled_Color(gadget, red);
                Gadget_Slider_Set_Enabled_Border_Color(gadget, light_red);

                Gadget_Slider_Set_Disabled_Left_End_Image(gadget, Win_Find_Image("HSliderDisabledLeftEnd"));
                Gadget_Slider_Set_Disabled_Right_End_Image(gadget, Win_Find_Image("HSliderDisabledRightEnd"));
                Gadget_Slider_Set_Disabled_Repeating_Centre_Image(gadget, Win_Find_Image("HSliderDisabledRepeatingCenter"));
                Gadget_Slider_Set_Disabled_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("HSliderDisabledSmallRepeatingCenter"));
                Gadget_Slider_Set_Disabled_Color(gadget, red);
                Gadget_Slider_Set_Disabled_Border_Color(gadget, light_red);

                Gadget_Slider_Set_Hilite_Left_End_Image(gadget, Win_Find_Image("HSliderHiliteLeftEnd"));
                Gadget_Slider_Set_Hilite_Right_End_Image(gadget, Win_Find_Image("HSliderHiliteRightEnd"));
                Gadget_Slider_Set_Hilite_Repeating_Centre_Image(gadget, Win_Find_Image("HSliderHiliteRepeatingCenter"));
                Gadget_Slider_Set_Hilite_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("HSliderHiliteSmallRepeatingCenter"));
                Gadget_Slider_Set_Hilite_Color(gadget, red);
                Gadget_Slider_Set_Hilite_Border_Color(gadget, light_red);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);

                Gadget_Slider_Set_Thumb_Enabled_Image(gadget, Win_Find_Image("HSliderThumbEnabled"));
                Gadget_Slider_Set_Thumb_Enabled_Color(gadget, Gadget_Slider_Get_Enabled_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Border_Color(gadget, Gadget_Slider_Get_Enabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Image(gadget, Win_Find_Image("HSliderThumbEnabled"));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Color(gadget, Gadget_Slider_Get_Enabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Border_Color(gadget, Gadget_Slider_Get_Enabled_Color(gadget));

                Gadget_Slider_Set_Thumb_Disabled_Image(gadget, Win_Find_Image("HSliderThumbDisabled"));
                Gadget_Slider_Set_Thumb_Disabled_Color(gadget, Gadget_Slider_Get_Disabled_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Border_Color(gadget, Gadget_Slider_Get_Disabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Image(gadget, Win_Find_Image("HSliderThumbDisabled"));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Color(gadget, Gadget_Slider_Get_Disabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Border_Color(gadget, Gadget_Slider_Get_Disabled_Color(gadget));

                Gadget_Slider_Set_Thumb_Hilite_Image(gadget, Win_Find_Image("HSliderThumbHilite"));
                Gadget_Slider_Set_Thumb_Hilite_Color(gadget, Gadget_Slider_Get_Hilite_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Border_Color(gadget, Gadget_Slider_Get_Hilite_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Image(gadget, Win_Find_Image("HSliderThumbHiliteSelected"));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Color(gadget, Gadget_Slider_Get_Hilite_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Border_Color(gadget, Gadget_Slider_Get_Hilite_Color(gadget));
            } else if ((instance_data->Get_Style() & GWS_VERT_SLIDER) != 0) {
                Gadget_Slider_Set_Enabled_Top_End_Image(gadget, Win_Find_Image("VSliderEnabledTopEnd"));
                Gadget_Slider_Set_Enabled_Bottom_End_Image(gadget, Win_Find_Image("VSliderEnabledBottomEnd"));
                Gadget_Slider_Set_Enabled_Repeating_Centre_Image(gadget, Win_Find_Image("VSliderEnabledRepeatingCenter"));
                Gadget_Slider_Set_Enabled_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("VSliderEnabledSmallRepeatingCenter"));
                Gadget_Slider_Set_Enabled_Color(gadget, red);
                Gadget_Slider_Set_Enabled_Border_Color(gadget, light_red);

                Gadget_Slider_Set_Disabled_Top_End_Image(gadget, Win_Find_Image("VSliderDisabledTopEnd"));
                Gadget_Slider_Set_Disabled_Bottom_End_Image(gadget, Win_Find_Image("VSliderDisabledBottomEnd"));
                Gadget_Slider_Set_Disabled_Repeating_Centre_Image(gadget, Win_Find_Image("VSliderDisabledRepeatingCenter"));
                Gadget_Slider_Set_Disabled_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("VSliderDisabledSmallRepeatingCenter"));
                Gadget_Slider_Set_Disabled_Color(gadget, red);
                Gadget_Slider_Set_Disabled_Border_Color(gadget, light_red);

                Gadget_Slider_Set_Hilite_Top_End_Image(gadget, Win_Find_Image("VSliderHiliteTopEnd"));
                Gadget_Slider_Set_Hilite_Bottom_End_Image(gadget, Win_Find_Image("VSliderHiliteBottomEnd"));
                Gadget_Slider_Set_Hilite_Repeating_Centre_Image(gadget, Win_Find_Image("VSliderHiliteRepeatingCenter"));
                Gadget_Slider_Set_Hilite_Small_Repeating_Centre_Image(
                    gadget, Win_Find_Image("VSliderHiliteSmallRepeatingCenter"));
                Gadget_Slider_Set_Hilite_Color(gadget, red);
                Gadget_Slider_Set_Hilite_Border_Color(gadget, light_red);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);

                Gadget_Slider_Set_Thumb_Enabled_Image(gadget, Win_Find_Image("VSliderThumbEnabled"));
                Gadget_Slider_Set_Thumb_Enabled_Color(gadget, Gadget_Slider_Get_Enabled_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Border_Color(gadget, Gadget_Slider_Get_Enabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Image(gadget, Win_Find_Image("VSliderThumbEnabled"));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Color(gadget, Gadget_Slider_Get_Enabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Enabled_Selected_Border_Color(gadget, Gadget_Slider_Get_Enabled_Color(gadget));

                Gadget_Slider_Set_Thumb_Disabled_Image(gadget, Win_Find_Image("VSliderThumbDisabled"));
                Gadget_Slider_Set_Thumb_Disabled_Color(gadget, Gadget_Slider_Get_Disabled_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Border_Color(gadget, Gadget_Slider_Get_Disabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Image(gadget, Win_Find_Image("VSliderThumbDisabled"));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Color(gadget, Gadget_Slider_Get_Disabled_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Disabled_Selected_Border_Color(gadget, Gadget_Slider_Get_Disabled_Color(gadget));

                Gadget_Slider_Set_Thumb_Hilite_Image(gadget, Win_Find_Image("VSliderThumbHilite"));
                Gadget_Slider_Set_Thumb_Hilite_Color(gadget, Gadget_Slider_Get_Hilite_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Border_Color(gadget, Gadget_Slider_Get_Hilite_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Image(gadget, Win_Find_Image("VSliderThumbHiliteSelected"));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Color(gadget, Gadget_Slider_Get_Hilite_Border_Color(gadget));
                Gadget_Slider_Set_Thumb_Hilite_Selected_Border_Color(gadget, Gadget_Slider_Get_Hilite_Color(gadget));
            } else if ((instance_data->Get_Style() & GWS_SCROLL_LISTBOX) != 0) {
                _ListboxData *lb_data = static_cast<_ListboxData *>(gadget->Win_Get_User_Data());
                Gadget_List_Box_Set_Colors(gadget,
                    red,
                    light_red,
                    yellow,
                    white,
                    gray,
                    light_gray,
                    light_gray,
                    white,
                    green,
                    dark_green,
                    white,
                    dark_green);

                Gadget_List_Box_Set_Enabled_Image(gadget, Win_Find_Image("ListBoxEnabled"));
                Gadget_List_Box_Set_Enabled_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemLeftEnd"));
                Gadget_List_Box_Set_Enabled_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemRightEnd"));
                Gadget_List_Box_Set_Enabled_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemRepeatingCenter"));
                Gadget_List_Box_Set_Enabled_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemSmallRepeatingCenter"));

                Gadget_List_Box_Set_Disabled_Image(gadget, Win_Find_Image("ListBoxDisabled"));
                Gadget_List_Box_Set_Disabled_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemLeftEnd"));
                Gadget_List_Box_Set_Disabled_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemRightEnd"));
                Gadget_List_Box_Set_Disabled_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemRepeatingCenter"));
                Gadget_List_Box_Set_Disabled_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemSmallRepeatingCenter"));

                Gadget_List_Box_Set_Hilite_Image(gadget, Win_Find_Image("ListBoxHilite"));
                Gadget_List_Box_Set_Hilite_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemLeftEnd"));
                Gadget_List_Box_Set_Hilite_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemRightEnd"));
                Gadget_List_Box_Set_Hilite_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemRepeatingCenter"));
                Gadget_List_Box_Set_Hilite_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemSmallRepeatingCenter"));

                GameWindow *lb_slider = lb_data->m_slider;

                if (lb_slider != nullptr) {
                    GameWindow *lb_up_button = lb_data->m_upButton;
                    GameWindow *lb_down_button = lb_data->m_downButton;

                    Gadget_Slider_Set_Enabled_Top_End_Image(lb_slider, Win_Find_Image("VSliderLargeEnabledTopEnd"));
                    Gadget_Slider_Set_Enabled_Bottom_End_Image(lb_slider, Win_Find_Image("VSliderLargeEnabledBottomEnd"));
                    Gadget_Slider_Set_Enabled_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeEnabledRepeatingCenter"));
                    Gadget_Slider_Set_Enabled_Small_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeEnabledSmallRepeatingCenter"));
                    Gadget_Slider_Set_Thumb_Enabled_Image(lb_slider, Win_Find_Image("VSliderLargeThumbEnabled"));
                    Gadget_Slider_Set_Thumb_Enabled_Selected_Image(lb_slider, Win_Find_Image("VSliderLargeThumbEnabled"));

                    Gadget_Slider_Set_Disabled_Top_End_Image(lb_slider, Win_Find_Image("VSliderLargeDisabledTopEnd"));
                    Gadget_Slider_Set_Disabled_Bottom_End_Image(lb_slider, Win_Find_Image("VSliderLargeDisabledBottomEnd"));
                    Gadget_Slider_Set_Disabled_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeDisabledRepeatingCenter"));
                    Gadget_Slider_Set_Disabled_Small_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeDisabledSmallRepeatingCenter"));
                    Gadget_Slider_Set_Thumb_Disabled_Image(lb_slider, Win_Find_Image("VSliderLargeThumbDisabled"));
                    Gadget_Slider_Set_Thumb_Disabled_Selected_Image(lb_slider, Win_Find_Image("VSliderLargeThumbDisabled"));

                    Gadget_Slider_Set_Hilite_Top_End_Image(lb_slider, Win_Find_Image("VSliderLargeHiliteTopEnd"));
                    Gadget_Slider_Set_Hilite_Bottom_End_Image(lb_slider, Win_Find_Image("VSliderLargeHiliteBottomEnd"));
                    Gadget_Slider_Set_Hilite_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeHiliteRepeatingCenter"));
                    Gadget_Slider_Set_Hilite_Small_Repeating_Centre_Image(
                        lb_slider, Win_Find_Image("VSliderLargeHiliteSmallRepeatingCenter"));
                    Gadget_Slider_Set_Thumb_Hilite_Image(lb_slider, Win_Find_Image("VSliderLargeThumbHilite"));
                    Gadget_Slider_Set_Thumb_Hilite_Selected_Image(lb_slider, Win_Find_Image("VSliderLargeThumbHilite"));

                    Gadget_Button_Set_Enabled_Image(lb_up_button, Win_Find_Image("VSliderLargeUpButtonEnabled"));
                    Gadget_Button_Set_Enabled_Selected_Image(lb_up_button, Win_Find_Image("VSliderLargeUpButtonEnabled"));
                    Gadget_Button_Set_Disabled_Image(lb_up_button, Win_Find_Image("VSliderLargeUpButtonDisabled"));
                    Gadget_Button_Set_Disabled_Selected_Image(lb_up_button, Win_Find_Image("VSliderLargeUpButtonDisabled"));
                    Gadget_Button_Set_Hilite_Image(lb_up_button, Win_Find_Image("VSliderLargeUpButtonHilite"));
                    Gadget_Button_Set_Hilite_Selected_Image(
                        lb_up_button, Win_Find_Image("VSliderLargeUpButtonHiliteSelected"));

                    Gadget_Button_Set_Enabled_Image(lb_down_button, Win_Find_Image("VSliderLargeDownButtonEnabled"));
                    Gadget_Button_Set_Enabled_Selected_Image(
                        lb_down_button, Win_Find_Image("VSliderLargeDownButtonEnabled"));
                    Gadget_Button_Set_Disabled_Image(lb_down_button, Win_Find_Image("VSliderLargeDownButtonDisabled"));
                    Gadget_Button_Set_Disabled_Selected_Image(
                        lb_down_button, Win_Find_Image("VSliderLargeDownButtonDisabled"));
                    Gadget_Button_Set_Hilite_Image(lb_down_button, Win_Find_Image("VSliderLargeDownButtonHilite"));
                    Gadget_Button_Set_Hilite_Selected_Image(
                        lb_down_button, Win_Find_Image("VSliderLargeDownButtonHiliteSelected"));
                }
            } else if ((instance_data->Get_Style() & GWS_COMBO_BOX) != 0) {
                Gadget_Combo_Box_Set_Colors(gadget,
                    red,
                    light_red,
                    yellow,
                    white,
                    gray,
                    light_gray,
                    light_gray,
                    white,
                    green,
                    dark_green,
                    white,
                    dark_green);

                Gadget_Combo_Box_Set_Enabled_Image(gadget, Win_Find_Image("ListBoxEnabled"));
                Gadget_Combo_Box_Set_Enabled_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemLeftEnd"));
                Gadget_Combo_Box_Set_Enabled_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemRightEnd"));
                Gadget_Combo_Box_Set_Enabled_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemRepeatingCenter"));
                Gadget_Combo_Box_Set_Enabled_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxEnabledSelectedItemSmallRepeatingCenter"));

                Gadget_Combo_Box_Set_Disabled_Image(gadget, Win_Find_Image("ListBoxDisabled"));
                Gadget_Combo_Box_Set_Disabled_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemLeftEnd"));
                Gadget_Combo_Box_Set_Disabled_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemRightEnd"));
                Gadget_Combo_Box_Set_Disabled_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemRepeatingCenter"));
                Gadget_Combo_Box_Set_Disabled_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxDisabledSelectedItemSmallRepeatingCenter"));

                Gadget_Combo_Box_Set_Hilite_Image(gadget, Win_Find_Image("ListBoxHilite"));
                Gadget_Combo_Box_Set_Hilite_Selected_Item_Left_End_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemLeftEnd"));
                Gadget_Combo_Box_Set_Hilite_Selected_Item_Right_End_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemRightEnd"));
                Gadget_Combo_Box_Set_Hilite_Selected_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemRepeatingCenter"));
                Gadget_Combo_Box_Set_Hilite_Selected_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ListBoxHiliteSelectedItemSmallRepeatingCenter"));

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);

                GameWindow *cb_drop_down_button = Gadget_Combo_Box_Get_Drop_Down_Button(gadget);

                if (cb_drop_down_button != nullptr) {
                    Gadget_Button_Set_Enabled_Image(cb_drop_down_button, Win_Find_Image("PushButtonEnabled"));
                    Gadget_Button_Set_Enabled_Selected_Image(
                        cb_drop_down_button, Win_Find_Image("PushButtonEnabledSelected"));
                    Gadget_Button_Set_Disabled_Image(cb_drop_down_button, Win_Find_Image("PushButtonDisabled"));
                    Gadget_Button_Set_Disabled_Selected_Image(
                        cb_drop_down_button, Win_Find_Image("PushButtonDisabledSelected"));
                    Gadget_Button_Set_Hilite_Image(cb_drop_down_button, Win_Find_Image("PushButtonHilite"));
                    Gadget_Button_Set_Hilite_Selected_Image(cb_drop_down_button, Win_Find_Image("PushButtonHiliteSelected"));
                }

                GameWindow *cb_edit_box = Gadget_Combo_Box_Get_Edit_Box(gadget);

                if (cb_edit_box != nullptr) {
                    Gadget_Text_Entry_Set_Enabled_Item_Left_End_Image(
                        cb_edit_box, Win_Find_Image("TextEntryEnabledLeftEnd"));
                    Gadget_Text_Entry_Set_Enabled_Item_Right_End_Image(
                        cb_edit_box, Win_Find_Image("TextEntryEnabledRightEnd"));
                    Gadget_Text_Entry_Set_Enabled_Item_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryEnabledRepeatingCenter"));
                    Gadget_Text_Entry_Set_Enabled_Item_Small_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryEnabledSmallRepeatingCenter"));

                    Gadget_Text_Entry_Set_Disabled_Item_Left_End_Image(
                        cb_edit_box, Win_Find_Image("TextEntryDisabledLeftEnd"));
                    Gadget_Text_Entry_Set_Disabled_Item_Right_End_Image(
                        cb_edit_box, Win_Find_Image("TextEntryDisabledRightEnd"));
                    Gadget_Text_Entry_Set_Disabled_Item_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryDisabledRepeatingCenter"));
                    Gadget_Text_Entry_Set_Disabled_Item_Small_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryDisabledSmallRepeatingCenter"));

                    Gadget_Text_Entry_Set_Hilite_Item_Left_End_Image(cb_edit_box, Win_Find_Image("TextEntryHiliteLeftEnd"));
                    Gadget_Text_Entry_Set_Hilite_Item_Right_End_Image(
                        cb_edit_box, Win_Find_Image("TextEntryHiliteRightEnd"));
                    Gadget_Text_Entry_Set_Hilite_Item_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryHiliteRepeatingCenter"));
                    Gadget_Text_Entry_Set_Hilite_Item_Small_Repeating_Center_Image(
                        cb_edit_box, Win_Find_Image("TextEntryHiliteSmallRepeatingCenter"));
                }

                GameWindow *cb_list_box = Gadget_Combo_Box_Get_List_Box(gadget);

                if (cb_list_box != nullptr) {
                    Gadget_List_Box_Set_Enabled_Image(cb_list_box, Win_Find_Image("ListBoxEnabled"));
                    Gadget_List_Box_Set_Enabled_Selected_Item_Left_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxEnabledSelectedItemLeftEnd"));
                    Gadget_List_Box_Set_Enabled_Selected_Item_Right_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxEnabledSelectedItemRightEnd"));
                    Gadget_List_Box_Set_Enabled_Selected_Item_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxEnabledSelectedItemRepeatingCenter"));
                    Gadget_List_Box_Set_Enabled_Selected_Item_Small_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxEnabledSelectedItemSmallRepeatingCenter"));

                    Gadget_List_Box_Set_Disabled_Image(cb_list_box, Win_Find_Image("ListBoxDisabled"));
                    Gadget_List_Box_Set_Disabled_Selected_Item_Left_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxDisabledSelectedItemLeftEnd"));
                    Gadget_List_Box_Set_Disabled_Selected_Item_Right_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxDisabledSelectedItemRightEnd"));
                    Gadget_List_Box_Set_Disabled_Selected_Item_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxDisabledSelectedItemRepeatingCenter"));
                    Gadget_List_Box_Set_Disabled_Selected_Item_Small_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxDisabledSelectedItemSmallRepeatingCenter"));

                    Gadget_List_Box_Set_Hilite_Image(cb_list_box, Win_Find_Image("ListBoxHilite"));
                    Gadget_List_Box_Set_Hilite_Selected_Item_Left_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxHiliteSelectedItemLeftEnd"));
                    Gadget_List_Box_Set_Hilite_Selected_Item_Right_End_Image(
                        cb_list_box, Win_Find_Image("ListBoxHiliteSelectedItemRightEnd"));
                    Gadget_List_Box_Set_Hilite_Selected_Item_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxHiliteSelectedItemRepeatingCenter"));
                    Gadget_List_Box_Set_Hilite_Selected_Item_Small_Repeating_Center_Image(
                        cb_list_box, Win_Find_Image("ListBoxHiliteSelectedItemSmallRepeatingCenter"));
                    GameWindow *cb_list_box_slider = Gadget_List_Box_Get_Slider(cb_list_box);

                    if (cb_list_box_slider != nullptr) {
                        GameWindow *cb_list_box_slider_up_button = Gadget_List_Box_Get_Up_Button(cb_list_box);
                        GameWindow *cb_list_box_slider_down_button = Gadget_List_Box_Get_Down_Button(cb_list_box);

                        Gadget_Slider_Set_Enabled_Top_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeEnabledTopEnd"));
                        Gadget_Slider_Set_Enabled_Bottom_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeEnabledBottomEnd"));
                        Gadget_Slider_Set_Enabled_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeEnabledRepeatingCenter"));
                        Gadget_Slider_Set_Enabled_Small_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeEnabledSmallRepeatingCenter"));
                        Gadget_Slider_Set_Thumb_Enabled_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeThumbEnabled"));
                        Gadget_Slider_Set_Thumb_Enabled_Selected_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeThumbEnabled"));

                        Gadget_Slider_Set_Disabled_Top_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeDisabledTopEnd"));
                        Gadget_Slider_Set_Disabled_Bottom_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeDisabledBottomEnd"));
                        Gadget_Slider_Set_Disabled_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeDisabledRepeatingCenter"));
                        Gadget_Slider_Set_Disabled_Small_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeDisabledSmallRepeatingCenter"));
                        Gadget_Slider_Set_Thumb_Disabled_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeThumbDisabled"));
                        Gadget_Slider_Set_Thumb_Disabled_Selected_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeThumbDisabled"));

                        Gadget_Slider_Set_Hilite_Top_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeHiliteTopEnd"));
                        Gadget_Slider_Set_Hilite_Bottom_End_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeHiliteBottomEnd"));
                        Gadget_Slider_Set_Hilite_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeHiliteRepeatingCenter"));
                        Gadget_Slider_Set_Hilite_Small_Repeating_Centre_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeHiliteSmallRepeatingCenter"));
                        Gadget_Slider_Set_Thumb_Hilite_Image(cb_list_box_slider, Win_Find_Image("VSliderLargeThumbHilite"));
                        Gadget_Slider_Set_Thumb_Hilite_Selected_Image(
                            cb_list_box_slider, Win_Find_Image("VSliderLargeThumbHilite"));

                        Gadget_Button_Set_Enabled_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonEnabled"));
                        Gadget_Button_Set_Enabled_Selected_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonEnabled"));
                        Gadget_Button_Set_Disabled_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonDisabled"));
                        Gadget_Button_Set_Disabled_Selected_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonDisabled"));
                        Gadget_Button_Set_Hilite_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonHilite"));
                        Gadget_Button_Set_Hilite_Selected_Image(
                            cb_list_box_slider_up_button, Win_Find_Image("VSliderLargeUpButtonHiliteSelected"));

                        Gadget_Button_Set_Enabled_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonEnabled"));
                        Gadget_Button_Set_Enabled_Selected_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonEnabled"));
                        Gadget_Button_Set_Disabled_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonDisabled"));
                        Gadget_Button_Set_Disabled_Selected_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonDisabled"));
                        Gadget_Button_Set_Hilite_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonHilite"));
                        Gadget_Button_Set_Hilite_Selected_Image(
                            cb_list_box_slider_down_button, Win_Find_Image("VSliderLargeDownButtonHiliteSelected"));
                    }
                }
            } else if ((instance_data->Get_Style() & GWS_PROGRESS_BAR) != 0) {
                Gadget_Progress_Bar_Set_Enabled_Color(gadget, red);
                Gadget_Progress_Bar_Set_Enabled_Border_Color(gadget, light_red);
                Gadget_Progress_Bar_Set_Enabled_Left_End_Image(gadget, Win_Find_Image("ProgressBarEnabledLeftEnd"));
                Gadget_Progress_Bar_Set_Enabled_Right_End_Image(gadget, Win_Find_Image("ProgressBarEnabledRightEnd"));
                Gadget_Progress_Bar_Set_Enabled_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarEnabledRepeatingCenter"));
                Gadget_Progress_Bar_Set_Enabled_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarEnabledSmallRepeatingCenter"));

                Gadget_Progress_Bar_Set_Enabled_Bar_Color(gadget, yellow);
                Gadget_Progress_Bar_Set_Enabled_Bar_Border_Color(gadget, white);
                Gadget_Progress_Bar_Set_Enabled_Bar_Left_End_Image(gadget, Win_Find_Image("ProgressBarEnabledBarLeftEnd"));
                Gadget_Progress_Bar_Set_Enabled_Bar_Right_End_Image(gadget, Win_Find_Image("ProgressBarEnabledBarRightEnd"));
                Gadget_Progress_Bar_Set_Enabled_Bar_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarEnabledBarRepeatingCenter"));
                Gadget_Progress_Bar_Set_Enabled_Bar_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarEnabledBarSmallRepeatingCenter"));

                Gadget_Progress_Bar_Set_Disabled_Color(gadget, dark_gray);
                Gadget_Progress_Bar_Set_Disabled_Border_Color(gadget, light_gray);
                Gadget_Progress_Bar_Set_Disabled_Left_End_Image(gadget, Win_Find_Image("ProgressBarDisabledLeftEnd"));
                Gadget_Progress_Bar_Set_Disabled_Right_End_Image(gadget, Win_Find_Image("ProgressBarDisabledRightEnd"));
                Gadget_Progress_Bar_Set_Disabled_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarDisabledRepeatingCenter"));
                Gadget_Progress_Bar_Set_Disabled_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarDisabledSmallRepeatingCenter"));

                Gadget_Progress_Bar_Set_Disabled_Bar_Color(gadget, light_gray);
                Gadget_Progress_Bar_Set_Disabled_Bar_Border_Color(gadget, white);
                Gadget_Progress_Bar_Set_Disabled_Bar_Left_End_Image(gadget, Win_Find_Image("ProgressBarDisabledBarLeftEnd"));
                Gadget_Progress_Bar_Set_Disabled_Bar_Right_End_Image(
                    gadget, Win_Find_Image("ProgressBarDisabledBarRightEnd"));
                Gadget_Progress_Bar_Set_Disabled_Bar_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarDisabledBarRepeatingCenter"));
                Gadget_Progress_Bar_Set_Disabled_Bar_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarDisabledBarSmallRepeatingCenter"));

                Gadget_Progress_Bar_Set_Hilite_Color(gadget, green);
                Gadget_Progress_Bar_Set_Hilite_Border_Color(gadget, dark_green);
                Gadget_Progress_Bar_Set_Hilite_Left_End_Image(gadget, Win_Find_Image("ProgressBarHiliteLeftEnd"));
                Gadget_Progress_Bar_Set_Hilite_Right_End_Image(gadget, Win_Find_Image("ProgressBarHiliteRightEnd"));
                Gadget_Progress_Bar_Set_Hilite_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarHiliteRepeatingCenter"));
                Gadget_Progress_Bar_Set_Hilite_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarHiliteSmallRepeatingCenter"));

                Gadget_Progress_Bar_Set_Hilite_Bar_Color(gadget, yellow);
                Gadget_Progress_Bar_Set_Hilite_Bar_Border_Color(gadget, white);
                Gadget_Progress_Bar_Set_Hilite_Bar_Left_End_Image(gadget, Win_Find_Image("ProgressBarHiliteBarLeftEnd"));
                Gadget_Progress_Bar_Set_Hilite_Bar_Right_End_Image(gadget, Win_Find_Image("ProgressBarHiliteBarRightEnd"));
                Gadget_Progress_Bar_Set_Hilite_Bar_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarHiliteBarRepeatingCenter"));
                Gadget_Progress_Bar_Set_Hilite_Bar_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("ProgressBarHiliteBarSmallRepeatingCenter"));
            } else if ((instance_data->Get_Style() & GWS_STATIC_TEXT) != 0) {
                Gadget_Static_Text_Set_Enabled_Image(gadget, Win_Find_Image("StaticTextEnabled"));
                Gadget_Static_Text_Set_Enabled_Color(gadget, red);
                Gadget_Static_Text_Set_Enabled_Border_Color(gadget, light_red);

                Gadget_Static_Text_Set_Disabled_Image(gadget, Win_Find_Image("StaticTextDisabled"));
                Gadget_Static_Text_Set_Disabled_Color(gadget, dark_gray);
                Gadget_Static_Text_Set_Disabled_Border_Color(gadget, light_gray);

                Gadget_Static_Text_Set_Hilite_Image(gadget, Win_Find_Image("StaticTextHilite"));
                Gadget_Static_Text_Set_Hilite_Color(gadget, dark_green);
                Gadget_Static_Text_Set_Hilite_Border_Color(gadget, light_green);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);
            } else if ((instance_data->Get_Style() & GWS_ENTRY_FIELD) != 0) {
                Gadget_Text_Entry_Set_Enabled_Item_Left_End_Image(gadget, Win_Find_Image("TextEntryEnabledLeftEnd"));
                Gadget_Text_Entry_Set_Enabled_Item_Right_End_Image(gadget, Win_Find_Image("TextEntryEnabledRightEnd"));
                Gadget_Text_Entry_Set_Enabled_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryEnabledRepeatingCenter"));
                Gadget_Text_Entry_Set_Enabled_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryEnabledSmallRepeatingCenter"));
                Gadget_Text_Entry_Set_Enabled_Color(gadget, red);
                Gadget_Text_Entry_Set_Enabled_Border_Color(gadget, light_red);

                Gadget_Text_Entry_Set_Disabled_Item_Left_End_Image(gadget, Win_Find_Image("TextEntryDisabledLeftEnd"));
                Gadget_Text_Entry_Set_Disabled_Item_Right_End_Image(gadget, Win_Find_Image("TextEntryDisabledRightEnd"));
                Gadget_Text_Entry_Set_Disabled_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryDisabledRepeatingCenter"));
                Gadget_Text_Entry_Set_Disabled_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryDisabledSmallRepeatingCenter"));
                Gadget_Text_Entry_Set_Disabled_Color(gadget, gray);
                Gadget_Text_Entry_Set_Disabled_Border_Color(gadget, black);

                Gadget_Text_Entry_Set_Hilite_Item_Left_End_Image(gadget, Win_Find_Image("TextEntryHiliteLeftEnd"));
                Gadget_Text_Entry_Set_Hilite_Item_Right_End_Image(gadget, Win_Find_Image("TextEntryHiliteRightEnd"));
                Gadget_Text_Entry_Set_Hilite_Item_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryHiliteRepeatingCenter"));
                Gadget_Text_Entry_Set_Hilite_Item_Small_Repeating_Center_Image(
                    gadget, Win_Find_Image("TextEntryHiliteSmallRepeatingCenter"));
                Gadget_Text_Entry_Set_Hilite_Color(gadget, green);
                Gadget_Text_Entry_Set_Hilite_Border_Color(gadget, dark_green);

                gadget->Win_Set_Enabled_Text_Colors(enabled_text, enabled_text_border);
                gadget->Win_Set_Disabled_Text_Colors(disabled_text, disabled_text_border);
                gadget->Win_Set_Hilite_Text_Colors(hilite_text, hilite_text_border);
                gadget->Win_Set_IME_Composite_Text_Colors(ime_composite_text, ime_composite_text_border);
            }
        }
    }
}

Utf16String GameWindowManager::Win_Text_Label_To_Text(Utf8String label)
{
    if (label.Is_Empty()) {
        return Utf16String::s_emptyString;
    } else {
        Utf16String str;
        str.Translate(label);
        return str;
    }
}

GameWindow *GameWindowManager::Get_Window_Under_Cursor(int x, int y, bool ignore_enabled)
{
    if (m_mouseCaptor != nullptr) {
        return m_mouseCaptor->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    if (m_grabWindow != nullptr) {
        return m_grabWindow->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    if (m_modalHead != nullptr && m_modalHead->m_window != nullptr) {
        return m_modalHead->m_window->Win_Point_In_Child(x, y, ignore_enabled, false);
    }

    GameWindow *window;

    for (window = m_windowList; window != nullptr; window = window->m_next) {
        if ((window->m_status & WIN_STATUS_ABOVE) != 0 && (window->m_status & WIN_STATUS_HIDDEN) == 0
            && x >= window->m_region.lo.x && x <= window->m_region.hi.x && y >= window->m_region.lo.y
            && y <= window->m_region.hi.y && ((window->m_status & WIN_STATUS_ENABLED) != 0 || ignore_enabled)) {
            window = window->Win_Point_In_Child(x, y, ignore_enabled, false);
            break;
        }
    }

    if (window == nullptr) {
        for (window = m_windowList; window != nullptr; window = window->m_next) {
            if ((window->m_status & (WIN_STATUS_BELOW | WIN_STATUS_ABOVE | WIN_STATUS_HIDDEN)) == 0
                && x >= window->m_region.lo.x && x <= window->m_region.hi.x && y >= window->m_region.lo.y
                && y <= window->m_region.hi.y && ((window->m_status & WIN_STATUS_ENABLED) != 0 || ignore_enabled)) {
                window = window->Win_Point_In_Child(x, y, ignore_enabled, false);
                break;
            }
        }
    }

    if (window == nullptr) {
        for (window = m_windowList; window != nullptr; window = window->m_next) {
            if ((window->m_status & WIN_STATUS_BELOW) != 0 && (window->m_status & WIN_STATUS_HIDDEN) == 0
                && x >= window->m_region.lo.x && x <= window->m_region.hi.x && y >= window->m_region.lo.y
                && y <= window->m_region.hi.y && ((window->m_status & WIN_STATUS_ENABLED) != 0 || ignore_enabled)) {
                window = window->Win_Point_In_Child(x, y, ignore_enabled, false);
                break;
            }
        }
    }

    if (window != nullptr) {
        if ((window->m_status & WIN_STATUS_NO_INPUT) != 0) {
            return nullptr;
        } else if (ignore_enabled && (window->m_status & WIN_STATUS_ENABLED) == 0) {
            return nullptr;
        }
    }

    return window;
}

bool GameWindowManager::Init_Test_GUI()
{
    return true;
}

void GameWindowManager::Win_Next_Tab(GameWindow *window)
{
    if (m_tabList.size() != 0 && m_modalHead == nullptr) {
        auto it = m_tabList.begin();
        for (; it != m_tabList.end(); it++) {
            if (*it == window) {
                it++;
                break;
            }
        }

        if (it != m_tabList.end()) {
            Win_Set_Focus(*it);
        } else {
            Win_Set_Focus(*m_tabList.begin());
        }

        Win_Set_Lone_Window(nullptr);
    }
}

void GameWindowManager::Win_Prev_Tab(GameWindow *window)
{
    if (m_tabList.size() != 0 && m_modalHead == nullptr) {
        auto it = m_tabList.rbegin();
        for (; it != m_tabList.rend(); it++) {
            if (*it == window) {
                it++;
                break;
            }
        }

        if (it != m_tabList.rend()) {
            Win_Set_Focus(*it);
        } else {
            Win_Set_Focus(*m_tabList.rbegin());
        }

        Win_Set_Lone_Window(nullptr);
    }
}

void GameWindowManager::Register_Tab_List(std::list<GameWindow *> list)
{
    m_tabList.clear();
    m_tabList = list;
}

void GameWindowManager::Clear_Tab_List()
{
    m_tabList.clear();
}

WindowDrawFunc GameWindowManager::Get_Default_Draw()
{
    return Game_Win_Default_Draw;
}

WindowCallbackFunc GameWindowManager::Get_Default_System()
{
    return Game_Win_Default_System;
}

WindowCallbackFunc GameWindowManager::Get_Default_Input()
{
    return Game_Win_Default_Input;
}

WindowTooltipFunc GameWindowManager::Get_Default_Tooltip()
{
    return Game_Win_Default_Tooltip;
}
