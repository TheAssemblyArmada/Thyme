/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Game state tracker.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamestate.h"
#include "filetransfer.h"
#include "gamedebug.h"
#include "globaldata.h"
#include "maputil.h"

#ifndef THYME_STANDALONE
GameState *&g_theGameState = Make_Global<GameState *>(0x00A2BA04);
#else
GameState *g_theGameState = nullptr;
#endif

GameState::GameState() {}

GameState::~GameState() {}

void GameState::Init() {}

void GameState::Reset() {}

void GameState::Xfer_Snapshot(Xfer *xfer) {}

AsciiString GameState::Get_Save_Dir()
{
    AsciiString ret = g_theWriteableGlobalData->m_userDataDirectory;
    ret += "Save/";

    return ret;
}

AsciiString GameState::Real_To_Portable_Map_Path(const AsciiString &path)
{
    AsciiString ret;
    AsciiString ppath = path.Posix_Path();

    DEBUG_LOG("'%s' to portable:\n", path.Str());

    if (ppath.Starts_With_No_Case(Get_Save_Dir().Posix_Path())) {
        ret = "Save/";
        ret += Get_File_From_Path(ppath);
    } else {
        if (ppath.Starts_With_No_Case(TheMapCache->Get_Map_Dir().Str())) {
            ret = "Maps/";
            ret += Get_Leaf_And_Dir_Name(ppath);
        } else {
            if (ppath.Starts_With_No_Case(TheMapCache->Get_User_Map_Dir().Str())) {
                ret = "UserData/Maps/";
                ret += Get_Leaf_And_Dir_Name(ppath);
            } else {
                ret = ppath;
            }
        }
    }

    ret.To_Lower();
    DEBUG_LOG("    '%s'\n", ret.Windows_Path().Str());

    return ret.Windows_Path();
}

AsciiString GameState::Portable_To_Real_Map_Path(const AsciiString &path)
{
    AsciiString ret;

    DEBUG_LOG("'%s' to real:\n", path.Str());

    if (path.Starts_With_No_Case("Save/") || path.Starts_With_No_Case("Save\\")) {
        ret = Get_Save_Dir();
        ret += Get_File_From_Path(path);
    } else if (path.Starts_With_No_Case("Maps/") || path.Starts_With_No_Case("Maps\\")) {
        ret = TheMapCache->Get_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else if (path.Starts_With_No_Case("UserData/Maps/") || path.Starts_With_No_Case("UserData\\Maps\\")) {
        ret = TheMapCache->Get_User_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else {
        ret = path;
    }

    ret.To_Lower();
    DEBUG_LOG("    '%s'\n", ret.Windows_Path().Str());

    return ret.Windows_Path();
}

// Returns file and last two containing directories from path in the form "path/to/file"
AsciiString Get_Leaf_And_Dir_Name(const AsciiString &path)
{
    int len = path.Get_Length();
    const char *str = path.Str();
    const char *leaf = &str[len - 1];

    // Search backwards for first directory
    while (leaf >= str) {
        if (*leaf == '\\' || *leaf == '/') {
            break;
        }

        --leaf;
    }

    if (leaf == nullptr || leaf <= str) {
        return path;
    }

    --leaf;

    // Search backwards from last pos for the second directory.
    while (leaf > str) {
        if (*leaf == '\\' || *leaf == '/') {
            break;
        }

        --leaf;
    }

    if (leaf == nullptr || leaf <= str) {
        return path;
    }

    return AsciiString(++leaf);
}
