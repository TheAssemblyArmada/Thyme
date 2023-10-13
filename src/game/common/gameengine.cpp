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
#include "actionmanager.h"
#include "ai.h"
#include "archivefilesystem.h"
#include "armor.h"
#include "audiomanager.h"
#include "buildassistant.h"
#include "cavesystem.h"
#include "cdmanager.h"
#include "commandline.h"
#include "commandlist.h"
#include "cratesystem.h"
#include "damage.h"
#include "damagefx.h"
#include "disabledtypes.h"
#include "drawable.h"
#include "filesystem.h"
#include "functionlexicon.h"
#include "fxlist.h"
#include "gameclient.h"
#include "gamelod.h"
#include "gamelogic.h"
#include "gameresultsthread.h"
#include "gamestate.h"
#include "gamestatemap.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "globallanguage.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "ini.h"
#include "kindof.h"
#include "localfilesystem.h"
#include "locomotor.h"
#include "maputil.h"
#include "messagestream.h"
#include "metaevent.h"
#include "modulefactory.h"
#include "multiplayersettings.h"
#include "namekeygenerator.h"
#include "network.h"
#include "objectcreationlist.h"
#include "particlesysmanager.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "radar.h"
#include "randomvalue.h"
#include "rankinfo.h"
#include "recorder.h"
#include "registryget.h"
#include "science.h"
#include "scriptengine.h"
#include "sideslist.h"
#include "specialpower.h"
#include "subsysteminterface.h"
#include "team.h"
#include "terrainroads.h"
#include "terraintypes.h"
#include "thingfactory.h"
#include "upgrade.h"
#include "version.h"
#include "victoryconditions.h"
#include "view.h"
#include "weapon.h"
#include "windowlayout.h"
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

    // un-necessary CComModule::Init call omitted
}

GameEngine::~GameEngine()
{
    delete g_theMapCache;
    g_theMapCache = nullptr;

    delete g_theGameResultsQueue;
    g_theGameResultsQueue = nullptr;

    g_theSubsystemList->Shutdown_All();
    delete g_theSubsystemList;
    g_theSubsystemList = nullptr;

    delete g_theNetwork;
    g_theNetwork = nullptr;

    delete g_theCommandList;
    g_theCommandList = nullptr;

    delete g_theNameKeyGenerator;
    g_theNameKeyGenerator = nullptr;

    delete g_theFileSystem;
    g_theFileSystem = nullptr;

    delete g_theGameLODManager;
    g_theGameLODManager = nullptr;
    Drawable::Kill_Static_Images();

    // un-necessary CComModule::~CComModule call omitted

#ifdef PLATFORM_WINDOWS
    timeEndPeriod(1);
#endif
}

void GameEngine::Reset()
{
    WindowLayout *layout = g_theWindowManager->Win_Create_Layout("Menus/BlankWindow.wnd");
    captainslog_dbgassert(layout != nullptr, "We Couldn't Load Menus/BlankWindow.wnd");
    layout->Hide(false);
    layout->Bring_Forward();
    GameWindow *window = layout->Get_Window_List();
    window->Win_Clear_Status(WIN_STATUS_IMAGE);
    bool multiplayer = g_theGameLogic->Is_In_Multiplayer_Game();
    g_theSubsystemList->Reset_All();

    if (multiplayer) {
        captainslog_dbgassert(g_theNetwork != nullptr, "Deleting NULL TheNetwork!");
        delete g_theNetwork;
        g_theNetwork = nullptr;
    }

    if (layout != nullptr) {
        layout->Destroy_Windows();
        layout->Delete_Instance();
    }
}

void GameEngine::Update()
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO CRCVerification
#endif

    // TODO WB has extra subsystem debug stuff here, not implemented yet
    g_theRadar->Update();
    g_theAudio->Update();
    g_theGameClient->Update();
    g_theMessageStream->Propagate_Messages();

    if (g_theNetwork != nullptr) {
        g_theNetwork->Update();
    }

    g_theCDManager->Update();

    if ((g_theNetwork == nullptr && !g_theGameLogic->Is_Game_Paused())
        || (g_theNetwork != nullptr && g_theNetwork->Is_Frame_Data_Ready())) {
        g_theGameLogic->Update();
    }
}

void GameEngine::Init(int argc, char *argv[])
{
    Real_Init(argc, argv);
}

void GameEngine::Real_Init(int argc, char *argv[])
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

#if defined GAME_DEBUG_STRUCTS && defined GAME_DLL
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

    // We don't support most after this
#ifndef GAME_DLL
#pragma message("SKIPPING NOT YET SUPPORTED SUBSYSTEMS !!!")
    captainslog_info("Skipping not yet supported subsystems");
    return;
#endif

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

    Init_Subsystem(g_theBuildAssistant, "TheBuildAssistant", new BuildAssistant);
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheBuildAssistant = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theThingFactory,
        "TheThingFactory",
        new ThingFactory,
        &xfer,
        "Data/INI/Default/Object.ini",
        nullptr,
        "Data/INI/Object");
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheThingFactory = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theUpgradeCenter,
        "TheUpgradeCenter",
        new UpgradeCenter,
        &xfer,
        "Data/INI/Default/Upgrade.ini",
        "Data/INI/Upgrade.ini");

    Init_Subsystem(g_theGameClient, "TheGameClient", Create_Game_Client());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug(
        "----------------------------------------------------------------------------After TheGameClient = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Init_Subsystem(g_theAI, "TheAI", new AI, &xfer, "Data/INI/Default/AIData.ini", "Data/INI/AIData.ini");
    Init_Subsystem(g_theGameLogic, "TheGameLogic", Create_Game_Logic());
    Init_Subsystem(g_theTeamFactory, "TheTeamFactory", new TeamFactory);
    Init_Subsystem(
        g_theCrateSystem, "TheCrateSystem", new CrateSystem, &xfer, "Data/INI/Default/Crate.ini", "Data/INI/Crate.ini");
    Init_Subsystem(g_thePlayerList, "ThePlayerList", new PlayerList);
    Init_Subsystem(g_theRecorder, "TheRecorder", Create_Recorder());
    Init_Subsystem(g_theRadar, "TheRadar", Create_Radar());

    Init_Subsystem(g_theVictoryConditions, "TheVictoryConditions", Create_Victory_Conditions());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheVictoryConditions = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    Utf8String name;
    name.Format("Data\\%s\\CommandMap.ini", Get_Registry_Language().Str());
    Init_Subsystem(g_theMetaMap, "TheMetaMap", new MetaMap(), nullptr, name.Str(), "Data\\INI\\CommandMap.ini");
#ifdef GAME_DEBUG_STRUCTS
    ini.Load("Data\\INI\\CommandMapDebug.ini", INI_LOAD_UNK, nullptr);
#endif

    Init_Subsystem(g_theActionManager, "TheActionManager", new ActionManager);
    Init_Subsystem(g_theGameStateMap, "TheGameStateMap", new GameStateMap);
    Init_Subsystem(g_theGameState, "TheGameState", new GameState);

    Init_Subsystem(g_theGameResultsQueue, "TheGameResultsQueue", GameResultsInterface::Create_New_Game_Results_Interface());
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheGameResultsQueue = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    xfer.Close();
    g_theWriteableGlobalData->m_iniCRC = xfer.Get_CRC();
    captainslog_debug("INI CRC is 0x%8.8X", g_theWriteableGlobalData->m_iniCRC);
    g_theSubsystemList->Post_Process_Load_All();
    Set_FPS_Limit(g_theWriteableGlobalData->m_framesPerSecondLimit);
    g_theAudio->Set_On(g_theWriteableGlobalData->m_audioOn && g_theWriteableGlobalData->m_musicOn, AUDIOAFFECT_MUSIC);
    g_theAudio->Set_On(g_theWriteableGlobalData->m_audioOn && g_theWriteableGlobalData->m_soundsOn, AUDIOAFFECT_SOUND);
    g_theAudio->Set_On(g_theWriteableGlobalData->m_audioOn && g_theWriteableGlobalData->m_sounds3DOn, AUDIOAFFECT_3DSOUND);
    g_theAudio->Set_On(g_theWriteableGlobalData->m_audioOn && g_theWriteableGlobalData->m_speechOn, AUDIOAFFECT_SPEECH);
    g_theNetwork = nullptr;

#ifndef GAME_DEBUG_STRUCTS
    if (strcasecmp(g_theArchiveFileSystem->Get_Archive_Filename_For_File("generalsbzh.sec").Str(), "genseczh.big") != 0) {
        m_isQuitting = true;
    }

    if (strcasecmp(g_theArchiveFileSystem->Get_Archive_Filename_For_File("generalsazh.sec").Str(), "musiczh.big") != 0) {
        m_isQuitting = true;
    }
#endif

    g_theMapCache = new MapCache;
    g_theMapCache->Update_Cache();
#ifdef GAME_DEBUG_STRUCTS
    captainslog_debug("----------------------------------------------------------------------------After "
                      "TheMapCache->Update_Cache = %f seconds ",
        0.0f); // TODO processor frequency stuff
#endif

    if (g_theWriteableGlobalData->m_buildMapCache) {
        m_isQuitting = true;
    }

    if (!g_theWriteableGlobalData->m_initialFile.Is_Empty()) {
        Utf8String initial_file(g_theWriteableGlobalData->m_initialFile);

        if (initial_file.Ends_With_No_Case(".map")) {
            g_theWriteableGlobalData->m_shellMapOn = false;
            g_theWriteableGlobalData->m_playIntro = false;
            g_theWriteableGlobalData->m_pendingFile = g_theWriteableGlobalData->m_initialFile;
            GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_NEW_GAME);
            message->Append_Int_Arg(0);
            message->Append_Int_Arg(1);
            message->Append_Int_Arg(0);
            Init_Random();
        } else {
            if (initial_file.Ends_With_No_Case(".rep")) {
                g_theRecorder->Playback_File(initial_file);
            }
        }
    }

    if (g_theMapCache != nullptr && g_theWriteableGlobalData->m_shellMapOn) {
        Utf8String shell_map(g_theWriteableGlobalData->m_shellMapName);
        shell_map.To_Lower();
        auto it = g_theMapCache->find(shell_map);

        if (it == g_theMapCache->end()) {
            g_theWriteableGlobalData->m_shellMapOn = false;
        }
    }

    if (!g_theWriteableGlobalData->m_playIntro) {
        g_theWriteableGlobalData->m_afterIntro = true;
    }

    Init_KindOf_Masks();
    Init_Disabled_Masks();
    Init_Damage_Type_Masks();
    g_theSubsystemList->Reset_All();
    Hide_Control_Bar(true);
}

void GameEngine::Execute()
{
    unsigned int time = 0;
    unsigned int time2 = 0;
#ifdef PLATFORM_WINDOWS
    time = timeGetTime();
    time2 = timeGetTime() / 1000;
#endif

    while (!m_isQuitting) {
#ifdef GAME_DEBUG_STRUCTS
        // TODO benchmark stuff
#endif
        Update();
        if (g_theTacticalView->Get_Time_Multiplier() <= 1 || !g_theScriptEngine->Is_Time_Fast()) {
#ifdef GAME_DEBUG_STRUCTS
            rts::Sleep_Ms(1);

            if (!g_theWriteableGlobalData->m_TiVOFastMode || !g_theGameLogic->Is_In_Replay_Game()) {
#else
            if (!g_theWriteableGlobalData->m_demoToggleSpecialPowerDelays || !g_theGameLogic->Is_In_Replay_Game()) {
#endif
                unsigned int time3 = 0;
#ifdef PLATFORM_WINDOWS
                time3 = timeGetTime();
#endif

                while (g_theWriteableGlobalData->m_useFPSLimit && time3 - time < 1000.0f / m_maxFPS - 1.0f) {
                    rts::Sleep_Ms(0);
#ifdef PLATFORM_WINDOWS
                    time3 = timeGetTime();
#endif
                }

                time = time3;
            }
        }
    }
}

bool GameEngine::Is_Multiplayer_Session()
{
    return g_theRecorder->Is_Multiplayer();
}

FileSystem *GameEngine::Create_File_System()
{
    return new FileSystem;
}

MessageStream *GameEngine::Create_Message_Stream()
{
    return new MessageStream;
}
