/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberium Technologies
 *
 * @brief Text rendering classes.
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
#include "refcount.h"
#include "vector.h"
#include "w3dmpo.h"
#include "wwstring.h"
#include <new>

constexpr unsigned CHAR_BUFFER_LEN = 0x8000;

struct FontCharsBuffer
{
    uint16_t buffer[CHAR_BUFFER_LEN];
    IMPLEMENT_W3D_POOL(FontCharsBuffer)
};

class FontCharsClass : public W3DMPO, public RefCountClass
{
    ALLOW_HOOKING
    IMPLEMENT_W3D_POOL(FontCharsClass)

    struct CharDataStruct : public W3DMPO
    {
        unichar_t value;
        int16_t width;
        uint16_t *buffer;
        IMPLEMENT_NAMED_W3D_POOL(CharDataStruct, FontCharsClassCharDataStruct)
    };

public:
    FontCharsClass();
    virtual ~FontCharsClass();

    void Initialize_GDI_Font(const char *font_name, int point_size, bool is_bold);
    bool Is_Font(const char *font_name, int point_size, bool is_bold);
    const char *Get_Name() { return m_name; }
    int Get_Char_Height() { return m_charHeight; }
    int Get_Char_Width(unichar_t ch);
    int Get_Char_Spacing(unichar_t ch);
    void Blit_Char(unichar_t ch, uint16_t *dest_ptr, int dest_stride, int x, int y);

#ifdef GAME_DLL
    FontCharsClass *Hook_Ctor() { return new (this) FontCharsClass; }
    void Hook_Dtor() { FontCharsClass::~FontCharsClass(); }
#endif

private:
    void Create_GDI_Font(const char *font_name);
    void Free_GDI_Font();
    const CharDataStruct *Store_GDI_Char(unichar_t ch);
    void Update_Current_Buffer(int char_width);
    const CharDataStruct *Get_Char_Data(unichar_t ch);
    void Grow_Unicode_Array(unichar_t ch);
    void Free_Character_Arrays();

private:
    FontCharsClass *m_alternateUnicodeFont;
    StringClass m_name;
    DynamicVectorClass<FontCharsBuffer *> m_bufferList;
    int m_currPixelOffset;
    int m_charHeight;
    int m_ascent;
    int m_overhang;
    int m_widthReduction;
    int m_pointSize;
    StringClass m_gdiFontName;
#ifdef PLATFORM_WINDOWS
    HFONT m_oldGDIFont;
    HBITMAP m_oldGDIBitmap;
    HBITMAP m_gdiBitmap;
    HFONT m_gdiFont;
    uint8_t *m_gdiBitmapBits;
    HDC m_memDC;
#endif
    FontCharsClass::CharDataStruct *m_asciiCharArray[256];
    FontCharsClass::CharDataStruct **m_unicodeCharArray;
    unichar_t m_firstUnicodeChar;
    unichar_t m_lastUnicodeChar;
    bool m_isBold;
};
