/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Manager class for thumbnail info.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thumbnailmanager.h"
#include "thumbnail.h"

#ifndef THYME_STANDALONE
DLListClass<ThumbnailManagerClass> &ThumbnailManagerClass::ThumbnailManagerList =
    Make_Global<DLListClass<ThumbnailManagerClass>>(0x00A544B0);
ThumbnailManagerClass *&g_thumbnailManager = Make_Global<ThumbnailManagerClass *>(0x00A544A8);
#else
DLListClass<ThumbnailManagerClass> ThumbnailManagerClass::ThumbnailManagerList;
ThumbnailManagerClass *g_thumbnailManager;
#endif

namespace
{
StringClass Create_Texture_Hash_Name(const StringClass &textureFileName)
{
    StringClass result = textureFileName;
    result.Truncate_Right(4);
    result.To_Lower();
    return result;
}

} // namespace

ThumbnailManagerClass::ThumbnailManagerClass(const char *thumbfilename, const char *mixfilename) {}

ThumbnailManagerClass::~ThumbnailManagerClass() {}

ThumbnailClass *ThumbnailManagerClass::Peek_Thumbnail_Instance(const StringClass &texture)
{
    return Get_From_Hash(texture);
}

void ThumbnailManagerClass::Insert_To_Hash(ThumbnailClass *thumbnail)
{
    Loading = true;
    Hash.Insert(Create_Texture_Hash_Name(thumbnail->Get_Name()), thumbnail);
}

ThumbnailClass *ThumbnailManagerClass::Get_From_Hash(const StringClass &texture)
{
    return nullptr;
}

void ThumbnailManagerClass::Remove_From_Hash(ThumbnailClass *thumbnail) {}

void ThumbnailManagerClass::Create_Thumbnails() {}

void ThumbnailManagerClass::Save(bool force)
{
    // Stripped to minimum for Zero Hour, Renegade has functional version.
    if (Loading || force) {
        Loading = false;
    }
}

void ThumbnailManagerClass::Add_Thumbnail_Manager(const char *thumbfilename, const char *mixfilename) {}

void ThumbnailManagerClass::Remove_Thumbnail_Manager(const char *thumbfilename) {}

void ThumbnailManagerClass::Update_Thumbnail_File(const char *thumbfilename, bool b) {}

ThumbnailManagerClass *ThumbnailManagerClass::Peek_Thumbnail_Manager(const char *thumbfilename)
{
    return nullptr;
}

ThumbnailClass *ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager(const StringClass &texture)
{
    return nullptr;
}

void ThumbnailManagerClass::Init()
{
    g_thumbnailManager = new ThumbnailManagerClass("global.th6", nullptr);
    g_thumbnailManager->AllowThumbnailCreation = true;
}

void ThumbnailManagerClass::Deinit()
{
    while (ThumbnailManagerList.Head() != nullptr) {
        delete ThumbnailManagerList.Head();
    }

    delete g_thumbnailManager;
    g_thumbnailManager = nullptr;
}
