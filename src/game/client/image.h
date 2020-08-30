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

class Utf8String;
class TextureClass;

class Image : public MemoryPoolObject
{
    IMPLEMENT_POOL(Image);

public:
    Image();
    virtual ~Image();

    // Methods
    void Clear_Status(uint32_t bit);//TODO not hooked or verified
    void Set_Status(uint32_t bit);

    // initFromINIMulti variants for Field Parsing Functions.
    static void Parse_Image_Coords(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Image_Status(INI *ini, void *formal, void *store, const void *user_data);//TODO not hooked or verified

#ifdef GAME_DLL
    Image *Hook_Ctor() { return new (this) Image(); }
    void Hook_Dtor() { Image::~Image(); }
#endif

private:
    Utf8String m_name;
    Utf8String m_filename;
    ICoord2D m_textureSize;
    Region2D m_UVCoords;
    ICoord2D m_imageSize;
    TextureClass *m_rawTextureData;
    uint32_t m_status;
};