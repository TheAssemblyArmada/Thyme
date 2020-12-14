/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Font information and handling classes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamefont.h"

#ifndef GAME_DLL
FontLibrary *g_theFontLibrary;
#endif

FontLibrary::FontLibrary() : m_fontList(nullptr), m_count(0) {}

FontLibrary::~FontLibrary()
{
    Delete_All_Fonts();
}

void FontLibrary::Reset()
{
    Delete_All_Fonts();
}

void FontLibrary::Link_Font(GameFont *font)
{
    if (font != nullptr) {
        font->m_next = m_fontList;
        m_fontList = font;
        ++m_count;
    }
}

void FontLibrary::Unlink_Font(GameFont *font)
{
    if (font == nullptr) {
        return;
    }

    GameFont *i = m_fontList;
    while (i != nullptr) {
        if (i == font) {
            break;
        }
        i = i->m_next;
    }

    if (i == nullptr) {
        captainslog_fatal("Font '%s' not found in library", font->m_nameString.Str());
        return;
    }

    i = m_fontList;
    while (i != nullptr) {
        if (i->m_next == font) {
            break;
        }

        i = i->m_next;
    }

    if (i == nullptr) {
        m_fontList = font->m_next;
    } else {
        i->m_next = font->m_next;
    }

    font->m_next = nullptr;
    --m_count;
}

void FontLibrary::Delete_All_Fonts()
{
    while (m_fontList != nullptr) {
        GameFont *fnt = m_fontList;
        Unlink_Font(m_fontList);
        Release_Font_Data(fnt);
        fnt->Delete_Instance();
    }
}

GameFont *FontLibrary::Get_Font(Utf8String name, int point_size, bool bold)
{
    GameFont *font = nullptr;

    for (font = m_fontList; font != nullptr; font = font->m_next) {
        if (font->m_nameString == name && font->m_pointSize == point_size && font->m_bold == bold) {
            return font;
        }
    }

    font = NEW_POOL_OBJ(GameFont);
    if (font == nullptr) {
        captainslog_fatal("Unable to allocate new font list element");
        return nullptr;
    }

    font->m_nameString = name;
    font->m_pointSize = point_size;
    font->m_bold = bold;
    font->m_fontData = nullptr;

    captainslog_debug("Loading font '%s' with %d point size", name.Str(), point_size);
    if (!Load_Font_Data(font)) {
        captainslog_fatal("Unable to load font data pointer '%s'", name.Str());
        font->Delete_Instance();
        return nullptr;
    }

    Link_Font(font);
    return font;
}
