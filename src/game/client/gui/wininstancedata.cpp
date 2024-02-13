/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Information relating to a font to use in game.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "wininstancedata.h"
#include "displaystringmanager.h"
#include <captainslog.h>

WinInstanceData::WinInstanceData() : m_text(nullptr), m_tooltip(nullptr), m_videoBuffer(nullptr)
{
    Init();
}

WinInstanceData::~WinInstanceData()
{
    if (m_text != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_text);
    }
    if (m_tooltip != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_tooltip);
    }
    m_videoBuffer = nullptr;
}

void WinInstanceData::Init()
{
    for (int i = 0; i < MAX_DRAW_DATA; ++i) {
        m_enabledDrawData[i].image = nullptr;
        m_enabledDrawData[i].color = WIN_COLOR_UNDEFINED;
        m_enabledDrawData[i].borderColor = WIN_COLOR_UNDEFINED;

        m_disabledDrawData[i].image = nullptr;
        m_disabledDrawData[i].color = WIN_COLOR_UNDEFINED;
        m_disabledDrawData[i].borderColor = WIN_COLOR_UNDEFINED;

        m_hiliteDrawData[i].image = nullptr;
        m_hiliteDrawData[i].color = WIN_COLOR_UNDEFINED;
        m_hiliteDrawData[i].borderColor = WIN_COLOR_UNDEFINED;
    }

    m_enabledText.color = WIN_COLOR_UNDEFINED;
    m_enabledText.borderColor = WIN_COLOR_UNDEFINED;

    m_disabledText.color = WIN_COLOR_UNDEFINED;
    m_disabledText.borderColor = WIN_COLOR_UNDEFINED;

    m_hiliteText.color = WIN_COLOR_UNDEFINED;
    m_hiliteText.borderColor = WIN_COLOR_UNDEFINED;

    m_id = 0;
    m_state = 0;
    m_style = 0;
    m_status = 0;
    m_owner = nullptr;

    m_textLabelString.Clear();
    m_tooltipString.Clear();

    m_tooltipDelay = -1;

    m_decoratedNameString.Clear();

    m_imageOffset.x = 0;
    m_imageOffset.y = 0;

    m_font = nullptr;

    if (m_text != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_text);
        m_text = nullptr;
    }

    if (m_tooltip != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_tooltip);
        m_tooltip = nullptr;
    }

    m_videoBuffer = nullptr;
}

void WinInstanceData::Set_Text(Utf16String text)
{
    if (m_text == nullptr) {
        m_text = g_theDisplayStringManager->New_Display_String();
    }

    captainslog_dbgassert(m_text != nullptr, "no text");

    m_text->Set_Text(text);
}

Utf16String WinInstanceData::Get_Text()
{
    if (m_text != nullptr) {
        return m_text->Get_Text();
    }

    return Utf16String(Utf16String::s_emptyString);
}

int WinInstanceData::Get_Text_Length()
{
    if (m_text != nullptr) {
        return m_text->Get_Text_Length();
    }

    return 0;
}

void WinInstanceData::Set_Tooltip_Text(Utf16String tip)
{
    if (m_tooltip == nullptr) {
        m_tooltip = g_theDisplayStringManager->New_Display_String();
    }

    captainslog_dbgassert(m_tooltip != nullptr, "no tooltip");

    m_tooltip->Set_Text(tip);
}

Utf16String WinInstanceData::Get_Tooltip_Text()
{
    if (m_tooltip != nullptr) {
        return m_tooltip->Get_Text();
    }

    return Utf16String::s_emptyString;
}

int WinInstanceData::Get_Tooltip_Text_Length()
{
    if (m_tooltip != nullptr) {
        return m_tooltip->Get_Text_Length();
    }

    return 0;
}

void WinInstanceData::Set_VideoBuffer(VideoBuffer *videoBuffer)
{
    m_videoBuffer = videoBuffer;
}

WinInstanceData &WinInstanceData::operator=(const WinInstanceData &that)
{
    m_id = that.m_id;
    m_state = that.m_state;
    m_style = that.m_style;
    m_status = that.m_status;
    m_owner = that.m_owner;

    for (int i = 0; i < MAX_DRAW_DATA; ++i) {
        m_enabledDrawData[i] = that.m_enabledDrawData[i];
        m_disabledDrawData[i] = that.m_disabledDrawData[i];
        m_hiliteDrawData[i] = that.m_hiliteDrawData[i];
    }

    m_enabledText = that.m_enabledText;
    m_disabledText = that.m_disabledText;
    m_hiliteText = that.m_hiliteText;

    m_imeCompositeText = that.m_imeCompositeText;

    m_imageOffset = that.m_imageOffset;

    m_font = that.m_font;

    m_textLabelString = that.m_textLabelString;
    m_decoratedNameString = that.m_decoratedNameString;
    m_tooltipString = that.m_tooltipString;
    m_headerTemplateString = that.m_headerTemplateString;

    m_tooltipDelay = that.m_tooltipDelay;
    m_text = that.m_text;
    m_tooltip = that.m_tooltip;
    m_videoBuffer = that.m_videoBuffer;

    return *this;
}

#ifdef GAME_DLL
#include <new>
WinInstanceData *WinInstanceData::Hook_Ctor()
{
    return new (this) WinInstanceData;
}
#endif
