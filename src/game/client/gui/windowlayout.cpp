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
#include "gamewindowmanager.h"
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
    for (GameWindow *window = m_windowList; window != nullptr; window = window->Win_Get_Next_In_Layout()) {
        window->Win_Hide(hide);
    }

    m_hidden = hide;
}

void WindowLayout::Destroy_Windows()
{
    for (GameWindow *window = m_windowList; window != nullptr; window = m_windowList) {
        Remove_Window(window);
        g_theWindowManager->Win_Destroy(window);
    }
}

void WindowLayout::Bring_Forward()
{
    int count = m_windowCount;
    GameWindow *tail = m_windowTail;

    while (count != 0) {
        captainslog_dbgassert(tail != nullptr, "Must have window: m_windowCount is off");
        GameWindow *prev = tail->Win_Get_Prev_In_Layout();
        tail->Win_Bring_To_Top();
        count--;
        tail = prev;
    }
}

bool WindowLayout::Load(Utf8String filename)
{
    if (filename.Is_Empty()) {
        return false;
    } else {
        WindowLayoutInfo layout;

        if (g_theWindowManager->Win_Create_From_Script(filename, &layout)) {
            for (auto it = layout.m_windowList.begin(); it != layout.m_windowList.end(); it++) {
                Add_Window(*it);
            }

            m_filenameString = filename;
            Set_Init(layout.m_initFunc);
            Set_Update(layout.m_updateFunc);
            Set_Shutdown(layout.m_shutdownFunc);
            return true;
        } else {
            captainslog_dbgassert(false, "WindowLayout::Load - Failed to load layout");
            captainslog_debug("WindowLayout::Load - Unable to load layout file '%s'", filename.Str());
            return false;
        }
    }
}
