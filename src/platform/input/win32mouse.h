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

class Win32Mouse : public Mouse
{
public:
    Win32Mouse();

    virtual ~Win32Mouse() = default;

    virtual void Init() override;
    virtual void Init_Cursor_Resources() override;
    virtual void Set_Cursor(MouseCursor cursor) override;
    virtual void Capture() override{};
    virtual void Release_Capture() override{};
    virtual void Set_Visibility(bool visibility) override;
    virtual bool Get_Mouse_Event(MouseIO *io, int8_t unk) override;

    void Add_Win32_Event(uint32_t msg, uint32_t wParam, uint32_t lParam, uint32_t wheel_position);

private:
    void Translate_Event(uint32_t message_num, MouseIO *io);

    struct Win32MouseEvent
    {
        uint32_t msg;
        uint32_t wParam;
        uint32_t lParam;
        uint32_t wheel_position;
    };

protected:
    Win32MouseEvent m_eventBuffer[MAX_EVENTS];
    uint32_t m_nextFreeIndex;
    uint32_t m_nextGetIndex;
    uint32_t m_currentWin32Cursor;
    uint32_t m_cursorDirection;
    bool m_unknownbool;
};
