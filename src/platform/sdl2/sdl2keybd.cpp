/**
 * @file
 *
 * @author feliwir
 *
 * @brief SDL2 implementation of the Keyboard interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "sdl2keybd.h"
#include "main.h"
#include <captainslog.h>

namespace Thyme
{
#ifndef GAME_DLL
SDL2InputKeyboard *g_theSDL2Keyboard = nullptr;
#endif

SDL2InputKeyboard::SDL2InputKeyboard() {}

SDL2InputKeyboard::~SDL2InputKeyboard() {}

/**
 * Initialise the subsystem.
 */
void SDL2InputKeyboard::Init()
{
    Keyboard::Init();
}

/**
 * Checks if the caps lock is on.
 */
bool SDL2InputKeyboard::Get_Caps_State()
{
    SDL_Keymod mod = SDL_GetModState();
    return mod & KMOD_CAPS;
}

void SDL2InputKeyboard::Add_SDL2_Event(SDL_Event *ev)
{
    auto &event = m_eventBuffer[m_nextFreeIndex];

    // If no space (0 type == empty) for more events throw away event
    if (event.type != SDL_FIRSTEVENT) {
        return;
    }

    event = *ev;

    m_nextFreeIndex++;
    if (m_nextFreeIndex >= MAX_EVENTS) {
        m_nextFreeIndex = 0;
    }
}

/**
 * Gets key state.
 */
void SDL2InputKeyboard::Get_Key(KeyboardIO *io)
{
    if (m_eventBuffer[m_nextGetIndex].type == 0) {
        return;
    }

    // Translate_Event(m_nextGetIndex, io);

    // Clear the current event - can be overwritten
    m_eventBuffer[m_nextGetIndex].type = SDL_FIRSTEVENT;
    m_nextGetIndex++;
    if (m_nextGetIndex >= MAX_EVENTS) {
        m_nextGetIndex = 0;
    }
}
} // namespace Thyme
