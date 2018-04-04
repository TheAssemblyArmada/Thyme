/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for strings to be displayed.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "coord.h"
#include "gamefont.h"
#include "mempoolobj.h"
#include "unicodestring.h"

class DisplayString : public MemoryPoolObject
{
    IMPLEMENT_POOL(DisplayString);
    friend class DisplayStringManager;

public:
    DisplayString();
    virtual ~DisplayString();

    virtual void Set_Text(UnicodeString text);
    virtual UnicodeString Get_Text() { return m_textString; }
    virtual int Get_Text_Length() { return m_textString.Get_Length(); }
    virtual void Notify_Text_Changed() {}
    virtual void Reset();
    virtual void Set_Font(GameFont *font) { m_font = font; }
    virtual GameFont *Get_Font() { return m_font; }
    virtual void Set_Word_Wrap(int length) = 0;
    virtual void Set_Word_Wrap_Centered(char unk) = 0;
    virtual void Draw(int x, int y, int w, int h, int unk1, int unk2) = 0;
    virtual void Draw(int x, int y, int w, int h) = 0;
    virtual void Get_Size(int *x, int *y) = 0;
    virtual int Get_Width(int char_count) = 0;
    virtual void Set_Clip_Region(IRegion2D *region) = 0;
    virtual void Remove_Last_Char();
    virtual void Add_Char(wchar_t ch);

protected:
    UnicodeString m_textString;
    GameFont *m_font;
    DisplayString *m_next;
    DisplayString *m_prev;
};