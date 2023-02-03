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
#include "windowlayout.h"
#include "gamewindow.h"
#include <captainslog.h>

WindowLayout::WindowLayout() :
    m_filenameString("EmptyLayout"),
    m_windowList(nullptr),
    m_windowTail(nullptr),
    m_windowCount(0),
    m_hidden(false),
    m_initFunc(nullptr),
    m_updateFunc(nullptr),
    m_shutdownFunc(nullptr)
{
}

WindowLayout::~WindowLayout()
{
    captainslog_dbgassert(m_windowList == nullptr, "Window layout being destroyed still has window references");
    captainslog_dbgassert(m_windowTail == nullptr, "Window layout being destroyed still has window references");
}

void WindowLayout::Add_Window(GameWindow *window)
{
    if (Find_Window(window) == nullptr) {

        captainslog_dbgassert(window->Win_Get_Next_In_Layout() == nullptr, "Next_In_Layout should be NULL before adding");
        captainslog_dbgassert(window->Win_Get_Prev_In_Layout() == nullptr, "Prev_In_Layout should be NULL before adding");

        window->Win_Set_Prev_In_Layout(nullptr);
        window->Win_Set_Next_In_Layout(m_windowList);

        if (m_windowList != nullptr) {
            m_windowList->Win_Set_Prev_In_Layout(window);
        }

        m_windowList = window;

        window->Win_Set_Layout(this);

        if (m_windowTail == nullptr) {
            m_windowTail = window;
        }

        ++m_windowCount;
    }
}

void WindowLayout::Remove_Window(GameWindow *window)
{
    window = Find_Window(window);

    if (window != nullptr) {
        GameWindow *prev = window->Win_Get_Prev_In_Layout();
        GameWindow *next = window->Win_Get_Next_In_Layout();

        if (next != nullptr) {
            next->Win_Set_Prev_In_Layout(prev);
        }

        if (prev != nullptr) {
            prev->Win_Set_Next_In_Layout(next);
        } else {
            m_windowList = next;
        }

        window->Win_Set_Layout(nullptr);

        window->Win_Set_Next_In_Layout(nullptr);
        window->Win_Set_Prev_In_Layout(nullptr);

        if (m_windowTail == window) {
            m_windowTail = prev;
        }
        --m_windowCount;
    }
}

GameWindow *WindowLayout::Find_Window(GameWindow *window)
{
    for (GameWindow *i = m_windowList; i != nullptr; i = i->Win_Get_Next_In_Layout()) {

        if (i == window) {
            return i;
        }
    }

    return nullptr;
}

void WindowLayout::Hide(bool hide)
{
#ifdef GAME_DLL
    Call_Method<void, WindowLayout, bool>(PICK_ADDRESS(0x004F83F0, 0x008E4EF4), this, hide);
#endif
}

void WindowLayout::Destroy_Windows()
{
#ifdef GAME_DLL
    Call_Method<void, WindowLayout>(PICK_ADDRESS(0x004F8530, 0x008E50FD), this);
#endif
}

void WindowLayout::Bring_Forward()
{
#ifdef GAME_DLL
    Call_Method<void, WindowLayout>(PICK_ADDRESS(0x004F8900, 0x008E531E), this);
#endif
}
