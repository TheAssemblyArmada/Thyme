/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for the game engine implementation.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameengine.h"
#include "archivefilesystem.h"
#include "armor.h"
#include "audiomanager.h"
#include "cavesystem.h"
#include "cdmanager.h"
#include "commandline.h"
#include "commandlist.h"
#include "damagefx.h"
#include "filesystem.h"
#include "functionlexicon.h"
#include "fxlist.h"
#include "gamelod.h"
#include "gametext_impl.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "ini.h"
#include "localfilesystem.h"
#include "locomotor.h"
#include "messagestream.h"
#include "modulefactory.h"
#include "multiplayersettings.h"
#include "namekeygenerator.h"
#include "objectcreationlist.h"
#include "particlesysmanager.h"
#include "playertemplate.h"
#include "randomvalue.h"
#include "rankinfo.h"
#include "science.h"
#include "sideslist.h"
#include "specialpower.h"
#include "subsysteminterface.h"
#include "terrainroads.h"
#include "terraintypes.h"
#include "version.h"
#include "weapon.h"
#include "xfercrc.h"

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#endif

#ifndef GAME_DLL
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

#ifdef GAME_DEBUG_STRUCTS
    if (g_theVersion != nullptr) {
        captainslog_debug("================================================================================");
        captainslog_debug("Thyme version %s (%s)", g_theVersion->Get_Ascii_Version().Str(), "Internal");
        captainslog_debug("Build date: %s", g_theVersion->Get_Ascii_Build_Time().Str());
        captainslog_debug("Branch: %s", g_theVersion->Get_Ascii_Branch().Str());
        captainslog_debug("Build commit: %s", g_theVersion->Get_Ascii_Commit_Hash().Str());
        captainslog_debug("================================================================================");
    }

    captainslog_debug("Calculating CPU frequency for performance timers.");
    // TODO processor frequency stuff
#endif

    m_maxFPS = 45;

    g_theSubsystemList = new SubsystemInterfaceList;
    g_theSubsystemList->Add_Subsystem(this);

    Init_Random();

    g_theFileSystem = Create_File_System();

    // this used to exist here
    // DeleteFile("Data\\INI\\INIZH.big");

    g_theNameKeyGenerator = new NameKeyGenerator;
    g_theNameKeyGenerator->Init();
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheNameKeyGenerator  = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    g_theCommandList = new CommandList;
    g_theCommandList->Init();
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheCommandList  = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    XferCRC xfer;
    xfer.Open("lightCRC");

    Init_Subsystem(g_theLocalFileSystem, "TheLocalFileSystem", Create_Local_File_System());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After TheLocalFileSystem "
                      " = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theArchiveFileSystem, "TheArchiveFileSystem", Create_Archive_File_System());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheArchiveFileSystem  = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theWriteableGlobalData,
        "TheWriteableGlobalData",
        new GlobalData,
        &xfer,
        "Data/INI/Default/GameData.ini",
        "Data/INI/GameData.ini");

#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After  "
                      "TheWritableGlobalData = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

#ifdef GAME_DEBUG_STRUCTS
    ini.Load("Data/INI/GameDataDebug.ini", INI_LOAD_OVERWRITE, nullptr);
#endif

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
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After water INI's = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

#ifdef GAME_DEBUG_STRUCTS
    // TODO TheDeepCRCSanityCheck
#endif

    // Text manager isn't controlled by ini files, it uses either a csf or str file.
    Init_Subsystem(g_theGameText, "TheGameText", GameTextManager::Create_Game_Text_Interface());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheGameText = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theScienceStore,
        "TheScienceStore",
        new ScienceStore,
        &xfer,
        "Data/INI/Default/Science.ini",
        "Data/INI/Science.ini");
    Init_Subsystem(g_theMultiplayerSettings,
        "TheMultiplayerSettings",
        new MultiplayerSettings,
        &xfer,
        "Data/INI/Default/Multiplayer.ini",
        "Data/INI/Multiplayer.ini");
    Init_Subsystem(g_theTerrainTypes,
        "TheTerrainTypes",
        new TerrainTypeCollection,
        &xfer,
        "Data/INI/Default/Terrain.ini",
        "Data/INI/Terrain.ini");
    Init_Subsystem(g_theTerrainRoads,
        "TheTerrainRoads",
        new TerrainRoadCollection,
        &xfer,
        "Data/INI/Default/Roads.ini",
        "Data/INI/Roads.ini");
    Init_Subsystem(g_theGlobalLanguage, "TheGlobalLanguageData", new GlobalLanguage);

    Init_Subsystem(g_theCDManager, "TheCDManager", Create_CD_Manager());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheCDManager = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theAudio, "TheAudio", Create_Audio_Manager());

    if (!g_theAudio->Is_Music_Already_Loaded()) {
        Set_Quitting(true);
    }
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheAudio = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theFunctionLexicon, "TheFunctionLexicon", Create_Function_Lexicon());
    Init_Subsystem(g_theModuleFactory, "TheModuleFactory", Create_Module_Factory());
    Init_Subsystem(g_theMessageStream, "TheMessageStream", Create_Message_Stream());
    Init_Subsystem(g_theSidesList, "TheSidesList", new SidesList);
    Init_Subsystem(g_theCaveSystem, "TheCaveSystem", new CaveSystem);
    Init_Subsystem(g_theRankInfoStore, "TheRankInfoStore", new RankInfoStore, &xfer, nullptr, "Data/INI/Rank.ini");
    Init_Subsystem(g_thePlayerTemplateStore,
        "ThePlayerTemplateStore",
        new PlayerTemplateStore,
        &xfer,
        "Data/INI/Default/PlayerTemplate.ini",
        "Data/INI/PlayerTemplate.ini");

    Init_Subsystem(g_theParticleSystemManager, "TheParticleSystemManager", Create_Particle_System_Manager());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheParticleSystemManager = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(
        g_theFXListStore, "TheFXListStore", new FXListStore, &xfer, "Data/INI/Default/FXList.ini", "Data/INI/FXList.ini");
    Init_Subsystem(g_theWeaponStore, "TheWeaponStore", new WeaponStore, &xfer, nullptr, "Data/INI/Weapon.ini");
    Init_Subsystem(g_theObjectCreationListStore,
        "TheObjectCreationListStore",
        new ObjectCreationListStore,
        &xfer,
        "Data/INI/Default/ObjectCreationList.ini",
        "Data/INI/ObjectCreationList.ini");
    Init_Subsystem(g_theLocomotorStore, "TheLocomotorStore", new LocomotorStore, &xfer, nullptr, "Data/INI/Locomotor.ini");
    Init_Subsystem(g_theSpecialPowerStore,
        "TheSpecialPowerStore",
        new SpecialPowerStore,
        &xfer,
        "Data/INI/Default/SpecialPower.ini",
        "Data/INI/SpecialPower.ini");
    Init_Subsystem(g_theDamageFXStore, "TheDamageFXStore", new DamageFXStore, &xfer, nullptr, "Data/INI/DamageFX.ini");
    Init_Subsystem(g_theArmorStore, "TheArmorStore", new ArmorStore, &xfer, nullptr, "Data/INI/Armor.ini");

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
    return new MessageStream;
}
