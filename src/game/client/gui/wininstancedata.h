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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "unicodestring.h"

class Image;
class GameWindow;
class DisplayString;
class VideoBuffer;

class GameFont;

struct WinDrawData
{
    const Image *image;
    int color;
    int borderColor;
};

struct TextDrawData
{
    int color;
    int borderColor;
};

class WinInstanceData
{
public:
    enum
    {
        MAX_DRAW_DATA = 9,
        WIN_COLOR_UNDEFINED = 0xFFFFFF,
    };

    WinInstanceData();
    virtual ~WinInstanceData();

    void Init();

    void Set_Text(Utf16String text);
    Utf16String Get_Text();
    int Get_Text_Length();
    DisplayString *Get_Text_DisplayString() { return m_text; }

    void Set_Tooltip_Text(Utf16String tip);
    Utf16String Get_Tooltip_Text();
    int Get_Tooltip_Text_Length();
    DisplayString *Get_Tooltip_DisplayString() { return m_tooltip; }

    unsigned Get_State() { return m_state; }
    unsigned Get_Style() { return m_style; }
    unsigned Get_Status() { return m_status; }
    GameWindow *Get_Owner() { return m_owner; }
    GameFont *Get_Font() { return m_font; }
    void Set_VideoBuffer(VideoBuffer *videoBuffer);

    WinInstanceData &operator=(const WinInstanceData &that);

#ifdef GAME_DLL
    WinInstanceData *Hook_Ctor();
#endif

public:
    int m_id;
    int m_state;
    unsigned m_style;
    unsigned m_status;
    GameWindow *m_owner;
    WinDrawData m_enabledDrawData[MAX_DRAW_DATA];
    WinDrawData m_disabledDrawData[MAX_DRAW_DATA];
    WinDrawData m_hiliteDrawData[MAX_DRAW_DATA];
    TextDrawData m_enabledText;
    TextDrawData m_disabledText;
    TextDrawData m_hiliteText;
    TextDrawData m_imeCompositeText;
    ICoord2D m_imageOffset;
    GameFont *m_font;
    Utf8String m_textLabelString;
    Utf8String m_decoratedNameString;
    Utf8String m_tooltipString;
    Utf8String m_headerTemplateString;
    int m_tooltipDelay;
    DisplayString *m_text;
    DisplayString *m_tooltip;
    VideoBuffer *m_videoBuffer;
};
