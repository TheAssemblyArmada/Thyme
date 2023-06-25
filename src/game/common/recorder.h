/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Recorder
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
#include "gameinfo.h"
#include "gametype.h"
#include "subsysteminterface.h"

class CRCInfo;

enum RecorderModeType
{
    RECORDERMODETYPE_RECORD,
    RECORDERMODETYPE_PLAYBACK,
};

class ReplayGameInfo : public GameInfo
{
private:
    GameSlot m_ReplaySlot[8];
};

class RecorderClass : public SubsystemInterface
{
public:
    virtual ~RecorderClass() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    bool Playback_File(Utf8String name);
    bool Is_Multiplayer();
    void Init_Controls();
    RecorderModeType Get_Mode() const { return m_mode; }
    GameMode Get_Org_Game_Mode() const { return m_orgGameMode; }
    GameInfo *Get_Game_Info() { return &m_gameInfo; }

private:
    CRCInfo *m_crcInfo;
    FILE *m_file;
    Utf8String m_fileName;
    int m_currentFilePosition;
    RecorderModeType m_mode;
    Utf8String m_unk;
    ReplayGameInfo m_gameInfo;
    bool m_wasDesync;
    bool m_unk2;
    GameMode m_orgGameMode;
    unsigned int m_nextFrame;
};

RecorderClass *Create_Recorder();

#ifdef GAME_DLL
extern RecorderClass *&g_theRecorder;
extern int &g_replayCRCInterval;
#else
extern RecorderClass *g_theRecorder;
extern int g_replayCRCInterval;
#endif
