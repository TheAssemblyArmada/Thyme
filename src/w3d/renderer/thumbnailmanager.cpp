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

#ifndef GAME_DLL
DLListClass<ThumbnailManagerClass> ThumbnailManagerClass::s_thumbnailManagerList;
ThumbnailManagerClass *g_thumbnailManager;
#endif

bool ThumbnailManagerClass::s_createIfNotFound = false;

namespace
{
/**
 * Helper function to generate correctly formatted string for the hash function.
 */
StringClass Create_Texture_Hash_Name(const StringClass &textureFileName)
{
    StringClass result = textureFileName;
    result.Truncate_Right(4);
    result.To_Lower();
    return result;
}

} // namespace

/**
 * 0x0086A810
 */
ThumbnailManagerClass::ThumbnailManagerClass(const char *thumbfilename, const char *mixfilename) :
    m_allowThumbnailCreation(false),
    m_thumbFileName(thumbfilename),
    m_mixFileName(mixfilename),
    m_bitmap(nullptr),
    m_loading(false),
    m_time(0)
{
    Load();
}

/**
 * 0x0086A9B0
 */
ThumbnailManagerClass::~ThumbnailManagerClass()
{
    Save(false);

    for (HashTemplateIterator<StringClass, ThumbnailClass *> thumbnail(Hash); !thumbnail.Is_Done(); thumbnail.Next()) {
        delete thumbnail.Peek_Value();
    }

    delete[] m_bitmap;
}

/**
 * Get a thumbnail instance from a texture name.
 */
ThumbnailClass *ThumbnailManagerClass::Peek_Thumbnail_Instance(const StringClass &texture)
{
    return Get_From_Hash(texture);
}

/**
 * Inserts a thumbnail into the managers hash table.
 */
void ThumbnailManagerClass::Insert_To_Hash(ThumbnailClass *thumbnail)
{
    m_loading = true;
    Hash.Insert(Create_Texture_Hash_Name(thumbnail->Get_Name()), thumbnail);
}

/**
 * Retrieves a thumbnail from the managers hash table.
 */
ThumbnailClass *ThumbnailManagerClass::Get_From_Hash(const StringClass &texture)
{
    return Hash.Get(Create_Texture_Hash_Name(texture));
}

/**
 * Removes a thumbnail from the managers hash table.
 *
 * 0x0086AE20
 */
void ThumbnailManagerClass::Remove_From_Hash(ThumbnailClass *thumbnail)
{
    m_loading = true;
    Hash.Remove(Create_Texture_Hash_Name(thumbnail->Get_Name()));
}

/**
 * Would have created thumbnails.
 */
void ThumbnailManagerClass::Create_Thumbnails()
{
    // Makes use of Renegade era MixFileFactory, probably not used.
    captainslog_dbgassert(false, "Called an unimplemented function.");
}

/**
 * Save thumbnails.
 */
void ThumbnailManagerClass::Save(bool force)
{
    // Stripped to minimum for Zero Hour, Renegade has functional version.
    if (m_loading || force) {
        m_loading = false;
    }
}

/**
 * Adds a thumbnail manager for a given thumbnail file.
 */
void ThumbnailManagerClass::Add_Thumbnail_Manager(const char *thumbfilename, const char *mixfilename)
{
    for (ThumbnailManagerClass *thumbnailmgr = ThumbnailManagerClass::s_thumbnailManagerList.Head(); thumbnailmgr != nullptr;
         thumbnailmgr = thumbnailmgr->Succ()) {
        if (strcmp(thumbnailmgr->m_thumbFileName, thumbfilename) == 0) {
            return;
        }
    }

    if (g_thumbnailManager == nullptr || strcmp(g_thumbnailManager->m_thumbFileName, thumbfilename) != 0) {
        Update_Thumbnail_File(mixfilename, false);
        ThumbnailManagerClass *manager = new ThumbnailManagerClass(thumbfilename, mixfilename);
        s_thumbnailManagerList.Add_Tail(manager);
    }
}

/**
 * Removes a thumbnailmanager for a given thumbnail file.
 */
void ThumbnailManagerClass::Remove_Thumbnail_Manager(const char *thumbfilename)
{
    for (ThumbnailManagerClass *thumbnailmgr = ThumbnailManagerClass::s_thumbnailManagerList.Head(); thumbnailmgr != nullptr;
         thumbnailmgr = thumbnailmgr->Succ()) {
        if (strcmp(thumbnailmgr->m_thumbFileName, thumbfilename) == 0) {
            delete thumbnailmgr;

            return;
        }
    }

    if (g_thumbnailManager != nullptr && strcmp(g_thumbnailManager->m_thumbFileName, thumbfilename) == 0) {
        delete g_thumbnailManager;
        g_thumbnailManager = nullptr;
    }
}

/**
 * Would have updated the managed thumbnail file.
 */
void ThumbnailManagerClass::Update_Thumbnail_File(const char *mixfilename, bool display_message)
{
    // All places where this is called appear to be ellided in windows build as called with null.
    captainslog_dbgassert(mixfilename == nullptr, "Code not implemented for string pointer not being null.");
}

/**
 * Retrieves the thumbnail manager for a given file.
 */
ThumbnailManagerClass *ThumbnailManagerClass::Peek_Thumbnail_Manager(const char *thumbfilename)
{
    for (ThumbnailManagerClass *thumbnailmgr = ThumbnailManagerClass::s_thumbnailManagerList.Head(); thumbnailmgr != nullptr;
         thumbnailmgr = thumbnailmgr->Succ()) {
        if (strcmp(thumbnailmgr->m_thumbFileName, thumbfilename) == 0) {
            return thumbnailmgr;
        }
    }

    if (g_thumbnailManager != nullptr && strcmp(g_thumbnailManager->m_thumbFileName, thumbfilename) == 0) {
        return g_thumbnailManager;
    }

    return nullptr;
}

/**
 * Retrieves a thumbnail from first thumbnail manager containing a matching thumbnail.
 *
 * 0x0086ABC0
 */
ThumbnailClass *ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager(const StringClass &texture)
{
    ThumbnailClass *thumb = nullptr;

    for (ThumbnailManagerClass *thumbnailmgr = ThumbnailManagerClass::s_thumbnailManagerList.Head(); thumbnailmgr != nullptr;
         thumbnailmgr = thumbnailmgr->Succ()) {
        thumb = thumbnailmgr->Peek_Thumbnail_Instance(texture);

        if (thumb != nullptr) {
            return thumb;
        }
    }

    if (g_thumbnailManager != nullptr) {
        thumb = g_thumbnailManager->Peek_Thumbnail_Instance(texture);

        if (thumb != nullptr) {
            return thumb;
        }
    }

    if (s_createIfNotFound && g_thumbnailManager != nullptr) {
        thumb = new ThumbnailClass(g_thumbnailManager, texture);

        if (thumb->Get_Bitmap() != nullptr) {
            return thumb;
        }

        delete thumb;
    }

    return nullptr;
}

/**
 * Initialises the global thumbnail manager.
 *
 * 0x0086AFE0
 */
void ThumbnailManagerClass::Init()
{
    g_thumbnailManager = new ThumbnailManagerClass("global.th6", nullptr);
    g_thumbnailManager->m_allowThumbnailCreation = true;
}

/**
 * Uninitialises the global thumbnail manager and the thumbnail manager list.
 *
 * 0x0086B070
 */
void ThumbnailManagerClass::Deinit()
{
    while (s_thumbnailManagerList.Head() != nullptr) {
        delete s_thumbnailManagerList.Head();
    }

    delete g_thumbnailManager;
    g_thumbnailManager = nullptr;
}
