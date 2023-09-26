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

#ifndef GAME_DLL
GameWindowManager *g_theWindowManager;
#endif

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
