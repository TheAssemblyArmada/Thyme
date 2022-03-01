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
#pragma once

#ifndef BUILD_WITH_SDL2
#error Only include sdl2keybd.h when building with SDL2
#endif

#include "always.h"
#include "keyboard.h"
#include <new>

#include <SDL.h>

namespace Thyme
{

class SDL2InputKeyboard;
#ifdef GAME_DLL
extern SDL2InputKeyboard *&g_theSDL2Keyboard;
#else
extern SDL2InputKeyboard *g_theSDL2Keyboard;
#endif

class SDL2InputKeyboard : public Keyboard
{
    enum
    {
        MAX_EVENTS = 256,
    };

public:
    SDL2InputKeyboard();
    virtual ~SDL2InputKeyboard();

    virtual void Init() override;

    virtual bool Get_Caps_State() override;
    virtual void Get_Key(KeyboardIO *io) override;

    void Add_SDL2_Event(SDL_Event *ev);

private:
    SDL_Event m_eventBuffer[MAX_EVENTS];
    uint32_t m_nextFreeIndex;
    uint32_t m_nextGetIndex;
};
} // namespace Thyme
