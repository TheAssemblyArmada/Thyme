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
#include "globaldata.h"
#include "maputil.h"
#include <captainslog.h>

#ifndef GAME_DLL
GameState *g_theGameState = nullptr;
#endif

SaveGameInfo::SaveGameInfo() : m_missionNumber(0), m_saveFileType(SAVE_TYPE_UNK) {}

GameState::GameState() : m_availableGames(nullptr), m_isLoading(false) {}

GameState::~GameState()
{
    for (int i = 0; i < SNAPSHOT_TYPE_COUNT; i++) {
        m_snapShotBlocks[i].clear();
    }

    m_snapShots.clear();
    Clear_Available_Games();
}

void GameState::Init()
{
#ifdef GAME_DLL
    Call_Method<void, GameState>(PICK_ADDRESS(0x00492650, 0x007DF007), this);
#endif
}

void GameState::Reset()
{
    m_snapShots.clear();
    Clear_Available_Games();
}

void GameState::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00494D10, 0x007E1D91), this, xfer);
#endif
}

void GameState::Clear_Available_Games()
{
    while (m_availableGames != nullptr) {
        AvailableGameInfo *next = m_availableGames->m_next;

        delete m_availableGames;
        m_availableGames = next;
    }
}

Utf8String GameState::Get_Save_Dir() const
{
    Utf8String ret = g_theWriteableGlobalData->m_userDataDirectory;
    ret += "Save\\";

    return ret;
}

Utf8String GameState::Real_To_Portable_Map_Path(const Utf8String &path) const
{
    Utf8String ret;
    Utf8String ppath = path.Posix_Path();

    if (ppath.Starts_With_No_Case(Get_Save_Dir().Posix_Path().Str())) {
        ret = "Save/";
        ret += Get_File_From_Path(ppath);
    } else {
        if (ppath.Starts_With_No_Case(g_theMapCache->Get_Map_Dir().Str())) {
            ret = "Maps/";
            ret += Get_Leaf_And_Dir_Name(ppath);
        } else {
            if (ppath.Starts_With_No_Case(g_theMapCache->Get_User_Map_Dir().Str())) {
                ret = "UserData/Maps/";
                ret += Get_Leaf_And_Dir_Name(ppath);
            } else {
                ret = ppath;
            }
        }
    }

    ret.To_Lower();

    return ret.Windows_Path();
}

Utf8String GameState::Portable_To_Real_Map_Path(const Utf8String &path) const
{
    Utf8String ret;

    if (path.Starts_With_No_Case("Save/") || path.Starts_With_No_Case("Save\\")) {
        ret = Get_Save_Dir();
        ret += Get_File_From_Path(path);
    } else if (path.Starts_With_No_Case("Maps/") || path.Starts_With_No_Case("Maps\\")) {
        ret = g_theMapCache->Get_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else if (path.Starts_With_No_Case("UserData/Maps/") || path.Starts_With_No_Case("UserData\\Maps\\")) {
        ret = g_theMapCache->Get_User_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else {
        ret = path;
    }

    ret.To_Lower();

    return ret.Windows_Path();
}

// Returns file and last two containing directories from path in the form "path/to/file"
Utf8String Get_Leaf_And_Dir_Name(const Utf8String &path)
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

    return Utf8String(++leaf);
}

bool GameState::Is_In_Save_Dir(const Utf8String &path) const
{
    Utf8String str = Get_Save_Dir();
    return path.Starts_With_No_Case(str.Str());
}

void GameState::Friend_Xfer_Save_Data_For_CRC(Xfer *xfer, SnapShotType type)
{
    captainslog_debug("GameState::friend_xferSaveDataForCRC() - SnapshotType %d", type);
    SaveGameInfo *info = Get_Save_Info();
    info->m_saveDescription.Clear();
    info->m_saveFileType = SAVE_TYPE_UNK;
    info->m_mapPath2.Clear();
    info->m_pristineMapPath.Clear();
    Xfer_Save_Data(xfer, type);
}

void GameState::Xfer_Save_Data(Xfer *xfer, SnapShotType type)
{
    if (xfer == nullptr) {
        throw 4;
    }

    if (xfer->Get_Mode() == XFER_SAVE) {
        captainslog_debug("GameState::xferSaveData() - XFER_SAVE");
        Utf8String name;

        for (auto it = m_snapShotBlocks[type].begin(); it != m_snapShotBlocks[type].end(); it++) {
            name = it->m_name;
            captainslog_debug("Looking at block '%s'", name.Str());

            if (Get_Save_Info()->m_saveFileType != SAVE_TYPE_UNK2 || name.Compare_No_Case("CHUNK_GameState") == 0
                || name.Compare_No_Case("CHUNK_Campaign") == 0) {
                xfer->xferAsciiString(&name);
                xfer->Begin_Block();
                xfer->xferSnapshot(it->m_snapShot);
                xfer->End_Block();
            }
        }

        Utf8String eof("SG_EOF");
        xfer->xferAsciiString(&eof);
    } else {
        captainslog_debug("GameState::xferSaveData() - not XFER_SAVE");
        Utf8String name;
        bool done = false;

        while (!done) {
            xfer->xferAsciiString(&name);

            if (name.Compare_No_Case("SG_EOF") != 0) {
                SnapShotBlock *block = Find_Block_Info_By_Token(name, type);

                if (block != nullptr) {
                    xfer->Begin_Block();
                    xfer->xferSnapshot(block->m_snapShot);
                    xfer->End_Block();
                } else {
                    captainslog_debug("GameState::xferSaveData - Skipping unknown block '%s'", name.Str());
                    xfer->Skip(xfer->Begin_Block());
                }
            } else {
                done = true;
            }
        }
    }
}

GameState::SnapShotBlock *GameState::Find_Block_Info_By_Token(Utf8String name, SnapShotType type)
{
    if (name.Is_Empty()) {
        return nullptr;
    } else {
        for (auto it = m_snapShotBlocks[type].begin(); it != m_snapShotBlocks[type].end(); it++) {
            if (it->m_name == name) {
                return &*it;
            }
        }
    }

    return nullptr;
}
