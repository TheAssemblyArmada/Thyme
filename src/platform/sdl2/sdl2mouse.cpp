/**
 * @file
 *
 * @author feliwir
 *
 * @brief SDL2 Mouse Input interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "sdl2mouse.h"
#include "editmain.h"
#include "gameclient.h"
#include "globaldata.h"
#include <cstdio>

namespace Thyme
{
SDL_Cursor *SDL2Mouse::s_loadedCursors[CURSOR_COUNT][8];

#ifndef GAME_DLL
SDL2Mouse *g_theSDL2Mouse = nullptr;
#endif

SDL2Mouse::SDL2Mouse() : m_nextFreeIndex(0), m_nextGetIndex(0)
{
    std::memset(m_eventBuffer, 0, sizeof(m_eventBuffer));
    std::memset(s_loadedCursors, 0, sizeof(s_loadedCursors));
    std::memset(&m_lastClick, 0, sizeof(m_lastClick));
}

void SDL2Mouse::Init()
{
    Mouse::Init();
}

void SDL2Mouse::Load_Cursor(const char *file_location, uint32_t load_index, uint32_t load_direction) const
{
    if (s_loadedCursors[load_index][load_direction] != NULL) {
        // Cursor at this position already loaded. Ignoring subsequent load.
        return;
    }
    // TODO: this should parse the ANI file and load the cursor correctly
    SDL_Surface *surface = SDL_LoadBMP(file_location);
    if (!surface) {
        // Failed to load surface
        captainslog_error("Load_Cursor: Failed to load ANI cursor [%s]", file_location);
        return;
    }
    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 0);
    if (!cursor) {
        // Failed to create cursor
        captainslog_error("Load_Cursor: Failed to create SDL2 color cursor [%s]", file_location);
        return;
    }
    s_loadedCursors[load_index][load_direction] = cursor;
}

void SDL2Mouse::Init_Cursor_Resources()
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

void SDL2Mouse::Set_Cursor(MouseCursor cursor)
{
    Set_Mouse_Text(cursor);
    m_currentCursor = cursor;
}

void SDL2Mouse::Set_Visibility(bool visibility)
{
    m_visible = visibility;
    SDL_ShowCursor(visibility);
}

void SDL2Mouse::Add_SDL2_Event(SDL_Event *ev)
{
    auto &event = m_eventBuffer[m_nextFreeIndex];

    // If no space (0 type == empty) for more events throw away event
    if (event.type != 0) {
        return;
    }

    event = *ev;

    m_nextFreeIndex++;
    if (m_nextFreeIndex >= MAX_EVENTS) {
        m_nextFreeIndex = 0;
    }
}

uint8_t SDL2Mouse::Get_Mouse_Event(MouseIO *io, int8_t unk)
{
    if (m_eventBuffer[m_nextGetIndex].type == SDL_FIRSTEVENT) {
        return false;
    }

    Translate_Event(m_nextGetIndex, io);

    // Clear the current event - can be overwritten
    m_eventBuffer[m_nextGetIndex].type = SDL_FIRSTEVENT;
    m_nextGetIndex++;
    if (m_nextGetIndex >= MAX_EVENTS) {
        m_nextGetIndex = 0;
    }
    return true;
}

int SDL2Mouse::Get_Double_Click_Time()
{
    return g_theWriteableGlobalData->m_doubleClickTime;
}

void SDL2Mouse::Translate_Event(uint32_t message_num, MouseIO *io)
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

    MouseIO::MouseState state;
    bool doubleClick = false;

    // TODO: hack double click
    switch (event.type) {
        case SDL_MOUSEMOTION:
            io->pos.x = event.motion.x;
            io->pos.y = event.motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            // Check if this is a double click
            doubleClick = event.button.button == m_lastClick.button.button
                && (event.button.timestamp - m_lastClick.button.timestamp) < Get_Double_Click_Time();
            m_lastClick = event;
        case SDL_MOUSEBUTTONUP:
            state = event.type == SDL_MOUSEBUTTONDOWN ? MouseIO::MouseState::MOUSE_STATE_DOWN :
                                                        MouseIO::MouseState::MOUSE_STATE_UP;
            if (doubleClick) {
                // It would be useful if we could just OR this flag
                state = MouseIO::MouseState::MOUSE_STATE_DBLCLICK;
            }

            io->pos.x = event.button.x;
            io->pos.y = event.button.y;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    io->left_state = state;
                    io->left_frame = frame_num;
                    break;
                case SDL_BUTTON_RIGHT:
                    io->right_state = state;
                    io->right_frame = frame_num;
                    break;
                case SDL_BUTTON_MIDDLE:
                    io->middle_state = state;
                    io->middle_frame = frame_num;
                    break;
                default:
                    captainslog_debug(
                        "Translate_Event: Unknown SDL mouse button event [%d,%d]", event.type, event.button.button);
                    break;
            }
            break;
        // TODO: mousewheel handling is a bit difficult - how to calculate a delta similar to win32mouse?
        case SDL_MOUSEWHEEL:
            io->wheel_delta = event.wheel.y;
            // TODO: fill x & y
            break;
        default:
            captainslog_debug("Translate_Event: Unknown SDL mouse event [%d]", event.type);
            break;
    }
}
} // namespace Thyme
