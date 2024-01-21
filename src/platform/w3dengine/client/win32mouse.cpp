/**
 * @file
 *
 * @author DuncansPumpkin
 *
 * @brief Win32 Mouse Input interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "win32mouse.h"
#include "editmain.h"
#include "gameclient.h"
#include <cstdio>

#ifdef PLATFORM_WINDOWS
#include <windowsx.h>
#include <winuser.h>
#endif

#ifndef GAME_DLL
#ifdef PLATFORM_WINDOWS
HCURSOR Win32Mouse::s_loadedCursors[CURSOR_COUNT][8];
#endif
Win32Mouse *g_theWin32Mouse = nullptr;
#endif

// 0x0073C0A0
Win32Mouse::Win32Mouse() :
    m_nextFreeIndex(0), m_nextGetIndex(0), m_currentWin32Cursor(0), m_cursorDirection(0), m_lostFocus(false)
{
    std::memset(m_eventBuffer, 0, sizeof(m_eventBuffer));
#ifdef PLATFORM_WINDOWS
    std::memset(s_loadedCursors, 0, sizeof(s_loadedCursors));
#endif
}

// 0x0073C130
void Win32Mouse::Init()
{
    Mouse::Init();
    m_inputMovesAbsolute = 1;
}

void Win32Mouse::Load_Cursor(const char *file_location, uint32_t load_index, uint32_t load_direction) const
{
#ifdef PLATFORM_WINDOWS
    if (s_loadedCursors[load_index][load_direction] != NULL) {
        // Cursor at this position already loaded. Ignoring subsequent load.
        return;
    }
    s_loadedCursors[load_index][load_direction] = LoadCursorFromFileA(file_location);
#endif
}

// 0x0073C260
void Win32Mouse::Init_Cursor_Resources()
{
    // First cursor is not loaded. As this is CURSOR_NONE
    for (auto i = 1; i < CURSOR_COUNT; ++i) {
        auto &cursor_info = m_cursorInfo[i];
        if (cursor_info.texture_name.Is_Empty()) {
            continue;
        }

        if (cursor_info.directions <= 1) {
            char buffer[256]{};
            snprintf(buffer, 256, "Data/Cursors/%s.ani", cursor_info.texture_name.Str());
            Load_Cursor(buffer, i, 0);
        } else {
            for (auto direction = 0; direction < cursor_info.directions; direction++) {
                char buffer[256]{};
                snprintf(buffer, 256, "Data/Cursors/%s%d.ani", cursor_info.texture_name.Str(), direction);
                Load_Cursor(buffer, i, direction);
            }
        }
    }
}

// 0x0073C320
void Win32Mouse::Set_Cursor(MouseCursor cursor)
{
    Set_Mouse_Text(cursor);

    if (m_lostFocus == false) {
#ifdef PLATFORM_WINDOWS
        HCURSOR hCursor = NULL;
        if (cursor != MouseCursor::CURSOR_NONE && m_visible != false) {
            hCursor = s_loadedCursors[cursor][m_cursorDirection];
        }
        SetCursor(hCursor);
#endif
        m_currentCursor = cursor;
        m_currentWin32Cursor = cursor;
    }
}

// 0x0073C200
void Win32Mouse::Set_Visibility(bool visibility)
{
    m_visible = visibility;
    Set_Cursor(m_currentCursor);
}

// 0x0073BE20
uint8_t Win32Mouse::Get_Mouse_Event(MouseIO *io, int8_t unk)
{
    if (m_eventBuffer[m_nextGetIndex].msg == 0) {
        return false;
    }

    Translate_Event(m_nextGetIndex, io);

    m_eventBuffer[m_nextGetIndex].msg = 0;
    m_nextGetIndex++;
    if (m_nextGetIndex >= MAX_EVENTS) {
        m_nextGetIndex = 0;
    }

    return true;
}

// 0x0073C170
void Win32Mouse::Add_Win32_Event(uint32_t msg, uint32_t wParam, uint32_t lParam, uint32_t wheel_position)
{
    auto &event = m_eventBuffer[m_nextFreeIndex];

    // If no space (0 msg == empty) for more events throw away event
    if (event.msg != 0) {
        return;
    }

    event.msg = msg;
    event.wParam = wParam;
    event.lParam = lParam;
    event.wheel_position = wheel_position;

    m_nextFreeIndex++;
    if (m_nextFreeIndex >= MAX_EVENTS) {
        m_nextFreeIndex = 0;
    }
}

// 0x0073BE90
void Win32Mouse::Translate_Event(uint32_t message_num, MouseIO *io)
{
    auto &event = m_eventBuffer[message_num];

    uint32_t frame_num = 1;
    if (g_theGameClient != nullptr) {
        frame_num = g_theGameClient->Get_Frame();
    }

    io->right_state = 0;
    io->middle_state = 0;
    io->left_state = 0;
    io->right_frame = 0;
    io->middle_frame = 0;
    io->left_frame = 0;
    io->wheel_delta = 0;
    io->pos.y = 0;
    io->pos.x = 0;
    io->wheel_pos = event.wheel_position;
#ifdef PLATFORM_WINDOWS
    switch (event.msg) {
        case WM_MOUSEMOVE:
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_LBUTTONDOWN:
            io->left_state = MouseIO::MouseState::MOUSE_STATE_DOWN;
            io->left_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_LBUTTONUP:
            io->left_state = MouseIO::MouseState::MOUSE_STATE_UP;
            io->left_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_LBUTTONDBLCLK:
            io->left_state = MouseIO::MouseState::MOUSE_STATE_DBLCLICK;
            io->left_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_RBUTTONDOWN:
            io->right_state = MouseIO::MouseState::MOUSE_STATE_DOWN;
            io->right_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_RBUTTONUP:
            io->right_state = MouseIO::MouseState::MOUSE_STATE_UP;
            io->right_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_RBUTTONDBLCLK:
            io->right_state = MouseIO::MouseState::MOUSE_STATE_DBLCLICK;
            io->right_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_MBUTTONDOWN:
            io->middle_state = MouseIO::MouseState::MOUSE_STATE_DOWN;
            io->middle_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_MBUTTONUP:
            io->middle_state = MouseIO::MouseState::MOUSE_STATE_UP;
            io->middle_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_MBUTTONDBLCLK:
            io->middle_state = MouseIO::MouseState::MOUSE_STATE_DBLCLICK;
            io->middle_frame = frame_num;
            io->pos.x = GET_X_LPARAM(event.lParam);
            io->pos.y = GET_Y_LPARAM(event.lParam);
            break;
        case WM_MOUSEWHEEL: {
            POINT p = { GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) };
            ScreenToClient(g_applicationHWnd, &p);
            io->wheel_delta = GET_WHEEL_DELTA_WPARAM(event.wParam);
            io->pos.x = p.x;
            io->pos.y = p.y;
            break;
        }
        default:
            captainslog_debug(
                "Translate_Event: Unknown Win32 mouse event [%d,%d,%d]\n", event.msg, event.wParam, event.lParam);
            break;
    }
#endif
}
