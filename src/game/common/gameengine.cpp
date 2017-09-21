/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Interface for the game engine implementation.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameengine.h"
#include "archivefilesystem.h"
#include "audiomanager.h"
#include "commandline.h"
#include "commandlist.h"
#include "filesystem.h"
#include "functionlexicon.h"
#include "gamelod.h"
#include "gametext.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "ini.h"
#include "localfilesystem.h"
#include "messagestream.h"
#include "modulefactory.h"
#include "multiplayersettings.h"
#include "namekeygenerator.h"
#include "randomvalue.h"
#include "science.h"
#include "subsysteminterface.h"
#include "terrainroads.h"
#include "terraintypes.h"
#include "xfercrc.h"

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#endif

#ifndef THYME_STANDALONE
GameEngine *&g_theGameEngine = Make_Global<GameEngine *>(0x00A29B80);
#else
GameEngine *g_theGameEngine = nullptr;
#endif

GameEngine::GameEngine() : m_maxFPS(0), m_isQuitting(false), m_isActive(false)
{
#ifdef PLATFORM_WINDOWS
    timeBeginPeriod(1);
#endif
}

GameEngine::~GameEngine()
{
// TODO

#ifdef PLATFORM_WINDOWS
    timeEndPeriod(1);
#endif
}

void GameEngine::Reset() {}

void GameEngine::Update() {}

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
    g_theSubsystemList->Init_Subsystem(g_theWriteableGlobalData,
        "Data/INI/Default/GameData.ini",
        "Data/INI/GameData.ini",
        nullptr,
        &xfer,
        "TheWriteableGlobalData");

    Parse_Command_Line(argc, argv);
    g_theGameLODManager = new GameLODManager;
    g_theGameLODManager->Init();

    // if ( g_theWriteableGlobalData->m_updateTGAtoDDS ) {
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
    g_theSubsystemList->Init_Subsystem(g_theGameText, nullptr, nullptr, nullptr, nullptr, "TheGameText");

    g_theScienceStore = new ScienceStore;
    g_theSubsystemList->Init_Subsystem(
        g_theScienceStore, "Data/INI/Default/Science.ini", "Data/INI/Science.ini", nullptr, &xfer, "TheScienceStore");

    g_theMultiplayerSettings = new MultiplayerSettings;
    g_theSubsystemList->Init_Subsystem(g_theMultiplayerSettings,
        "Data/INI/Default/Multiplayer.ini",
        "Data/INI/Multiplayer.ini",
        nullptr,
        &xfer,
        "TheMultiplayerSettings");

    g_theTerrainTypes = new TerrainTypeCollection;
    g_theSubsystemList->Init_Subsystem(
        g_theTerrainTypes, "Data/INI/Default/Terrain.ini", "Data/INI/Terrain.ini", nullptr, &xfer, "TheTerrainTypes");

    g_theTerrainRoads = new TerrainRoadCollection;
    g_theSubsystemList->Init_Subsystem(
        g_theTerrainRoads, "Data/INI/Default/Roads.ini", "Data/INI/Roads.ini", nullptr, &xfer, "TheTerrainRoads");

    g_theGlobalLanguage = new GlobalLanguage;
    g_theSubsystemList->Init_Subsystem(g_theGlobalLanguage, nullptr, nullptr, nullptr, nullptr, "TheGlobalLanguageData");

    g_theAudio = Create_Audio_Manager();
    g_theSubsystemList->Init_Subsystem(g_theAudio, nullptr, nullptr, nullptr, nullptr, "TheAudio");

    if (!g_theAudio->Is_Music_Already_Loaded()) {
        Set_Quitting(true);
    }

    g_theFunctionLexicon = Create_Function_Lexicon();
    g_theSubsystemList->Init_Subsystem(g_theFunctionLexicon, nullptr, nullptr, nullptr, nullptr, "TheFunctionLexicon");
    
    g_theModuleFactory = Create_Module_Factory();
    g_theSubsystemList->Init_Subsystem(g_theModuleFactory, nullptr, nullptr, nullptr, nullptr, "TheModuleFactory");

    g_theMessageStream = Create_Message_Stream();
    g_theSubsystemList->Init_Subsystem(g_theMessageStream, nullptr, nullptr, nullptr, nullptr, "TheMessageStream");
    
    // TODO this is a WIP
}

void GameEngine::Execute() {}

bool GameEngine::Is_Multiplayer_Session()
{
    return false;
}

FileSystem *GameEngine::Create_File_System()
{
    return new FileSystem;
}

MessageStream *GameEngine::Create_Message_Stream()
{
    return Call_Method<MessageStream*, GameEngine>(0x0040FF00, this);
}
