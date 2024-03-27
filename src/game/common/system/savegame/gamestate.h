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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "xfer.h"

struct SaveDate
{
    unsigned short year;
    unsigned short month;
    unsigned short day_of_week;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milliseconds;
    SaveDate() : year(0), month(0), day_of_week(0), day(0), hour(0), minute(0), second(0), milliseconds(0) {}
};

enum SaveFileType
{
    SAVE_TYPE_UNK,
    SAVE_TYPE_UNK2,
};

enum SnapShotType
{
    SNAPSHOT_TYPE_UNK1,
    SNAPSHOT_TYPE_UNK2,
    SNAPSHOT_TYPE_UNK3,
    SNAPSHOT_TYPE_COUNT,
};

class SaveGameInfo
{
public:
    SaveGameInfo();

    Utf8String m_filePath; // not 100% confirmed
    Utf8String m_pristineMapPath; // not 100% confirmed
    Utf8String m_mapPath; // not 100% confirmed
    SaveDate m_saveDate; // confirmed
    Utf8String m_campaign; // confirmed
    int m_missionNumber; // confirmed
    Utf16String m_saveDescription; // confirmed
    SaveFileType m_saveFileType; // confirmed
    Utf8String m_mapPath2; // not 100% confirmed
};

struct AvailableGameInfo
{
    Utf8String m_fileName;
    SaveGameInfo m_saveInfo;
    AvailableGameInfo *m_next;
    AvailableGameInfo *m_prev;
};

class GameState : public SubsystemInterface, public SnapShot
{
public:
    GameState();
    virtual ~GameState();

    // SubsystemInterface implementations.
    virtual void Init();
    virtual void Reset();
    virtual void Update() {}

    // SnapShot implementations.
    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    void Clear_Available_Games();

    Utf8String Get_Save_Dir() const;
    Utf8String Real_To_Portable_Map_Path(const Utf8String &path) const;
    Utf8String Portable_To_Real_Map_Path(const Utf8String &path) const;
    bool Is_In_Save_Dir(const Utf8String &path) const;
    void Friend_Xfer_Save_Data_For_CRC(Xfer *xfer, SnapShotType type);
    void Xfer_Save_Data(Xfer *xfer, SnapShotType type);

    bool Is_Loading() const { return m_isLoading; }
    void Set_Pristine_Map_Name(Utf8String path) { m_saveInfo.m_pristineMapPath = path; }
    SaveGameInfo *Get_Save_Info() { return &m_saveInfo; }

private:
    struct SnapShotBlock
    {
        SnapShot *m_snapShot;
        Utf8String m_name;
    };

    SnapShotBlock *Find_Block_Info_By_Token(Utf8String name, SnapShotType type);

    std::list<SnapShotBlock> m_snapShotBlocks[SNAPSHOT_TYPE_COUNT];
    SaveGameInfo m_saveInfo;
    std::list<SnapShot *> m_snapShots;
    AvailableGameInfo *m_availableGames;
    bool m_isLoading;
};

Utf8String Get_Leaf_And_Dir_Name(const Utf8String &path);

#ifdef GAME_DLL
extern GameState *&g_theGameState;
#else
extern GameState *g_theGameState;
#endif
