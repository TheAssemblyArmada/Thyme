/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Filesystem abstraction merging local and archive file handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "file.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <map>
#include <set>

#ifdef GAME_DLL
#include "hooker.h"
#endif

class FileSystem : public SubsystemInterface
{
public:
    FileSystem() : m_availableFiles() {}
    virtual ~FileSystem() {}

    // SubsystemInterface implementations
    virtual void Init();
    virtual void Reset();
    virtual void Update();

    // Filesystem
    File *Open(const char *filename, int mode);
    bool Does_File_Exist(const char *filename);
    void Get_File_List_From_Dir(Utf8String const &dir, Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist, bool a5);

    static bool Create_Dir(Utf8String name);
    static bool Are_Music_Files_On_CD();
    static bool Load_Music_Files_From_CD();
    static void Unload_Music_Files_From_CD();

#ifdef GAME_DLL
    static void Hook_Me();
#endif
private:
    std::map<unsigned int, bool> m_availableFiles;
};

#ifdef GAME_DLL
extern FileSystem *&g_theFileSystem;
inline void FileSystem::Hook_Me()
{
    Hook_Function(0x004469C0, Are_Music_Files_On_CD);
    Hook_Method(0x00446770, &Get_File_List_From_Dir);
}
#else
extern FileSystem *g_theFileSystem;
#endif