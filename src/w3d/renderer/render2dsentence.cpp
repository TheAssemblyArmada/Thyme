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
#include "dx8wrapper.h"
#include "render2d.h"
#include "surfaceclass.h"
#include "texture.h"
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
    FontCharsClass *unicode_alt = this;

    if (ch < 256) {
        retval = m_asciiCharArray[ch];
    } else {
        while (unicode_alt->m_alternateUnicodeFont != nullptr && unicode_alt->m_alternateUnicodeFont != this) {
            unicode_alt = unicode_alt->m_alternateUnicodeFont;
        }

        unicode_alt->Grow_Unicode_Array(ch);
        retval = unicode_alt->m_unicodeCharArray[ch - unicode_alt->m_firstUnicodeChar];
    }

    if (retval == nullptr) {
        retval = unicode_alt->Store_GDI_Char(ch);
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
        font_width = ((float)-font_height / 2.5f);
    }

    m_widthReduction = std::clamp(-(font_height / 8), 0, 4);

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

    // #BUGFIX Do not pass a BITMAPINFOHEADER* for BITMAPINFO* into CreateDIBSection function.

    BITMAPINFO bitmap_info = { 0 };
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = m_pointSize * 2;
    bitmap_info.bmiHeader.biHeight = -(m_pointSize * 2);
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 24;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    bitmap_info.bmiHeader.biSizeImage = 12 * m_pointSize * m_pointSize;
    bitmap_info.bmiHeader.biXPelsPerMeter = 0;
    bitmap_info.bmiHeader.biYPelsPerMeter = 0;
    bitmap_info.bmiHeader.biClrUsed = 0;
    bitmap_info.bmiHeader.biClrImportant = 0;
    m_gdiBitmap =
        CreateDIBSection(screen_dc, &bitmap_info, DIB_RGB_COLORS, reinterpret_cast<void **>(&m_gdiBitmapBits), nullptr, 0L);

    m_memDC = CreateCompatibleDC(screen_dc);
    ReleaseDC(W3D::Get_Window(), screen_dc);
    m_oldGDIBitmap = (HBITMAP)SelectObject(m_memDC, m_gdiBitmap);
    m_oldGDIFont = (HFONT)SelectObject(m_memDC, m_gdiFont);
    SetBkColor(m_memDC, RGB(0, 0, 0));
    SetTextColor(m_memDC, RGB(255, 255, 255));
    TEXTMETRICA text_metric = { 0 };
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

Render2DSentenceClass::Render2DSentenceClass() :
    m_font(nullptr),
    m_baseLocation(0.0f, 0.0f),
    m_location(0.0f, 0.0f),
    m_cursor(0.0f, 0.0f),
    m_textureOffset(0, 0),
    m_textureStartX(0),
    m_currTextureSize(0),
    m_textureSizeHint(0),
    m_curSurface(nullptr),
    m_monoSpaced(false),
    m_wrapWidth(0.0f),
    m_centered(false),
    m_clipRect(0, 0, 0, 0),
    m_drawExtents(0, 0, 0, 0),
    m_isClippedEnabled(false),
    m_processAmpersand(false),
    m_partialWords(false),
    m_lockedPtr(nullptr),
    m_lockedStride(0),
    m_curTexture(nullptr),
    m_shader(Render2DClass::Get_Default_Shader())
{
}

Render2DSentenceClass::~Render2DSentenceClass()
{
    Ref_Ptr_Release(m_font);
    Reset();
}

void Render2DSentenceClass::Render()
{
    Build_Textures();

    for (int i = 0; i < m_renderers.Count(); ++i) {
        m_renderers[i].renderer->Render();
    }
}

void Render2DSentenceClass::Reset()
{
    if (m_lockedPtr != nullptr) {
        m_curSurface->Unlock();
        m_lockedPtr = nullptr;
    }
    Ref_Ptr_Release(m_curSurface);

    while (m_renderers.Count() > 0) {
        delete m_renderers[0].renderer;
        m_renderers.Delete(0);
    }

    m_cursor.Set(0, 0);
    m_monoSpaced = false;
    m_processAmpersand = false;
    Release_Pending_Surfaces();
    Reset_Sentence_Data();
}

void Render2DSentenceClass::Reset_Polys()
{
    for (int index = 0; index < m_renderers.Count(); index++) {
        m_renderers[index].renderer->Reset();
    }
}

void Render2DSentenceClass::Set_Font(FontCharsClass *font)
{
    Reset();
    Ref_Ptr_Set(m_font, font);
}

void Render2DSentenceClass::Set_Location(const Vector2 &loc)
{
    m_location = loc;
}

void Render2DSentenceClass::Set_Base_Location(const Vector2 &loc)
{
    Vector2 dif = loc - m_baseLocation;
    m_baseLocation = loc;
    for (int i = 0; i < m_renderers.Count(); i++) {
        m_renderers[i].renderer->Move(dif);
    }
}

void Render2DSentenceClass::Make_Additive()
{
    m_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
    m_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
    m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
    m_shader.Set_Secondary_Gradient(ShaderClass::SECONDARY_GRADIENT_DISABLE);

    Set_Shader(m_shader);
}

void Render2DSentenceClass::Set_Shader(ShaderClass shader)
{
    m_shader = shader;
    for (int i = 0; i < m_renderers.Count(); i++) {
        ShaderClass *curr_shader = m_renderers[i].renderer->Get_Shader();
        (*curr_shader) = m_shader;
    }
}

Vector2 Render2DSentenceClass::Get_Text_Extents(const unichar_t *text)
{
    Vector2 extent(0, (float)m_font->Get_Char_Height());

    while (*text != U_CHAR('\0')) {
        unichar_t ch = *text++;

        if (ch != U_CHAR('\n')) {
            extent.X += m_font->Get_Char_Spacing(ch);
        }
    }

    return extent;
}

Vector2 Render2DSentenceClass::Get_Formatted_Text_Extents(const unichar_t *text)
{
    return Build_Sentence_Not_Centered(text, nullptr, nullptr, true);
}

void Render2DSentenceClass::Build_Sentence(const unichar_t *text, int *x, int *y)
{
    if (text == nullptr) {
        return;
    }

    if (m_centered && (m_wrapWidth > 0.0f || u_strchr(text, U_CHAR('\n')))) {
        Build_Sentence_Centered(text, x, y);
    } else {
        Build_Sentence_Not_Centered(text, x, y);
    }
}

void Render2DSentenceClass::Draw_Sentence(uint32_t color)
{
    Render2DClass *curr_renderer = nullptr;
    SurfaceClass *curr_surface = nullptr;

    m_drawExtents.Set(0, 0, 0, 0);
    for (int index = 0; index < m_sentenceData.Count(); index++) {
        SentenceDataStruct &data = m_sentenceData[index];

        if (data.surface != curr_surface) {
            curr_surface = data.surface;
            bool found = false;
            for (int renderer_index = 0; renderer_index < m_renderers.Count(); renderer_index++) {
                if (m_renderers[renderer_index].surface == curr_surface) {
                    found = true;
                    curr_renderer = m_renderers[renderer_index].renderer;
                    break;
                }
            }

            if (found == false) {
                curr_renderer = new Render2DClass;
                curr_renderer->Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());
                *curr_renderer->Get_Shader() = m_shader;
                RendererDataStruct render_info;
                render_info.renderer = curr_renderer;
                render_info.surface = curr_surface;
                m_renderers.Add(render_info);
                for (int surface_index = 0; surface_index < m_pendingSurfaces.Count(); surface_index++) {
                    PendingSurfaceStruct &surface_info = m_pendingSurfaces[surface_index];
                    if (surface_info.surface == curr_surface) {
                        surface_info.renderers.Add(curr_renderer);
                    }
                }
            }
        }

        if (curr_surface == nullptr) {
            return;
        }

        SurfaceClass::SurfaceDescription desc;
        curr_surface->Get_Description(desc);
        RectClass screen_rect = data.screen_rect;
        screen_rect += m_location;
        RectClass uv_rect = data.uv_rect;
        bool add_quad = true;

        if (m_isClippedEnabled) {
            if (screen_rect.right <= m_clipRect.left || screen_rect.bottom <= m_clipRect.top) {
                add_quad = false;
            } else {
                RectClass clipped_rect = screen_rect.Intersect(m_clipRect);
                RectClass clipped_uv_rect;
                float percent = ((clipped_rect.left - screen_rect.left) / screen_rect.Width());
                clipped_uv_rect.left = uv_rect.left + (uv_rect.Width() * percent);

                percent = ((clipped_rect.right - screen_rect.left) / screen_rect.Width());
                clipped_uv_rect.right = uv_rect.left + (uv_rect.Width() * percent);

                percent = ((clipped_rect.top - screen_rect.top) / screen_rect.Height());
                clipped_uv_rect.top = uv_rect.top + (uv_rect.Height() * percent);

                percent = ((clipped_rect.bottom - screen_rect.top) / screen_rect.Height());
                clipped_uv_rect.bottom = uv_rect.top + (uv_rect.Height() * percent);
                screen_rect = clipped_rect;
                uv_rect = clipped_uv_rect;
            }
        }

        if (add_quad) {
            uv_rect *= 1.0f / ((float)desc.width);

            if (curr_renderer != nullptr) {
                curr_renderer->Add_Quad(screen_rect, uv_rect, color);
            }

            if (m_drawExtents.Width() == 0) {
                m_drawExtents = screen_rect;
            } else {
                m_drawExtents += screen_rect;
            }
        }
    }
}

void Render2DSentenceClass::Reset_Sentence_Data()
{
    for (int index = 0; index < m_sentenceData.Count(); index++) {
        Ref_Ptr_Release(m_sentenceData[index].surface);
    }

    if (m_sentenceData.Count() > 0) {
        m_sentenceData.Delete_All();
    }
}

void Render2DSentenceClass::Build_Textures()
{
    if (m_lockedPtr != nullptr) {
        m_curSurface->Unlock();
        m_lockedPtr = nullptr;
    }

    Ref_Ptr_Release(m_curSurface);
    m_textureOffset.Set(0, 0);
    m_textureStartX = 0;

    for (int index = 0; index < m_pendingSurfaces.Count(); index++) {
        PendingSurfaceStruct &surface_info = m_pendingSurfaces[index];
        SurfaceClass *curr_surface = surface_info.surface;
        SurfaceClass::SurfaceDescription desc;
        curr_surface->Get_Description(desc);
        TextureClass *new_texture = new TextureClass(
            desc.width, desc.width, WW3D_FORMAT_A4R4G4B4, MipCountType::MIP_LEVELS_1, PoolType::POOL_MANAGED, false, true);
        SurfaceClass *texture_surface = new_texture->Get_Surface_Level(0);
        new_texture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        new_texture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        new_texture->Get_Texture_Filter()->Set_Min_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        new_texture->Get_Texture_Filter()->Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        new_texture->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);

#ifdef BUILD_WITH_D3D8
        DX8CALL(CopyRects(curr_surface->Peek_D3D_Surface(), nullptr, 0, texture_surface->Peek_D3D_Surface(), nullptr));
#endif
        Ref_Ptr_Release(texture_surface);

        for (int renderer_index = 0; renderer_index < surface_info.renderers.Count(); renderer_index++) {
            Render2DClass *renderer = surface_info.renderers[renderer_index];
            renderer->Set_Texture(new_texture);
        }

        Ref_Ptr_Release(new_texture);
        Ref_Ptr_Release(curr_surface);
    }

    m_pendingSurfaces.Reset_Active();
}

void Render2DSentenceClass::Record_Sentence_Chunk()
{
    int width = m_textureOffset.I - m_textureStartX;

    if (width > 0) {
        float char_height = (float)m_font->Get_Char_Height();
        SentenceDataStruct sentence_data;
        sentence_data.surface = m_curSurface;
        sentence_data.surface->Add_Ref();
        sentence_data.screen_rect.left = m_cursor.X;
        sentence_data.screen_rect.right = m_cursor.X + width;
        sentence_data.screen_rect.top = m_cursor.Y;
        sentence_data.screen_rect.bottom = m_cursor.Y + char_height;
        sentence_data.uv_rect.left = (float)m_textureStartX;
        sentence_data.uv_rect.top = (float)m_textureOffset.J;
        sentence_data.uv_rect.right = (float)m_textureOffset.I;
        sentence_data.uv_rect.bottom = (float)(m_textureOffset.J + char_height);
        m_sentenceData.Add(sentence_data);
    }
}

void Render2DSentenceClass::Allocate_New_Surface(const unichar_t *text, bool reuse_surface)
{
    if (!reuse_surface && m_lockedPtr != nullptr) {
        m_curSurface->Unlock();
        m_lockedPtr = nullptr;
    }

    int text_width = 0;

    for (int index = 0; text[index] != U_CHAR('\0'); index++) {
        text_width += m_font->Get_Char_Spacing(text[index]);
    }

    const int char_height = m_font->Get_Char_Height() + 1;

    m_currTextureSize = 256;
    int best_tex_mem_usage = 999999999;

    for (int pow2 = 6; pow2 <= 8; pow2++) {
        int size = 1 << pow2;
        int row_count = (text_width / size) + 1;
        int rows_per_texture = size / char_height;

        if (rows_per_texture > 0) {
            int texture_count = row_count / rows_per_texture;
            texture_count = std::max(texture_count, 1);
            int texture_mem_usage = (texture_count * size * size);

            if (texture_mem_usage < best_tex_mem_usage) {
                m_currTextureSize = size;
                best_tex_mem_usage = texture_mem_usage;
            }
        }
    }

    if (!reuse_surface) {
        m_currTextureSize = std::max(m_textureSizeHint, m_currTextureSize);
        Ref_Ptr_Release(m_curSurface);
        m_curSurface = new SurfaceClass(m_currTextureSize, m_currTextureSize, WW3D_FORMAT_A4R4G4B4);
        captainslog_dbgassert(m_curSurface != nullptr, "Surface allocation failed.");
        m_curSurface->Add_Ref();
        PendingSurfaceStruct surface_info;
        surface_info.surface = m_curSurface;
        m_pendingSurfaces.Add(surface_info);
    }

    m_textureOffset.Set(0, 0);
    m_textureStartX = 0;
}

void Render2DSentenceClass::Release_Pending_Surfaces()
{
    for (int index = 0; index < m_pendingSurfaces.Count(); index++) {
        SurfaceClass *curr_surface = m_pendingSurfaces[index].surface;
        Ref_Ptr_Release(curr_surface);
    }

    if (m_pendingSurfaces.Count() > 0) {
        m_pendingSurfaces.Delete_All();
    }
}

void Render2DSentenceClass::Build_Sentence_Centered(const unichar_t *text, int *x, int *y)
{
    int x_pos = 0;
    int y_pos = 0;
    int word_count = 0;
    int final_x_pos = 0;
    int tracked_width = 0;
    const unichar_t *cur_text = text;
    float char_height = m_font->Get_Char_Height();
    Vector2 position = Build_Sentence_Not_Centered(text, &x_pos, &y_pos, true);
    Reset_Sentence_Data();
    m_cursor.Set(0.0f, 0.0f);

    if (m_curSurface == nullptr) {
        Allocate_New_Surface(text, false);
    }

    bool sentence_done = false;
    bool ampersand_handled = false;

    while (true) {
        int word_width = 0;
        int sentence_width = 0;
        int word_count_tracker = 0;
        int spacing;

        // Calculate various parameters about the sentence we are going to render.
        while (true) {
            spacing = 0;

            // Process a word in the sentence.
            for (unichar_t test_ch = *cur_text; *cur_text != U_CHAR('\0'); test_ch = *cur_text) {
                if (test_ch <= U_CHAR(' ')) {
                    break;
                }

                if (m_processAmpersand && test_ch == U_CHAR('&')) {
                    // Spaces before the ampersand also get disregarded?
                    if (word_width != 0 && cur_text[-1] == U_CHAR(' ')) {
                        sentence_width -= word_width;
                    }

                    ++cur_text;
                    ampersand_handled = true;
                }

                unichar_t cur_ch = *cur_text;
                ++cur_text;
                spacing = m_font->Get_Char_Spacing(cur_ch);
                word_width += spacing;
                ++word_count;
                tracked_width = word_width;

                if (m_wrapWidth > 0.0f && word_width >= m_wrapWidth && m_partialWords) {
                    break;
                }
            }

            // Wrap the text if enabled by a wrapping width being greater than 0.
            if (m_wrapWidth > 0.0f) {
                tracked_width = word_width + sentence_width;

                if (tracked_width >= m_wrapWidth) {
                    if (word_count_tracker == 0) {
                        word_count_tracker = word_count - 1;
                        sentence_width += word_width - spacing;

                        if (*cur_text == U_CHAR('\0')) {
                            sentence_done = true;
                        }
                    } else {
                        --word_count_tracker;
                    }
                    break;
                }
            }

            if (*cur_text == U_CHAR('\0')) {
                word_count_tracker += word_count;
                sentence_width += word_width;
                sentence_done = true;
                break;
            }

            word_count_tracker += word_count + 1;
            sentence_width += word_width;

            if (*cur_text != U_CHAR(' ')) {
                break;
            }

            ++cur_text;
            word_width = m_font->Get_Char_Spacing(U_CHAR(' '));
            word_count = 0;
            sentence_width += word_width;
        }

        m_cursor.X = (int)std::max((position.X - sentence_width) * 0.5f, 0.0f);

        if (ampersand_handled) {
            ampersand_handled = false;
            final_x_pos = x_pos + m_cursor.X;
        }

        // Blit the prepared line to the current texture.
        for (int i = 0; i <= word_count_tracker; ++i) {
            unichar_t cur_ch = *text++;
            bool ampersand_processed = false;

            if (m_processAmpersand && cur_ch == U_CHAR('&')) {
                unichar_t next_ch = *text;
                if (next_ch > U_CHAR(' ')) {
                    cur_ch = next_ch;
                    ++text;
                    ampersand_processed = true;
                }
            }

            float char_spacing = m_font->Get_Char_Spacing(cur_ch);
            bool tex_too_small = m_currTextureSize <= m_textureOffset.I + char_spacing;

            if (tex_too_small || (cur_ch == U_CHAR(' ') || cur_ch == U_CHAR('\n') || cur_ch == U_CHAR('\0'))) {
                Record_Sentence_Chunk();
                m_cursor.X += m_textureOffset.I - m_textureStartX;
                m_textureStartX = m_textureOffset.I;

                if (cur_ch == U_CHAR(' ')) {
                    m_cursor.X += char_spacing;
                } else if (cur_ch == U_CHAR('\0') || cur_ch == U_CHAR('\n')) {
                    break;
                }

                if (tex_too_small) {
                    m_textureStartX = 0;
                    m_textureOffset.I = 0;
                    m_textureOffset.J += char_height;

                    if (m_currTextureSize <= m_textureOffset.J + char_height) {
                        Allocate_New_Surface(text, false);
                    }
                }
            }

            if (cur_ch != U_CHAR('\n') && cur_ch != U_CHAR(' ')) {
                if (m_lockedPtr == nullptr) {
                    m_lockedPtr = static_cast<uint16_t *>(m_curSurface->Lock(&m_lockedStride));
                    captainslog_assert(m_lockedPtr != nullptr);
                }

                captainslog_assert(((m_textureOffset.I + char_spacing) < m_currTextureSize)
                    && ((m_textureOffset.J + char_height) < m_currTextureSize));

                float x_increment;

                if (ampersand_processed) {
                    x_increment = m_font->m_widthReduction + char_spacing;

                    if (cur_ch == U_CHAR('M')) {
                        x_increment += 1.0f;
                    }
                } else {
                    m_font->Blit_Char(cur_ch, m_lockedPtr, m_lockedStride, m_textureOffset.I, m_textureOffset.J);
                    x_increment = char_spacing;
                }

                m_textureOffset.I += x_increment;
            }
        }

        m_cursor.X = 0.0f;
        m_cursor.Y += char_height;

        if (sentence_done) {
            break;
        }

        cur_text = text;
        word_count = 0;
    }

    if (x != nullptr) {
        *x = final_x_pos;
    }
    if (y != nullptr) {
        *y = 0;
    }
}

Vector2 Render2DSentenceClass::Build_Sentence_Not_Centered(const unichar_t *text, int *x, int *y, bool reuse_surface)
{
    if (!reuse_surface) {
        Reset_Sentence_Data();
    }

    float initial_cursor_x = m_cursor.X;
    float initial_cursor_y = m_cursor.Y;
    m_cursor.Set(0, 0);

    if (m_curSurface == nullptr) {
        Allocate_New_Surface(text, reuse_surface);
    }

    bool wrapped = false;
    int y_pos = 0;
    int x_pos = 0;
    int initial_tex_i = m_textureOffset.I;
    int initial_tex_j = m_textureOffset.J;
    m_textureOffset.Set(2, 0);
    m_textureStartX = 2;
    float char_height = (float)m_font->Get_Char_Height();
    float width = 0.0f;

    while (text != nullptr) {
        unichar_t ch = *text++;
        bool unkbool_ampersand = false;

        if (m_processAmpersand && ch == U_CHAR('&')) {
            unichar_t next_ch = *text;

            if (next_ch != U_CHAR('\0') && next_ch > U_CHAR(' ') && next_ch != U_CHAR('\n')) {

                y_pos = m_cursor.Y;

                if (wrapped) {
                    x_pos = 0;
                } else {
                    x_pos = m_textureOffset.I + m_cursor.X - m_textureStartX;
                }
                ch = *text++;
                unkbool_ampersand = true;
            }
        }

        float char_spacing = (float)m_font->Get_Char_Spacing(ch);
        bool texture_width_exceeded = ((m_textureOffset.I + char_spacing) >= m_currTextureSize);
        bool encountered_break_char = (ch == U_CHAR(' ') || ch == U_CHAR('\n') || ch == U_CHAR('\0'));
        bool wrap_width_exceeded = false;

        if (m_partialWords && m_wrapWidth != 0.0f) {
            if (m_textureOffset.I + m_cursor.X - m_textureStartX + char_spacing >= m_wrapWidth) {
                wrap_width_exceeded = true;
            }
        }

        if (texture_width_exceeded || encountered_break_char || wrap_width_exceeded) {
            if (!reuse_surface) {
                Record_Sentence_Chunk();
            }

            m_cursor.X += m_textureOffset.I - m_textureStartX;

            if (width <= m_cursor.X) {
                width = m_cursor.X;
            }

            m_textureStartX = m_textureOffset.I;

            switch (ch) {
                case U_CHAR(' '):
                    if (m_wrapWidth > 0.0f) {
                        float spacing = char_spacing;
                        const unichar_t *tmp_text = text;

                        while (*tmp_text != U_CHAR('\0')) {
                            if (*tmp_text <= U_CHAR(' ')) {
                                break;
                            }

                            if (m_processAmpersand && *tmp_text == U_CHAR('&')) {
                                ++tmp_text;
                            }

                            spacing += m_font->Get_Char_Spacing(*tmp_text);
                            ++tmp_text;
                        }

                        if (spacing + m_cursor.X >= m_wrapWidth) {
                            m_cursor.X = 0.0f;
                            m_cursor.Y = char_height + m_cursor.Y;
                            wrapped = true;
                        }
                    }
                    break;
                case U_CHAR('\n'):
                    m_cursor.X = 0.0f;
                    m_cursor.Y = char_height + m_cursor.Y;
                    break;
                case U_CHAR('\0'):
                    goto exit;
                default:
                    if (wrap_width_exceeded) {
                        m_cursor.X = 0.0f;
                        m_cursor.Y = char_height + m_cursor.Y;
                    }
                    break;
            }

            if (texture_width_exceeded) {
                m_textureStartX = 2;
                m_textureOffset.I = 2;
                m_textureOffset.J += char_height;

                if (m_currTextureSize <= m_textureOffset.J + char_height) {
                    Allocate_New_Surface(text, reuse_surface);
                }
            }
        }

        if (ch != U_CHAR('\n')) {
            if (!reuse_surface && m_lockedPtr == nullptr) {
                m_lockedPtr = static_cast<uint16_t *>(m_curSurface->Lock(&m_lockedStride));
                captainslog_assert(m_lockedPtr != nullptr);
            }

            captainslog_assert(((m_textureOffset.I + char_spacing) < m_currTextureSize)
                && ((m_textureOffset.J + char_height) < m_currTextureSize));

            if (!reuse_surface && !unkbool_ampersand) {
                m_font->Blit_Char(ch, m_lockedPtr, m_lockedStride, m_textureOffset.I, m_textureOffset.J);
            }

            m_textureOffset.I += char_spacing;
        }
    }

exit:
    Vector2 result(m_font->m_widthReduction + width, char_height + m_cursor.Y);
    m_cursor.Set(initial_cursor_x, initial_cursor_y);
    m_textureOffset.Set(initial_tex_i, initial_tex_j);

    if (x != nullptr) {
        *x = x_pos;
    }
    if (y != nullptr) {
        *y = y_pos;
    }

    return result;
}
