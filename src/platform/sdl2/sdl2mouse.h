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
#pragma once

#ifndef BUILD_WITH_SDL2
#error Only include sdl2mouse.h when building with SDL2
#endif

#include "always.h"
#include "mouse.h"

#include <SDL.h>

namespace Thyme
{

class SDL2Mouse;

#ifdef GAME_DLL
extern SDL2Mouse *&g_theSDL2Mouse;
#else
extern SDL2Mouse *g_theSDL2Mouse;
#endif

class SDL2Mouse : public Mouse
{
public:
    SDL2Mouse();

    virtual ~SDL2Mouse() override{};

    virtual void Init() override;
    virtual void Init_Cursor_Resources() override;
    virtual void Set_Cursor(MouseCursor cursor) override;
    virtual void Capture() override{};
    virtual void Release_Capture() override{};
    virtual void Set_Visibility(bool visibility) override;
    virtual uint8_t Get_Mouse_Event(MouseIO *io, int8_t unk) override;

    void Add_SDL2_Event(SDL_Event *ev);

private:
    void Translate_Event(uint32_t message_num, MouseIO *io);
    void Load_Cursor(const char *file_location, uint32_t load_index, uint32_t load_direction) const;
    int Get_Double_Click_Time();

protected:
    static SDL_Cursor *s_loadedCursors[CURSOR_COUNT][8];
    SDL_Event m_eventBuffer[MAX_EVENTS];
    uint32_t m_nextFreeIndex;
    uint32_t m_nextGetIndex;
    SDL_Event m_lastClick;
};
} // namespace Thyme
