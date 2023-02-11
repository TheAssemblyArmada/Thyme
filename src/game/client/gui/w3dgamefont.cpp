/**
 * @file
 *
 * @author tomsons26
 *
 * @brief W3D Font information and handling class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dgamefont.h"
#include "assetmgr.h"
#include "globallanguage.h"
#include "render2dsentence.h"

bool W3DFontLibrary::Load_Font_Data(GameFont *font)
{
    if (font == nullptr) {
        return false;
    }

    FontCharsClass *font_chars;
    FontCharsClass *alt_font_chars;

    if (font->m_pointSize > 100) {
        font_chars = nullptr;
    } else {
        font_chars =
            W3DAssetManager::Get_Instance()->Get_FontChars(font->m_nameString.Str(), font->m_pointSize, font->m_bold);
    }

    if (font_chars == nullptr) {
        captainslog_error("W3D load font: unable to find font '%s' from asset manager", font->m_nameString.Str());
        captainslog_dbgassert(0, "Missing or Corrupted Font.  Please see log for details");
        return false;
    }

    font->m_fontData = font_chars;
    font->m_height = font_chars->Get_Char_Height();

    if (g_theGlobalLanguage != nullptr) {
        alt_font_chars = W3DAssetManager::Get_Instance()->Get_FontChars(
            g_theGlobalLanguage->m_unicodeFontName.Str(), font->m_pointSize, font->m_bold);
    } else {
        alt_font_chars = W3DAssetManager::Get_Instance()->Get_FontChars("Arial Unicode MS", font->m_pointSize, font->m_bold);
    }

    if (alt_font_chars != nullptr) {
        font_chars->m_alternateUnicodeFont = alt_font_chars;
    }

    return true;
}

void W3DFontLibrary::Release_Font_Data(GameFont *font)
{
    if (font != nullptr) {
        Ref_Ptr_Release(font->m_fontData->m_alternateUnicodeFont);
        Ref_Ptr_Release(font->m_fontData);
    }
}
