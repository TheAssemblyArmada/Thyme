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
#pragma once

#include "always.h"
#include "dllist.h"
#include "hashtemplate.h"
#include "wwstring.h"

class ThumbnailClass;

class ThumbnailManagerClass : public DLNodeClass<ThumbnailManagerClass>
{
    IMPLEMENT_W3D_POOL(ThumbnailManagerClass)

public:
    ThumbnailManagerClass(const char *thumbfilename, const char *mixfilename);
    virtual ~ThumbnailManagerClass();
    ThumbnailClass *Peek_Thumbnail_Instance(const StringClass &texture);
    void Insert_To_Hash(ThumbnailClass *thumbnail);
    ThumbnailClass *Get_From_Hash(const StringClass &texture);
    void Remove_From_Hash(ThumbnailClass *thumbnail);
    void Create_Thumbnails();
    void Load()
    { /*Empty in Zero Hour*/
    }
    void Save(bool force);

    static void Add_Thumbnail_Manager(const char *thumbfilename, const char *mixfilename);
    static void Remove_Thumbnail_Manager(const char *thumbfilename);
    static void Update_Thumbnail_File(const char *thumbfilename, bool display_message);
    static ThumbnailManagerClass *Peek_Thumbnail_Manager(const char *thumbfilename);
    static ThumbnailClass *Peek_Thumbnail_Instance_From_Any_Manager(const StringClass &texture);
    static void Init();
    static void Deinit();
    static void Pre_Init(bool b)
    { /*Looks left over from Renegade, references .mix files*/
    }

private:
    bool m_allowThumbnailCreation;
    StringClass m_thumbFileName;
    StringClass m_mixFileName;
    HashTemplateClass<StringClass, ThumbnailClass *> Hash;
    uint8_t *m_bitmap;
    bool m_loading;
    unsigned m_time;

    static bool s_createIfNotFound;
#ifdef GAME_DLL
    static DLListClass<ThumbnailManagerClass> &s_thumbnailManagerList;
#else
    static DLListClass<ThumbnailManagerClass> s_thumbnailManagerList;
#endif
};

#ifdef GAME_DLL
extern ThumbnailManagerClass *&g_thumbnailManager;
#else
extern ThumbnailManagerClass *g_thumbnailManager;
#endif
