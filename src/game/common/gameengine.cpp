////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEENGINE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for the game engine implementation.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "gameengine.h"
#include "archivefilesystem.h"
#include "commandline.h"
#include "commandlist.h"
#include "filesystem.h"
#include "gamelod.h"
#include "gametext.h"
#include "globaldata.h"
#include "ini.h"
#include "localfilesystem.h"
#include "namekeygenerator.h"
#include "randomvalue.h"
#include "science.h"
#include "subsysteminterface.h"
#include "xfercrc.h"

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#endif

//GameEngine *g_theGameEngine = nullptr;

GameEngine::GameEngine() :
    MaxFPS(0),
    Quitting(false),
    Active(false)
{
#ifdef PLATFORM_WINDOWS
    timeBeginPeriod(1);
#endif
}

GameEngine::~GameEngine()
{
    //TODO

#ifdef PLATFORM_WINDOWS
    timeEndPeriod(1);
#endif
}

void GameEngine::Reset()
{
}

void GameEngine::Update()
{
}

void GameEngine::Init(int argc, char *argv[])
{
    INI ini;

    g_theSubsystemList = new SubsystemInterfaceList;
    Init_Random();
    g_theFileSystem = Create_File_System();
    g_theNameKeyGenerator = new NameKeyGenerator;
    g_theNameKeyGenerator->Init();
    g_theCommandList = new CommandList;
    g_theCommandList->Init();

    XferCRC xfer;
    xfer.Open("lightCRC");

    g_theLocalFileSystem = Create_Local_File_System();
    g_theSubsystemList->Init_Subsystem(g_theLocalFileSystem, nullptr, nullptr, nullptr, &xfer, "TheLocalFileSystem");
    g_theArchiveFileSystem = Create_Archive_File_System();
    g_theSubsystemList->Init_Subsystem(g_theArchiveFileSystem, nullptr, nullptr, nullptr, &xfer, "TheArchiveFileSystem");
    
    g_theWriteableGlobalData = new GlobalData;
    g_theSubsystemList->Init_Subsystem(
        g_theWriteableGlobalData,
        "Data/INI/Default/GameData.ini",
        "Data/INI/GameData.ini",
        nullptr,
        &xfer,
        "TheWriteableGlobalData"
    );

    Parse_Command_Line(argc, argv);
    g_theGameLODManager = new GameLODManager;
    g_theGameLODManager->Init();

    //if ( g_theWriteableGlobalData->m_updateTGAtoDDS ) {
        // Windows version looks for a file called buildDDS.txt to get a list of
        // TGA format textures to convert to DDS and then calls nvdxt as follows
        // "..\\Build\\nvdxt -list buildDDS.txt -dxt5 -full -outdir Art\\Textures > buildDDS.out"
        // There seems little point in keeping this as 1. its not cross platform and 2. most
        // textures are DDS in the release build anyhow.
    //}

    ini.Load("Data/INI/Default/Water.ini", INI_LOAD_OVERWRITE, &xfer);
    ini.Load("Data/INI/Water.ini", INI_LOAD_OVERWRITE, &xfer);
    ini.Load("Data/INI/Default/Weather.ini", INI_LOAD_OVERWRITE, &xfer);
    ini.Load("Data/INI/Weather.ini", INI_LOAD_OVERWRITE, &xfer);

    // Text manager isn't controlled by ini files, it uses either a csf or str file.
    g_theGameText = GameTextManager::Create_Game_Text_Interface();
    g_theSubsystemList->Init_Subsystem(
        g_theGameText,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "TheGameText"
    );

    g_theScienceStore = new ScienceStore;
    g_theSubsystemList->Init_Subsystem(
        g_theScienceStore,
        "Data\\INI\\Default\\Science.ini",
        "Data\\INI\\Science.ini",
        nullptr,
        &xfer,
        "TheScienceStore"
    );

    //TODO this is a WIP
}

void GameEngine::Execute()
{
}

void GameEngine::Set_FPS_Limit(int limit)
{
}

int GameEngine::Get_FPS_Limit()
{
    return 0;
}

void GameEngine::Set_Quitting(bool quitting)
{
}

void GameEngine::Get_Quitting()
{
}

bool GameEngine::Is_Multiplayer_Session()
{
    return false;
}

void GameEngine::Service_Windows_OS()
{
}

bool GameEngine::Get_Is_Active()
{
    return false;
}

void GameEngine::Set_Is_Active()
{
}

FileSystem *GameEngine::Create_File_System()
{
    return nullptr;
}

MessageStream *GameEngine::Create_Message_Stream()
{
    return nullptr;
}
