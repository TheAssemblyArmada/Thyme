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
#include "rect.h"
#include "refcount.h"
#include "shader.h"
#include "vector.h"
#include "vector2.h"
#include "vector2i.h"
#include "w3dmpo.h"
#include "wwstring.h"
#include <new>

class Render2DClass;
class SurfaceClass;
class TextureClass;

constexpr unsigned CHAR_BUFFER_LEN = 0x8000;

struct FontCharsBuffer : public W3DMPO
{
    uint16_t buffer[CHAR_BUFFER_LEN];
    IMPLEMENT_W3D_POOL(FontCharsBuffer)
};

#ifdef BUILD_WITH_FREETYPE
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_ *FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_ *FT_Face;
#endif

#ifdef BUILD_WITH_FONTCONFIG
struct _FcConfig;
typedef struct _FcConfig FcConfig;
#endif

class FontCharsClass : public W3DMPO, public RefCountClass
{
    friend class W3DFontLibrary;
    friend class Render2DSentenceClass;
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
    bool Locate_Font(const char *font_name, StringClass &font_file_path);
#ifdef BUILD_WITH_FONTCONFIG
    bool Locate_Font_FontConfig(const char *font_name, StringClass &font_file_path);
#endif
#ifdef PLATFORM_WINDOWS
    bool Locate_Font_WinRegistry(const char *font_name, StringClass &font_file_path);
#endif
    // GDI implementation
    void Create_GDI_Font(const char *font_name);
    void Free_GDI_Font();
    const CharDataStruct *Store_GDI_Char(unichar_t ch);
    // Freetype implementation
#ifdef BUILD_WITH_FREETYPE
    void Create_Freetype_Font(const char *font_name);
    void Free_Freetype_Font();
    const CharDataStruct *Store_Freetype_Char(unichar_t ch);
#endif
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
#ifdef BUILD_WITH_FREETYPE
    FT_Library m_ftLibrary = nullptr;
    FT_Face m_ftFace = nullptr;
#endif
#ifdef BUILD_WITH_FONTCONFIG
    FcConfig *m_fc = nullptr;
#endif
    FontCharsClass::CharDataStruct *m_asciiCharArray[256];
    FontCharsClass::CharDataStruct **m_unicodeCharArray;
    unichar_t m_firstUnicodeChar;
    unichar_t m_lastUnicodeChar;
    bool m_isBold;
};

class Render2DSentenceClass
{
    ALLOW_HOOKING
    struct SentenceDataStruct
    {
        SurfaceClass *surface;
        RectClass screen_rect;
        RectClass uv_rect;
        bool operator==(const SentenceDataStruct &that) { return false; }
        bool operator!=(const SentenceDataStruct &that) { return false; }
    };

    struct PendingSurfaceStruct
    {
        SurfaceClass *surface;
        DynamicVectorClass<Render2DClass *> renderers;
        bool operator==(const PendingSurfaceStruct &that) { return false; }
        bool operator!=(const PendingSurfaceStruct &that) { return false; }
    };

    struct RendererDataStruct
    {
        Render2DClass *renderer;
        SurfaceClass *surface;
        bool operator==(const RendererDataStruct &that) { return false; }
        bool operator!=(const RendererDataStruct &that) { return false; }
    };

public:
    Render2DSentenceClass();
    ~Render2DSentenceClass();

    virtual void Reset();

    void Render();
    void Reset_Polys();
    FontCharsClass *Peek_Font() { return m_font; }
    void Set_Font(FontCharsClass *font);
    void Set_Location(const Vector2 &loc);
    void Set_Base_Location(const Vector2 &loc);
    void Set_Wrapping_Width(float width) { m_wrapWidth = width; }
    void Set_Clipping_Rect(const RectClass &rect)
    {
        m_clipRect = rect;
        m_isClippedEnabled = true;
    }
    bool Is_Clipping_Enabled() const { return m_isClippedEnabled; }
    void Enable_Clipping(bool onoff) { m_isClippedEnabled = onoff; }
    void Make_Additive();
    ShaderClass Get_Shader() const { return m_shader; }
    void Set_Shader(ShaderClass shader);
    const RectClass &Get_Draw_Extents() { return m_drawExtents; }
    Vector2 Get_Text_Extents(const unichar_t *text);
    Vector2 Get_Formatted_Text_Extents(const unichar_t *text);
    void Build_Sentence(const unichar_t *text, int *x = nullptr, int *y = nullptr);
    void Draw_Sentence(uint32_t color = 0xFFFFFFFF);
    void Set_Texture_Size_Hint(int hint) { m_textureSizeHint = hint; }
    int Get_Texture_Size_Hint() const { return m_textureSizeHint; }
    void Set_Mono_Spaced(bool onoff) { m_monoSpaced = onoff; }

private:
    void Reset_Sentence_Data();
    void Build_Textures();
    void Record_Sentence_Chunk();
    void Allocate_New_Surface(const unichar_t *text, bool reuse_surface);
    void Release_Pending_Surfaces();
    void Build_Sentence_Centered(const unichar_t *text, int *x = nullptr, int *y = nullptr);
    Vector2 Build_Sentence_Not_Centered(
        const unichar_t *text, int *x = nullptr, int *y = nullptr, bool reuse_surface = false);

private:
    DynamicVectorClass<SentenceDataStruct> m_sentenceData;
    DynamicVectorClass<PendingSurfaceStruct> m_pendingSurfaces;
    DynamicVectorClass<RendererDataStruct> m_renderers;
    FontCharsClass *m_font;
    Vector2 m_baseLocation;
    Vector2 m_location;
    Vector2 m_cursor;
    Vector2i m_textureOffset;
    int m_textureStartX;
    int m_currTextureSize;
    int m_textureSizeHint;
    SurfaceClass *m_curSurface;
    bool m_monoSpaced;
    float m_wrapWidth;
    bool m_centered;
    RectClass m_clipRect;
    RectClass m_drawExtents;
    bool m_isClippedEnabled;
    bool m_processAmpersand;
    bool m_partialWords;
    uint16_t *m_lockedPtr;
    int m_lockedStride;
    TextureClass *m_curTexture;
    ShaderClass m_shader;
    friend class W3DDisplayString;
};
