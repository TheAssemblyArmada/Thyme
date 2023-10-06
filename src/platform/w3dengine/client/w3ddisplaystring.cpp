/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Display String
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddisplaystring.h"
#include "gameclient.h"
#include "globallanguage.h"
#include "hotkey.h"

W3DDisplayString::W3DDisplayString() :
    m_textChanged(false),
    m_fontChanged(false),
    m_hotKeyState(false),
    m_xPos(0),
    m_yPos(0),
    m_hotKeyColor(0),
    m_x(0),
    m_y(0),
    m_textColor(0),
    m_borderColor(0),
    m_xExtent(0),
    m_yExtent(0),
    m_frame(0)
{
    m_clipRegion.lo.x = 0;
    m_clipRegion.lo.y = 0;
    m_clipRegion.hi.x = 0;
    m_clipRegion.hi.y = 0;
}

void W3DDisplayString::Notify_Text_Changed()
{
    if (g_theGlobalLanguage != nullptr) {
        if (g_theGlobalLanguage->m_useHardWordWrap) {
            m_sentence.m_partialWords = true;
            m_hotKeySentence.m_partialWords = true;
        } else {
            m_sentence.m_partialWords = false;
            m_hotKeySentence.m_partialWords = false;
        }
    }

    Compute_Extents();
    m_textChanged = true;
    m_sentence.Reset();
    m_hotKeySentence.Reset();
}

void W3DDisplayString::Set_Font(GameFont *font)
{
    if (font != nullptr && font != m_font) {
        m_font = font;
        m_sentence.Set_Font(font->m_fontData);
        m_hotKeySentence.Set_Font(g_theFontLibrary->Get_Font(font->m_nameString, font->m_pointSize, true)->m_fontData);
        Compute_Extents();
        m_fontChanged = true;
    }
}

void W3DDisplayString::Set_Word_Wrap(int wrap_width)
{
    if (wrap_width != m_sentence.m_wrapWidth) {
        m_sentence.m_wrapWidth = wrap_width;
        Notify_Text_Changed();
    }
}

void W3DDisplayString::Set_Word_Wrap_Centered(bool on)
{
    if (on != m_sentence.m_centered) {
        m_sentence.m_centered = on;
        Notify_Text_Changed();
    }
}

void W3DDisplayString::Draw(int x, int y, int color, int border_color)
{
    Draw(x, y, color, border_color, 1, 1);
}

void W3DDisplayString::Draw(int x, int y, int color, int border_color, int border_x_offset, int border_y_offset)
{
    if (Get_Text_Length() != 0) {
        if (m_textChanged || m_fontChanged) {
            if (m_hotKeyState) {
                m_sentence.m_processAmpersand = true;
                m_sentence.Build_Sentence(Get_Text().Str(), &m_xPos, &m_yPos);
                m_hotKeyString.Translate(g_theHotKeyManager->Search_Hot_Key(Get_Text()));

                if (m_hotKeyString.Is_Empty()) {
                    m_hotKeyState = false;
                    m_hotKeySentence.Reset();
                } else {
                    m_hotKeySentence.Build_Sentence(m_hotKeyString.Str());
                }
            } else {
                m_sentence.Build_Sentence(Get_Text().Str());
            }

            m_fontChanged = false;
            m_textChanged = false;
        } else if (x == m_x && y == m_y && color == m_textColor && border_color == m_borderColor) {
            m_sentence.Render();

            if (g_theGameClient != nullptr) {
                m_frame = g_theGameClient->Get_Frame();
            }

            return;
        }

        m_x = x;
        m_y = y;
        m_textColor = color;
        m_borderColor = border_color;
        m_sentence.Reset_Polys();
        m_sentence.Set_Location(Vector2(m_x + border_x_offset, m_y + border_y_offset));
        m_sentence.Draw_Sentence(m_borderColor);
        m_sentence.Set_Location(Vector2(m_x, m_y));
        m_sentence.Draw_Sentence(m_textColor);

        if (m_hotKeyState) {
            m_hotKeySentence.Reset_Polys();
            m_hotKeySentence.Set_Location(Vector2(m_xPos + m_x + 1, m_yPos + m_y));
            m_hotKeySentence.Draw_Sentence(m_hotKeyColor);
            m_hotKeySentence.Render();
        }

        m_sentence.Render();

        if (g_theGameClient != nullptr) {
            m_frame = g_theGameClient->Get_Frame();
        }
    }
}

void W3DDisplayString::Get_Size(int *x, int *y)
{
    if (x != nullptr) {
        *x = m_xExtent;
    }

    if (y != nullptr) {
        *y = m_yExtent;
    }
}

int W3DDisplayString::Get_Width(int char_count)
{
    FontCharsClass *font = m_sentence.m_font;
    int width = 0;
    int i = 0;

    if (font != nullptr) {
        const unichar_t *str = m_textString.Str();
        unichar_t c = *str;

        for (const unichar_t *next = str + 1; c != '\0'; next++) {
            if (char_count != -1 && i >= char_count) {
                break;
            }

            if (c != '\n') {
                width += font->Get_Char_Spacing(c);
            }

            c = *next;
            i++;
        }
    }

    return width;
}

void W3DDisplayString::Set_Use_Hotkey(bool state, int color)
{
    m_hotKeyState = state;
    m_hotKeyColor = color;
    m_sentence.m_processAmpersand = true;
    Notify_Text_Changed();
}

void W3DDisplayString::Set_Clip_Region(IRegion2D *region)
{
    if (region->lo.x != m_clipRegion.lo.x || region->lo.y != m_clipRegion.lo.y || region->hi.x != m_clipRegion.hi.x
        || region->hi.y != m_clipRegion.hi.y) {
        m_clipRegion = *region;
        m_sentence.Set_Clipping_Rect(RectClass(m_clipRegion.lo.x, m_clipRegion.lo.y, m_clipRegion.hi.x, m_clipRegion.hi.y));
        m_hotKeySentence.Set_Clipping_Rect(
            RectClass(m_clipRegion.lo.x, m_clipRegion.lo.y, m_clipRegion.hi.x, m_clipRegion.hi.y));
    }
}

void W3DDisplayString::Compute_Extents()
{
    if (Get_Text_Length() != 0 && m_font != nullptr) {
        Vector2 extents = m_sentence.Get_Formatted_Text_Extents(Get_Text().Str());
        m_xExtent = extents.X;
        m_yExtent = extents.Y;
    } else {
        m_xExtent = 0;
        m_yExtent = 0;
    }
}
