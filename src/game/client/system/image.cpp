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
#include "always.h"
#include "image.h"
#include "globaldata.h"
#include "ini.h"
#include "namekeygenerator.h"
#include <stdio.h>
#if defined PLATFORM_WINDOWS
#include <utf.h>
#endif

#ifndef GAME_DLL
ImageCollection *g_theMappedImageCollection = nullptr;
#endif

const FieldParse Image::s_imageFieldParseTable[] = {
    { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(Image, m_filename) },
    { "TextureWidth", &INI::Parse_Int, nullptr, offsetof(Image, m_textureSize.x) },
    { "TextureHeight", &INI::Parse_Int, nullptr, offsetof(Image, m_textureSize.y) },
    { "Coords", &Parse_Image_Coords, nullptr, offsetof(Image, m_UVCoords) },
    { "Status", &Parse_Image_Status, nullptr, offsetof(Image, m_status) },
    { nullptr, nullptr, nullptr, 0 }
};

Image::Image() : m_name(), m_filename(), m_textureSize(), m_imageSize(), m_rawTextureData(), m_status(0)
{
    m_UVCoords.hi.x = 1.0f;
    m_UVCoords.hi.y = 1.0f;
    m_textureSize.x = 0;
    m_textureSize.y = 0;
    m_UVCoords.lo.x = 0.0f;
    m_UVCoords.lo.y = 0.0f;
    m_imageSize.x = 0;
    m_imageSize.y = 0;
    m_rawTextureData = nullptr;
}

Image::~Image()
{
    return;
}

// 0x00519260
void Image::Set_Status(uint32_t bit)
{
    m_status |= bit;
}

void Image::Clear_Status(uint32_t bit)
{
    m_status &= ~bit;
}

// 0x00518F10
void Image::Parse_Image_Coords(INI *ini, void *formal, void *store, const void *user_data)
{
    Image *image = static_cast<Image *>(formal);
    const char *token = ini->Get_Next_Sub_Token("Left");
    int left = ini->Scan_Int(token);
    token = ini->Get_Next_Sub_Token("Top");
    int top = ini->Scan_Int(token);
    token = ini->Get_Next_Sub_Token("Right");
    int right = ini->Scan_Int(token);
    token = ini->Get_Next_Sub_Token("Bottom");
    int bottom = ini->Scan_Int(token);
    float lowX = left;
    float lowY = top;
    float highX = right;
    float highY = bottom;
    if (image->m_textureSize.x != 0) {
        lowX = lowX / image->m_textureSize.x;
        highX = highX / image->m_textureSize.x;
    }
    if (image->m_textureSize.y != 0) {
        lowY = lowY / image->m_textureSize.y;
        highY = highY / image->m_textureSize.y;
    }
    if (image->m_UVCoords.hi.x != 0) {
        image->m_UVCoords.lo.x = lowX;
        image->m_UVCoords.lo.y = lowY;
        image->m_UVCoords.hi.x = highX;
        image->m_UVCoords.hi.y = highY;
    }
    image->m_imageSize.x = right - left;
    image->m_imageSize.y = bottom - top;
}

// 0x00519030
void Image::Parse_Image_Status(INI *ini, void *instance, void *store, const void *user_data)
{
    const char *_imageStatusNames[3] = { "ROTATED_90_CLOCKWISE", "RAW_TEXTURE", nullptr };

    Image *image = static_cast<Image *>(instance);
    ini->Parse_Bitstring32(ini, instance, store, _imageStatusNames);
    uint8_t *byte = static_cast<uint8_t *>(store);
    if (*byte & IMAGE_STATUS_ROTATED_90_CLOCKWISE) {
        int oldx = image->m_imageSize.x;
        image->m_imageSize.x = image->m_imageSize.y;
        image->m_imageSize.y = oldx;
    }
}

ImageCollection::~ImageCollection()
{
    for (auto &iter : m_imageMap) {
        iter.second->Delete_Instance();
    }
}

void ImageCollection::Add_Image(Image *image)
{
    m_imageMap[g_theNameKeyGenerator->Name_To_Lower_Case_Key(image->Get_Name().Str())] = image;
}

Image *ImageCollection::Find_Image_By_Name(const Utf8String &name)
{
    auto found = m_imageMap.find(g_theNameKeyGenerator->Name_To_Lower_Case_Key(name.Str()));

    if (found == m_imageMap.end()) {
        return nullptr;
    } else {
        return found->second;
    }
}

void ImageCollection::Load(int texture_size)
{
#ifdef PLATFORM_WINDOWS
    INI ini;
    Utf8String str;

    if (g_theWriteableGlobalData != nullptr) {
        str.Format("%sINI\\MappedImages\\*.ini", g_theWriteableGlobalData->Get_Path_User_Data().Str());
        WIN32_FIND_DATAW data;

        if (FindFirstFileW(UTF8To16(str.Str()), &data) != INVALID_HANDLE_VALUE) {
            str.Format("%sINI\\MappedImages", g_theWriteableGlobalData->Get_Path_User_Data().Str());
            ini.Load_Directory(str, true, INI_LOAD_OVERWRITE, nullptr);
        }
    }

    char buf[MAX_PATH];
    sprintf(buf, "Data\\INI\\MappedImages\\TextureSize_%d", texture_size);
    ini.Load_Directory(buf, true, INI_LOAD_OVERWRITE, nullptr);
    ini.Load_Directory("Data\\INI\\MappedImages\\HandCreated", true, INI_LOAD_OVERWRITE, nullptr);
#endif
}

void ImageCollection::Parse_Mapped_Image_Definition(INI *ini)
{
    Utf8String str;
    str.Set(ini->Get_Next_Token());

    if (g_theMappedImageCollection != nullptr) {
        Image *image = g_theMappedImageCollection->Find_Image_By_Name(str);
        captainslog_dbgassert(image == nullptr || image->Get_Raw_Texture_Data() == nullptr,
            "We are trying to parse over an existing image that contains a non-null rawTextureData, you should fix that");

        if (image == nullptr) {
            image = new Image();
            image->Set_Name(str);
            g_theMappedImageCollection->Add_Image(image);
            captainslog_dbgassert(image != nullptr, "Parse_Mapped_Image: unable to allocate image for '%s'", str.Str());
        }

        ini->Init_From_INI(image, Image::Get_Field_Parse());
    }
}

void ImageCollection::Parse_Mapped_Image(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *str = ini->Get_Next_Token();

    if (g_theMappedImageCollection != nullptr) {
        *(static_cast<Image **>(store)) = g_theMappedImageCollection->Find_Image_By_Name(str);
    }
}
