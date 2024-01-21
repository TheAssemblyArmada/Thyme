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
#pragma once

#include "mouse.h"
#ifdef PLATFORM_WINDOWS
#include <windef.h>
#endif

class Win32Mouse;

#ifdef GAME_DLL
extern Win32Mouse *&g_theWin32Mouse;
#else
extern Win32Mouse *g_theWin32Mouse;
#endif

class Win32Mouse : public Mouse
{
public:
    Win32Mouse();

    virtual ~Win32Mouse() { g_theWin32Mouse = nullptr; };

    virtual void Init() override;
    virtual void Reset() override { Mouse::Reset(); }
    virtual void Update() override { Mouse::Update(); }
    virtual void Init_Cursor_Resources() override;
    virtual void Set_Cursor(MouseCursor cursor) override;
    virtual void Capture() override {}
    virtual void Release_Capture() override {}
    virtual void Set_Visibility(bool visibility) override;
    virtual uint8_t Get_Mouse_Event(MouseIO *io, int8_t unk) override;

    void Add_Win32_Event(uint32_t msg, uint32_t wParam, uint32_t lParam, uint32_t wheel_position);
    void Set_Lost_Focus(bool state) { m_lostFocus = state; }

private:
    void Translate_Event(uint32_t message_num, MouseIO *io);
    void Load_Cursor(const char *file_location, uint32_t load_index, uint32_t load_direction) const;

    struct Win32MouseEvent
    {
        uint32_t msg;
        uint32_t wParam;
        uint32_t lParam;
        uint32_t wheel_position;
    };
#ifdef PLATFORM_WINDOWS
#ifdef GAME_DLL
    static ARRAY2D_DEC(HCURSOR, s_loadedCursors, CURSOR_COUNT, 8);
#else
    static HCURSOR s_loadedCursors[CURSOR_COUNT][8];
#endif
#endif
protected:
    Win32MouseEvent m_eventBuffer[MAX_EVENTS];
    uint32_t m_nextFreeIndex;
    uint32_t m_nextGetIndex;
    uint32_t m_currentWin32Cursor;
    uint32_t m_cursorDirection;
    bool m_lostFocus;
};
