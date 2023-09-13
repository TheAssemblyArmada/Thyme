/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief W3DLib style wrapper around filesystem.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dfilesystem.h"
#include "asciistring.h"
#include "filesystem.h"
#include "globaldata.h"
#include "registryget.h"
#include <cstdio>

#ifndef GAME_DLL
W3DFileSystem *g_theW3DFileSystem;
#endif

W3DFileSystem::W3DFileSystem()
{
    g_theFileFactory = this;
}

W3DFileSystem::~W3DFileSystem()
{
    g_theFileFactory = nullptr;
}

FileClass *W3DFileSystem::Get_File(const char *filename)
{
    return new GameFileClass(filename);
}

void W3DFileSystem::Return_File(FileClass *file)
{
    if (file != nullptr) {
        delete file;
    }
}

GameFileClass::GameFileClass(const char *filename) : m_theFile(nullptr), m_fileExists(false)
{
    m_filePath[0] = '\0';
    m_filename[0] = '\0';

    if (filename != nullptr) {
        Set_Name(filename);
    }
}

GameFileClass::~GameFileClass()
{
    Close();
}

const char *GameFileClass::File_Name()
{
    return m_filename;
}

const char *GameFileClass::Set_Name(const char *filename)
{
    if (Is_Open()) {
        Close();
    }

    char buff[PATH_MAX] = {};
    char ext[32] = {};

    strlcpy_tpl(m_filename, filename);
    strlcpy_tpl(buff, filename);

    int count = 1;
    for (size_t i = strlen(buff) - 1; i > 0; --i) {
        if (count >= 32) {
            break;
        }

        if (buff[i] == '.') {
            strlcpy_tpl(ext, &buff[i]);
            buff[i] = '\0';

            break;
        }
        ++count;
    }

    // Strip spaces.
    int put = 0;
    for (int get = 0; buff[get] != '\0'; ++get) {
        if (buff[get] != ' ') {
            buff[put++] = buff[get];
        }
    }

    buff[put] = '\0';

    GameFileType file_type = GAME_FILE_UNK;

    // Handle special paths for certain asset types
    if (strcasecmp(ext, ".w3d") == 0) {
        file_type = GAME_FILE_W3D;
        snprintf(m_filePath, sizeof(m_filePath), "Data/%s/Art/W3D/", Get_Registry_Language().Str());
        strlcat_tpl(m_filePath, filename);
    } else {
        if (strcasecmp(ext, ".tga") == 0) {
            file_type = GAME_FILE_TGA;
        }

        if (strcasecmp(ext, ".dds") == 0) {
            file_type = GAME_FILE_DDS;
        }

        if (file_type != GAME_FILE_UNK) {
            snprintf(m_filePath, sizeof(m_filePath), "Data/%s/Art/Textures/", Get_Registry_Language().Str());

            strlcat_tpl(m_filePath, filename);
        }
    }

    m_fileExists = g_theFileSystem->Does_File_Exist(m_filePath);

    if (!m_fileExists) {
        switch (file_type) {
            case GAME_FILE_W3D:
                strlcpy_tpl(m_filePath, "Art/W3D/");
                strlcat_tpl(m_filePath, filename);
                break;
            case GAME_FILE_TGA:
            case GAME_FILE_DDS: // Fallthrough
                strlcpy_tpl(m_filePath, "Art/Textures/");
                strlcat_tpl(m_filePath, filename);
                break;
            default:
                strlcpy_tpl(m_filePath, filename);
                break;
        }

        m_fileExists = g_theFileSystem->Does_File_Exist(m_filePath);
    }

    if (!m_fileExists) {
        switch (file_type) {
            case GAME_FILE_W3D:
            case GAME_FILE_TGA:
            case GAME_FILE_DDS: // Fallthrough
                strlcpy_tpl(m_filePath, "../TestArt/");
                strlcat_tpl(m_filePath, filename);
                break;
            default:
                break;
        }

        m_fileExists = g_theFileSystem->Does_File_Exist(m_filePath);
    }

    if (!m_fileExists && g_theWriteableGlobalData != nullptr) {
        switch (file_type) {
            case GAME_FILE_W3D:
                snprintf(m_filePath, sizeof(m_filePath), "%sW3D/", g_theWriteableGlobalData->m_userDataDirectory.Str());
                strlcat_tpl(m_filePath, filename);
                break;
            case GAME_FILE_TGA:
            case GAME_FILE_DDS: // Fallthrough
                snprintf(m_filePath, sizeof(m_filePath), "%sTextures/", g_theWriteableGlobalData->m_userDataDirectory.Str());
                strlcat_tpl(m_filePath, filename);
                break;
            default:
                break;
        }

        m_fileExists = g_theFileSystem->Does_File_Exist(m_filePath);
    }

    if (!m_fileExists && g_theWriteableGlobalData != nullptr) {
        switch (file_type) {
            case GAME_FILE_TGA:
                // TODO Allow DDS for map previews as well at some point?
                // case GAME_FILE_DDS: //Fallthrough
                snprintf(
                    m_filePath, sizeof(m_filePath), "%sMapPreviews/", g_theWriteableGlobalData->m_userDataDirectory.Str());
                strlcat_tpl(m_filePath, filename);
                break;
            default:
                break;
        }

        m_fileExists = g_theFileSystem->Does_File_Exist(m_filePath);
    }

    return m_filename;
}

bool GameFileClass::Create()
{
    return true;
}

bool GameFileClass::Delete()
{
    return true;
}

bool GameFileClass::Is_Available(bool forced)
{
    return m_fileExists;
}

bool GameFileClass::Is_Open()
{
    return m_theFile != nullptr;
}

bool GameFileClass::Open(const char *filename, int rights)
{
    Set_Name(filename);

    if (!Is_Available()) {
        return false;
    }

    return Open(rights);
}

bool GameFileClass::Open(int rights)
{
    if (rights != FM_READ) {
        return false;
    }

    m_theFile = g_theFileSystem->Open_File(m_filePath, File::READ | File::BINARY);

    return m_theFile != nullptr;
}

int GameFileClass::Read(void *buffer, int length)
{
    if (m_theFile == nullptr) {
        return 0;
    }

    return m_theFile->Read(buffer, length);
}

off_t GameFileClass::Seek(off_t offset, int whence)
{
    File::SeekMode file_whence;

    if (m_theFile == nullptr) {
        return -1;
    }

    switch (whence) {
        case FS_SEEK_START:
            file_whence = File::START;
            break;
        case FS_SEEK_END:
            file_whence = File::END;
            break;
        case FS_SEEK_CURRENT:
        default:
            file_whence = File::CURRENT;
            break;
    }

    return m_theFile->Seek(offset, file_whence);
}

off_t GameFileClass::Size()
{
    if (m_theFile == nullptr) {
        return -1;
    }

    return m_theFile->Size();
}

int GameFileClass::Write(void const *buffer, int size)
{
    return 0;
}

void GameFileClass::Close()
{
    if (m_theFile != nullptr) {
        m_theFile->Close();
        m_theFile = nullptr;
    }
}
