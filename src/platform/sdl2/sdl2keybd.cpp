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
#include <SDL.h>
#include <captainslog.h>

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

/**
 * Gets key state.
 */
void SDL2InputKeyboard::Get_Key(KeyboardIO *io)
{
    io->key = 0;
    io->sequence = 0;

    // TODO: use an event query
    const uint8_t *state = SDL_GetKeyboardState(NULL);
}
