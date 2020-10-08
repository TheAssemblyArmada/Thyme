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
#include "render2dsentence.h"
#include "w3d.h"
#include <algorithm>
#include <captainslog.h>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

FontCharsClass::FontCharsClass() :
    m_alternateUnicodeFont(nullptr),
    m_currPixelOffset(0),
    m_charHeight(0),
    m_ascent(0),
    m_overhang(0),
    m_widthReduction(0),
    m_pointSize(0),
#ifdef PLATFORM_WINDOWS
    m_oldGDIFont(nullptr),
    m_oldGDIBitmap(nullptr),
    m_gdiBitmap(nullptr),
    m_gdiFont(nullptr),
    m_gdiBitmapBits(nullptr),
    m_memDC(nullptr),
#endif
    m_unicodeCharArray(nullptr),
    m_firstUnicodeChar(0xFFFF),
    m_lastUnicodeChar(0),
    m_isBold(false)
{
    std::memset(m_asciiCharArray, 0, sizeof(m_asciiCharArray));
}

FontCharsClass::~FontCharsClass()
{
    for (int i = 0; i < m_bufferList.Count(); ++i) {
        delete m_bufferList[i];
    }

    m_bufferList.Reset_Active();

    Free_GDI_Font();
    Free_Character_Arrays();
}

const FontCharsClass::CharDataStruct *FontCharsClass::Get_Char_Data(unichar_t ch)
{
    const CharDataStruct *retval = nullptr;

    if (ch < 256) {
        retval = m_asciiCharArray[ch];
    } else {
        FontCharsClass *unicode_alt = this;

        while (unicode_alt->m_alternateUnicodeFont != nullptr && unicode_alt->m_alternateUnicodeFont != this) {
            unicode_alt = unicode_alt->m_alternateUnicodeFont;
        }

        unicode_alt->Grow_Unicode_Array(ch);
        retval = m_unicodeCharArray[ch - m_firstUnicodeChar];
    }

    if (retval == nullptr) {
        retval = Store_GDI_Char(ch);
    }

    captainslog_assert(retval->value == ch);

    return retval;
}

int FontCharsClass::Get_Char_Width(unichar_t ch)
{
    const CharDataStruct *data = Get_Char_Data(ch);

    if (data != nullptr) {
        return data->width;
    }

    return 0;
}

int FontCharsClass::Get_Char_Spacing(unichar_t ch)
{
    const CharDataStruct *data = Get_Char_Data(ch);

    if (data != nullptr && data->width != 0) {
        return data->width - m_widthReduction - m_overhang;
    }

    return 0;
}

void FontCharsClass::Blit_Char(unichar_t ch, uint16_t *dest_ptr, int dest_stride, int x, int y)
{
    const CharDataStruct *data = Get_Char_Data(ch);
    if (data != nullptr && data->width != 0) {
        int dest_inc = (dest_stride >> 1);
        uint16_t *src_ptr = data->buffer;
        dest_ptr += (dest_inc * y) + x;

        for (int row = 0; row < m_charHeight; ++row) {
            for (int col = 0; col < data->width; ++col) {
                uint16_t tmp = *src_ptr++;

                if (col < m_widthReduction) {
                    tmp |= dest_ptr[col];
                }

                dest_ptr[col] = tmp;
            }

            dest_ptr += dest_inc;
        }
    }
}

const FontCharsClass::CharDataStruct *FontCharsClass::Store_GDI_Char(unichar_t ch)
{
    int width = m_pointSize * 2;
    int height = m_pointSize * 2;
    int x_pos = 0;
#ifdef PLATFORM_WINDOWS
    RECT rect = { 0, 0, width, height };

    if (ch == 'W') {
        x_pos = 1;
    }

    ExtTextOutW(m_memDC, x_pos, 0, ETO_OPAQUE, &rect, &ch, 1, nullptr);
    SIZE char_size = { 0 };
    GetTextExtentPoint32W(m_memDC, &ch, 1, &char_size);
    char_size.cx += m_widthReduction + x_pos;
    Update_Current_Buffer(char_size.cx);
    uint16_t *curr_buffer = m_bufferList[m_bufferList.Count() - 1]->buffer;
    curr_buffer += m_currPixelOffset;
    int stride = ((width * 3 + 3) & ~3);

    for (int row = 0; row < char_size.cy; row++) {
        int index = (row * stride);

        for (int col = 0; col < char_size.cx; col++) {
            uint8_t pixel_value = m_gdiBitmapBits[index];
            index += 3;

            uint16_t pixel_color = 0;

            if (pixel_value != 0) {
                pixel_color = 0x0FFF;
            }

            *curr_buffer++ = pixel_color | ((pixel_value >> 4) << 12);
        }
    }

    CharDataStruct *char_data = new CharDataStruct;
    char_data->value = ch;
    char_data->width = (int16_t)char_size.cx;
    char_data->buffer = m_bufferList[m_bufferList.Count() - 1]->buffer + m_currPixelOffset;

    if (ch < 256) {
        m_asciiCharArray[ch] = char_data;
    } else {
        m_unicodeCharArray[ch - m_firstUnicodeChar] = char_data;
    }

    m_currPixelOffset += (char_size.cx * m_charHeight);
    return char_data;
#else
    return nullptr;
#endif
}

void FontCharsClass::Update_Current_Buffer(int char_width)
{
    bool needs_new_buffer = (m_bufferList.Count() == 0);

    if (needs_new_buffer == false) {

        if ((char_width * m_charHeight + m_currPixelOffset) > CHAR_BUFFER_LEN) {
            needs_new_buffer = true;
        }
    }

    if (needs_new_buffer) {
        m_bufferList.Add(new FontCharsBuffer);
        m_currPixelOffset = 0;
    }
}

void FontCharsClass::Create_GDI_Font(const char *font_name)
{
#ifdef PLATFORM_WINDOWS
    HDC screen_dc = GetDC(W3D::Get_Window());
    int font_height = -MulDiv(m_pointSize, 96, 72);
    int font_width = 0;
    DWORD bold = m_isBold ? FW_BOLD : FW_NORMAL;
    constexpr DWORD italic = 0;
    constexpr DWORD underline = 0;
    constexpr DWORD strikeout = 0;
    constexpr DWORD charset = 1;
    bool is_generals = false;

    if (std::strcmp(font_name, "Generals") == 0) {
        font_name = "Arial";
        is_generals = true;
    }

    if (is_generals) {
        font_width = ((double)-font_height / 2.5);
    }

    m_widthReduction = std::clamp(font_height / ~7, 0, 4);

    m_gdiFont = CreateFontA(font_height,
        font_width,
        0,
        0,
        bold,
        italic,
        underline,
        strikeout,
        charset,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        VARIABLE_PITCH,
        font_name);

    BITMAPINFOHEADER bitmap_info = { 0 };
    bitmap_info.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.biWidth = m_pointSize * 2;
    bitmap_info.biHeight = m_pointSize * ~2;
    bitmap_info.biPlanes = 1;
    bitmap_info.biBitCount = 24;
    bitmap_info.biCompression = BI_RGB;
    bitmap_info.biSizeImage = 12 * m_pointSize * m_pointSize;
    bitmap_info.biXPelsPerMeter = 0;
    bitmap_info.biYPelsPerMeter = 0;
    bitmap_info.biClrUsed = 0;
    bitmap_info.biClrImportant = 0;
    m_gdiBitmap = CreateDIBSection(
        screen_dc, (const BITMAPINFO *)&bitmap_info, DIB_RGB_COLORS, (void **)&m_gdiBitmapBits, nullptr, 0L);

    m_memDC = CreateCompatibleDC(screen_dc);
    ReleaseDC(W3D::Get_Window(), screen_dc);
    m_oldGDIBitmap = (HBITMAP)SelectObject(m_memDC, m_gdiBitmap);
    m_oldGDIFont = (HFONT)SelectObject(m_memDC, m_gdiFont);
    SetBkColor(m_memDC, RGB(0, 0, 0));
    SetTextColor(m_memDC, RGB(255, 255, 255));
    TEXTMETRIC text_metric = { 0 };
    GetTextMetricsA(m_memDC, &text_metric);
    m_ascent = text_metric.tmAscent;
    m_charHeight = text_metric.tmHeight;
    m_overhang = text_metric.tmOverhang;

    if (is_generals) {
        m_overhang = 0;
    }
#endif
}

void FontCharsClass::Free_GDI_Font()
{
#ifdef PLATFORM_WINDOWS
    if (m_gdiFont != nullptr) {
        SelectObject(m_memDC, m_oldGDIFont);
        DeleteObject(m_gdiFont);
        m_gdiFont = nullptr;
    }

    if (m_gdiBitmap != nullptr) {
        SelectObject(m_memDC, m_oldGDIBitmap);
        DeleteObject(m_gdiBitmap);
        m_gdiBitmap = nullptr;
    }

    if (m_memDC != nullptr) {
        DeleteDC(m_memDC);
        m_memDC = nullptr;
    }
#endif
}

void FontCharsClass::Initialize_GDI_Font(const char *font_name, int point_size, bool is_bold)
{
#ifdef PLATFORM_WINDOWS
    m_name.Format("%s%d", font_name, point_size);
    m_gdiFontName = font_name;
    m_pointSize = point_size;
    m_isBold = is_bold;
    Create_GDI_Font(font_name);
#endif
}

bool FontCharsClass::Is_Font(const char *font_name, int point_size, bool is_bold)
{
    if ((m_gdiFontName.Compare_No_Case(font_name) == 0) && (point_size == m_pointSize) && (is_bold == m_isBold)) {
        return true;
    }

    return false;
}

void FontCharsClass::Grow_Unicode_Array(unichar_t ch)
{
    if (ch < 256) {
        return;
    }

    if (ch >= m_firstUnicodeChar && ch <= m_lastUnicodeChar) {
        return;
    }

    uint16_t first_index = std::min(m_firstUnicodeChar, ch);
    uint16_t last_index = std::max(m_lastUnicodeChar, ch);
    uint16_t count = (last_index - first_index) + 1;
    CharDataStruct **new_array = new CharDataStruct *[count];
    std::memset(new_array, 0, sizeof(CharDataStruct *) * count);

    if (m_unicodeCharArray != nullptr) {
        int start_offset = (m_firstUnicodeChar - first_index);
        int old_count = (m_lastUnicodeChar - m_firstUnicodeChar) + 1;
        std::memcpy(&new_array[start_offset], m_unicodeCharArray, sizeof(CharDataStruct *) * old_count);
        delete[] m_unicodeCharArray;
        m_unicodeCharArray = nullptr;
    }

    m_firstUnicodeChar = first_index;
    m_lastUnicodeChar = last_index;
    m_unicodeCharArray = new_array;
}

void FontCharsClass::Free_Character_Arrays()
{
    if (m_unicodeCharArray != nullptr) {
        int count = (m_lastUnicodeChar - m_firstUnicodeChar) + 1;

        for (int index = 0; index < count; index++) {
            if (m_unicodeCharArray[index] != nullptr) {
                delete m_unicodeCharArray[index];
                m_unicodeCharArray[index] = nullptr;
            }
        }

        delete[] m_unicodeCharArray;
        m_unicodeCharArray = nullptr;
    }

    for (int index = 0; index < 256; index++) {
        if (m_asciiCharArray[index] != nullptr) {
            delete m_asciiCharArray[index];
            m_asciiCharArray[index] = nullptr;
        }
    }
}
