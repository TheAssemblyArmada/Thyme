/**
 * @file
 *
 * @author Smileynator
 *
 * @brief Handles Image coordinates and references for asssistance in display of images.
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
#include "ini.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"
#include <map>

class Utf8String;
class TextureClass;

class Image : public MemoryPoolObject
{
    IMPLEMENT_POOL(Image);

protected:
    virtual ~Image() override;

public:
    Image();

    // Methods
    void Clear_Status(uint32_t bit); // TODO not hooked or verified
    void Set_Status(uint32_t bit);
    bool Is_Set_Status(uint32_t bit) const { return m_status & bit; }
    TextureClass *Get_Raw_Texture_Data() const { return m_rawTextureData; }
    Utf8String Get_File_Name() const { return m_filename; }
    Utf8String Get_Name() { return m_name; }
    void Set_Name(Utf8String str) { m_name = str; }
    void Set_Filename(Utf8String str) { m_filename = str; }
    Region2D Get_UV_Region() const { return m_UVCoords; }
    void Set_Texture_Width(int width) { m_textureSize.x = width; }
    void Set_Texture_Height(int height) { m_textureSize.y = height; }
    void Set_UV_Region(Region2D *region)
    {
        if (region != nullptr) {
            m_UVCoords = *region;
        }
    }
    int Get_Image_Width() const { return m_imageSize.x; }
    int Get_Image_Height() const { return m_imageSize.y; }
    const ICoord2D *Get_Image_Size() const { return &m_imageSize; }

    // initFromINIMulti variants for Field Parsing Functions.
    static void Parse_Image_Coords(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Image_Status(INI *ini, void *formal, void *store, const void *user_data);

#ifdef GAME_DLL
    Image *Hook_Ctor() { return new (this) Image(); }
    void Hook_Dtor() { Image::~Image(); }
#endif

    enum ImageStatus // Note these are flags
    {
        IMAGE_STATUS_NONE = 0,
        IMAGE_STATUS_ROTATED_90_CLOCKWISE = (1 << 0),
        IMAGE_STATUS_RAW_TEXTURE = (1 << 1),
    };

    static const FieldParse *Get_Field_Parse() { return s_imageFieldParseTable; }

private:
    Utf8String m_name;
    Utf8String m_filename;
    ICoord2D m_textureSize;
    Region2D m_UVCoords;
    ICoord2D m_imageSize;
    TextureClass *m_rawTextureData;
    uint32_t m_status;
    static const FieldParse s_imageFieldParseTable[];
};

class ImageCollection : public SubsystemInterface
{
public:
    ImageCollection() {}
    virtual ~ImageCollection() override;
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    void Add_Image(Image *image);
    void Load(int texture_size);
    Image *Find_Image_By_Name(const Utf8String &name);
    static void Parse_Mapped_Image_Definition(INI *ini);
    static void Parse_Mapped_Image(INI *ini, void *formal, void *store, const void *user_data);

#ifdef GAME_DLL
    ImageCollection *Hook_Ctor() { return new (this) ImageCollection(); }
#endif

private:
    std::map<unsigned int, Image *> m_imageMap;
};

#ifdef GAME_DLL
extern ImageCollection *&g_theMappedImageCollection;
#else
extern ImageCollection *g_theMappedImageCollection;
#endif
