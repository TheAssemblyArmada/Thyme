/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Base client object providing overall control of client IO and rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameclient.h"
#include "anim2d.h"
#include "campaignmanager.h"
#include "challengegenerals.h"
#include "commandxlat.h"
#include "controlbar.h"
#include "displaystringmanager.h"
#include "drawable.h"
#include "drawgroupinfo.h"
#include "eva.h"
#include "fpusetting.h"
#include "gamefont.h"
#include "gamemessage.h"
#include "gamewindowmanager.h"
#include "ghostobject.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "guicommandtranslator.h"
#include "headertemplate.h"
#include "hintspy.h"
#include "hotkey.h"
#include "image.h"
#include "imemanager.h"
#include "ingameui.h"
#include "keyboard.h"
#include "languagefilter.h"
#include "lookatxlat.h"
#include "metaevent.h"
#include "mouse.h"
#include "objectcreationlist.h"
#include "particlesysmanager.h"
#include "placeeventtranslator.h"
#include "playerlist.h"
#include "rayeffect.h"
#include "scriptengine.h"
#include "selectionxlat.h"
#include "shell.h"
#include "snow.h"
#include "terrainvisual.h"
#include "thingfactory.h"
#include "videoplayer.h"
#include "w3ddisplay.h"
#include "windowlayout.h"
#include "windowxlat.h"

#ifdef GAME_DLL
#include "hooker.h"
#else
GameClient *g_theGameClient;
#endif
GameClient::GameClient() :
    m_frame(0),
    m_drawableList(nullptr),
    m_nextDrawableID(static_cast<DrawableID>(1)),
    m_translatorCount(0),
    m_commandTranslator(nullptr),
    m_onScreenObjectCount(0)
{
    for (int i = 0; i < MAX_CLIENT_TRANSLATORS; i++) {
        m_translators[i] = 0;
    }

    g_theDrawGroupInfo = new DrawGroupInfo();
}

GameClient::~GameClient()
{
    if (g_theDrawGroupInfo != nullptr) {
        delete g_theDrawGroupInfo;
        g_theDrawGroupInfo = nullptr;
    }

    m_drawableTOC.clear();

    if (g_theCampaignManager != nullptr) {
        delete g_theCampaignManager;
        g_theCampaignManager = nullptr;
    }

    Drawable *next_draw;

    for (Drawable *draw = m_drawableList; draw != nullptr; draw = next_draw) {
        next_draw = draw->Get_Next();
        Destroy_Drawable(draw);
    }

    m_drawableList = nullptr;

    if (g_theRayEffects != nullptr) {
        delete g_theRayEffects;
        g_theRayEffects = nullptr;
    }

    if (g_theHotKeyManager != nullptr) {
        delete g_theHotKeyManager;
        g_theHotKeyManager = nullptr;
    }

    if (g_theInGameUI != nullptr) {
        delete g_theInGameUI;
        g_theInGameUI = nullptr;
    }

    if (g_theChallengeGenerals != nullptr) {
        delete g_theChallengeGenerals;
        g_theChallengeGenerals = nullptr;
    }

    if (g_theShell != nullptr) {
        delete g_theShell;
        g_theShell = nullptr;
    }

    if (g_theIMEManager != nullptr) {
        delete g_theIMEManager;
        g_theIMEManager = nullptr;
    }

    if (g_theWindowManager != nullptr) {
        delete g_theWindowManager;
        g_theWindowManager = nullptr;
    }

    if (g_theFontLibrary != nullptr) {
        g_theFontLibrary->Reset();
        delete g_theFontLibrary;
        g_theFontLibrary = nullptr;
    }

    if (g_theMouse != nullptr) {
        delete g_theMouse;
        g_theMouse = nullptr;
    }

    if (g_theTerrainVisual != nullptr) {
        delete g_theTerrainVisual;
        g_theTerrainVisual = nullptr;
    }

    if (g_theDisplay != nullptr) {
        delete g_theDisplay;
        g_theDisplay = nullptr;
    }

    if (g_theHeaderTemplateManager != nullptr) {
        delete g_theHeaderTemplateManager;
        g_theHeaderTemplateManager = nullptr;
    }

    if (g_theLanguageFilter != nullptr) {
        delete g_theLanguageFilter;
        g_theLanguageFilter = nullptr;
    }

    if (g_theVideoPlayer != nullptr) {
        delete g_theVideoPlayer;
        g_theVideoPlayer = nullptr;
    }

    for (unsigned int i = 0; i < m_translatorCount; i++) {
        g_theMessageStream->Remove_Translator(m_translators[i]);
    }

    m_translatorCount = 0;
    m_commandTranslator = nullptr;

    if (g_theAnim2DCollection != nullptr) {
        delete g_theAnim2DCollection;
        g_theAnim2DCollection = nullptr;
    }

    if (g_theMappedImageCollection != nullptr) {
        delete g_theMappedImageCollection;
        g_theMappedImageCollection = nullptr;
    }

    if (g_theKeyboard != nullptr) {
        delete g_theKeyboard;
        g_theKeyboard = nullptr;
    }

    if (g_theDisplayStringManager != nullptr) {
        delete g_theDisplayStringManager;
        g_theDisplayStringManager = nullptr;
    }

    if (g_theEva != nullptr) {
        delete g_theEva;
        g_theEva = nullptr;
    }

    if (g_theSnowManager != nullptr) {
        delete g_theSnowManager;
        g_theSnowManager = nullptr;
    }
}

void GameClient::Init()
{
    Set_Frame_Rate(33.333332f);
    INI ini;
    ini.Load("Data\\INI\\DrawGroupInfo.ini", INI_LOAD_OVERWRITE, nullptr);

    if (g_theGlobalLanguage != nullptr && g_theGlobalLanguage->Draw_Group_Info_Font().Name().Is_Not_Empty()) {
        g_theDrawGroupInfo->m_fontName = g_theGlobalLanguage->Draw_Group_Info_Font().Name();
        g_theDrawGroupInfo->m_fontSize = g_theGlobalLanguage->Draw_Group_Info_Font().Point_Size();
        g_theDrawGroupInfo->m_fontIsBold = g_theGlobalLanguage->Draw_Group_Info_Font().Bold();
    }

    g_theDisplayStringManager = Create_DisplayStringManager();

    if (g_theDisplayStringManager != nullptr) {
        g_theDisplayStringManager->Init();
        g_theDisplayStringManager->Set_Name("TheDisplayStringManager");
    }

    g_theKeyboard = Create_Keyboard();
    g_theKeyboard->Init();
    g_theKeyboard->Set_Name("TheKeyboard");

    g_theMappedImageCollection = new ImageCollection();
    g_theMappedImageCollection->Load(512);

    g_theAnim2DCollection = new Anim2DCollection();
    g_theAnim2DCollection->Init();
    g_theAnim2DCollection->Set_Name("TheAnim2DCollection");

    if (g_theMessageStream != nullptr) {
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new WindowTranslator(), 10);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new MetaEventTranslator(), 20);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new HotKeyTranslator(), 25);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new PlaceEventTranslator(), 30);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new GUICommandTranslator(), 40);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new SelectionTranslator(), 50);
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new LookAtTranslator(), 60);
        m_translators[m_translatorCount] = g_theMessageStream->Attach_Translator(new CommandTranslator(), 70);
        m_commandTranslator =
            static_cast<CommandTranslator *>(g_theMessageStream->Find_Translator(m_translators[m_translatorCount]));
        m_translatorCount++;
        m_translators[m_translatorCount++] = g_theMessageStream->Attach_Translator(new HintSpyTranslator(), 100);
        m_translators[m_translatorCount++] =
            g_theMessageStream->Attach_Translator(new GameClientMessageDispatcher(), 999999999);
    }

    g_theFontLibrary = Create_FontLibrary();

    if (g_theFontLibrary != nullptr) {
        g_theFontLibrary->Init();
    }

    g_theMouse = Create_Mouse();
    g_theMouse->Parse_INI();
    g_theMouse->Init_Cursor_Resources();
    g_theMouse->Set_Name("TheMouse");

    g_theDisplay = Create_GameDisplay();

    if (g_theDisplay != nullptr) {
        g_theDisplay->Init();
        g_theDisplay->Set_Name("TheDisplay");
    }

    g_theHeaderTemplateManager = new HeaderTemplateManager();

    if (g_theHeaderTemplateManager != nullptr) {
        g_theHeaderTemplateManager->Init();
    }

    g_theWindowManager = Create_WindowManager();

    if (g_theWindowManager != nullptr) {
        g_theWindowManager->Init();
        g_theWindowManager->Set_Name("TheWindowManager");
    }

    g_theIMEManager = Create_IME_Manager_Interface();

    if (g_theIMEManager != nullptr) {
        g_theIMEManager->Init();
        g_theIMEManager->Set_Name("TheIMEManager");
    }

    g_theShell = new Shell();

    if (g_theShell != nullptr) {
        g_theShell->Init();
        g_theShell->Set_Name("TheShell");
    }

    g_theInGameUI = Create_InGameUI();

    if (g_theInGameUI != nullptr) {
        g_theInGameUI->Init();
        g_theInGameUI->Set_Name("TheInGameUI");
    }

    g_theChallengeGenerals = Create_Challenge_Generals();

    if (g_theChallengeGenerals != nullptr) {
        g_theChallengeGenerals->Init();
    }

    g_theHotKeyManager = new HotKeyManager();

    if (g_theHotKeyManager != nullptr) {
        g_theHotKeyManager->Init();
        g_theHotKeyManager->Set_Name("TheHotKeyManager");
    }

    g_theTerrainVisual = Create_TerrainVisual();

    if (g_theTerrainVisual != nullptr) {
        g_theTerrainVisual->Init();
        g_theTerrainVisual->Set_Name("TheTerrainVisual");
    }

    g_theRayEffects = new RayEffectSystem();

    if (g_theRayEffects != nullptr) {
        g_theRayEffects->Init();
        g_theRayEffects->Set_Name("TheRayEffects");
    }

    if (g_theMouse != nullptr) {
        g_theMouse->Init();
        g_theMouse->Set_Position(0, 0);
        g_theMouse->Set_Mouse_Limits();
        g_theMouse->Set_Name("TheMouse");
    }

    g_theVideoPlayer = Create_VideoPlayer();

    if (g_theVideoPlayer != nullptr) {
        g_theVideoPlayer->Init();
        g_theVideoPlayer->Set_Name("TheVideoPlayer");
    }

    g_theLanguageFilter = Create_Language_Filter();

    if (g_theLanguageFilter != nullptr) {
        g_theLanguageFilter->Init();
        g_theLanguageFilter->Set_Name("TheLanguageFilter");
    }

    g_theCampaignManager = new CampaignManager();
    g_theCampaignManager->Init();

    g_theEva = new Eva();
    g_theEva->Init();
    g_theEva->Set_Name("TheEva");

    g_theDisplayStringManager->PostProcessLoad();

    g_theSnowManager = Create_SnowManager();

    if (g_theSnowManager != nullptr) {
        g_theSnowManager->Init();
        g_theSnowManager->Set_Name("TheSnowManager");
    }
}

void GameClient::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x004AE080, 0x0082C7BA), this, xfer);
#endif
}

void GameClient::Load_Post_Process()
{
    for (Drawable *draw = Get_Drawable_List(); draw != nullptr; draw = draw->Get_Next()) {
        if (draw->Get_ID() >= m_nextDrawableID) {
            m_nextDrawableID = static_cast<DrawableID>(draw->Get_ID() + 1);
        }
    }
}

void GameClient::Reset()
{
    m_drawableLUT.clear();
    m_drawableLUT.resize(0x2000);
    g_theInGameUI->Reset();

    Drawable *next_draw;

    for (Drawable *draw = m_drawableList; draw != nullptr; draw = next_draw) {
        next_draw = draw->Get_Next();
        Destroy_Drawable(draw);
    }

    m_drawableList = nullptr;
    g_theDisplay->Reset();
    g_theTerrainVisual->Reset();
    g_theRayEffects->Reset();
    g_theVideoPlayer->Reset();
    g_theEva->Reset();

    if (g_theSnowManager != nullptr) {
        g_theSnowManager->Reset();
    }

    m_drawableTOC.clear();
}

void GameClient::Update()
{
    GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_FRAME_TICK);
    unsigned int frame = Get_Frame();
    msg->Append_Time_Stamp_Arg(frame);
    static bool playSizzle = false;
    if (g_theWriteableGlobalData->m_playIntro && !g_theDisplay->Is_Movie_Playing()) {
        if (g_theGameLODManager != nullptr && g_theGameLODManager->Did_Mem_Pass()) {
            g_theDisplay->Play_Logo_Movie("EALogoMovie", 5000, 3000);
        } else {
            g_theDisplay->Play_Logo_Movie("EALogoMovie640", 5000, 3000);
        }

        g_theWriteableGlobalData->m_playIntro = false;
        g_theWriteableGlobalData->m_afterIntro = true;
        playSizzle = true;
    }

    if (g_theWriteableGlobalData->m_afterIntro && !g_theDisplay->Is_Movie_Playing()) {
        if (playSizzle && g_theWriteableGlobalData->m_playSizzle) {
            g_theWriteableGlobalData->m_allowSkipMovie = true;

            if (g_theGameLODManager != nullptr && g_theGameLODManager->Did_Mem_Pass()) {
                g_theDisplay->Play_Movie("Sizzle");
            } else {
                g_theDisplay->Play_Movie("Sizzle640");
            }

            playSizzle = false;
        } else {
            g_theWriteableGlobalData->m_unkBool26 = true;
            g_theWriteableGlobalData->m_allowSkipMovie = true;

            if (g_theGameLODManager != nullptr && !g_theGameLODManager->Did_Mem_Pass()) {
                g_theWriteableGlobalData->m_unkBool26 = false;
                WindowLayout *layout = g_theWindowManager->Win_Create_Layout("Menus/LegalPage.wnd");

                if (layout != nullptr) {
                    layout->Hide(false);
                    layout->Bring_Forward();
                    unsigned int time = rts::Get_Time();

                    for (;;) {
                        if (time + 4000 <= rts::Get_Time()) {
                            break;
                        }

                        g_theWindowManager->Update();
                        g_theDisplay->Draw();
                        rts::Sleep_Ms(100);
                    }

                    Set_FP_Mode();
                    layout->Destroy_Windows();
                    layout->Delete_Instance();
                }

                g_theWriteableGlobalData->m_unkBool26 = true;
            }

            g_theShell->Show_Shell_Map(true);
            g_theShell->Show_Shell(1);
            g_theWriteableGlobalData->m_afterIntro = false;
        }
    }

    if (g_theSnowManager != nullptr) {
        g_theSnowManager->Update();
    }

    g_theAnim2DCollection->Update();

    if (g_theKeyboard != nullptr) {
        g_theKeyboard->Update();
        g_theKeyboard->Create_Stream_Messages();
    }

    g_theEva->Update();

    if (g_theMouse != nullptr) {
        g_theMouse->Update();
        g_theMouse->Create_Stream_Messages();
    }

    if (g_theInGameUI->Is_Camera_Tracking_Drawable()) {
        Drawable *drawable = g_theInGameUI->Get_First_Selected_Drawable();

        if (drawable != nullptr) {
            g_theTacticalView->Look_At(drawable->Get_Position());
        } else {
            g_theInGameUI->Set_Camera_Drawing_Trackable(false);
        }
    }

    if (g_theWriteableGlobalData->m_playIntro || g_theWriteableGlobalData->m_afterIntro) {
        g_theDisplay->Draw();
        g_theDisplay->Update();
    } else {
        g_theWindowManager->Update();
        g_theVideoPlayer->Update();

        bool is_paused = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();
        is_paused = is_paused || g_theScriptEngine->Is_Time_Frozen_Debug();
        is_paused = is_paused || g_theScriptEngine->Is_Time_Frozen_Script();
        is_paused = is_paused || g_theGameLogic->Is_Game_Paused();
        unsigned int player_index;

        if (g_thePlayerList != nullptr) {
            player_index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            player_index = 0;
        }

        static unsigned int lastFrame = -1;

        is_paused = is_paused || (lastFrame == m_frame);
        lastFrame = m_frame;

        if (!is_paused) {
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_shroudOn)
#endif
            {
                g_theGhostObjectManager->Update_Orphaned_Objects(nullptr, 0);
            }

            Drawable *next_drawable;

            for (Drawable *draw = First_Drawable(); draw != nullptr; draw = next_drawable) {
                next_drawable = draw->Get_Next();
#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_shroudOn)
#endif
                {
                    Object *obj = draw->Get_Object();

                    if (obj != nullptr) {
                        ObjectShroudStatus status = obj->Get_Shrouded_Status(player_index);

                        if (status >= SHROUDED_SEEN && draw->Get_Remain_Visible_Frames() != 0) {
                            int frames = 60;

                            if (obj->Is_Effectively_Dead()) {
                                frames += 90;
                            }

                            if (g_theGameLogic->Get_Frame() < draw->Get_Remain_Visible_Frames() + frames) {
                                status = SHROUDED_NONE;
                            }
                        }

                        draw->Set_Fully_Obscured_By_Shroud(status >= SHROUDED_SEEN);
                    }
                }

                draw->Update_Drawable();
            }
        }

        if (static_cast<unsigned int>(g_theWriteableGlobalData->m_frameToJumpTo) <= g_theGameLogic->Get_Frame()
            || g_theGameLogic->Get_Frame() == 0) {
            if (!is_paused) {
                g_theParticleSystemManager->Set_Player_Index(player_index);
            }

            g_theTerrainVisual->Update();
            g_theDisplay->Update();
            g_theDisplay->Draw();
            g_theDisplayStringManager->Update();
            g_theShell->Update();
            g_theInGameUI->Update();
        }
    }
}

void GameClient::Register_Drawable(Drawable *drawable)
{
    drawable->Set_ID(Alloc_Drawable_ID());
    drawable->Prepend_To_List(&m_drawableList);
}

Drawable *GameClient::Find_Drawable_By_ID(DrawableID id)
{
    if (id == INVALID_DRAWABLE_ID) {
        return nullptr;
    }

    if (id > m_drawableLUT.size()) {
        return nullptr;
    }

    return m_drawableLUT[id];
}

Drawable *GameClient::First_Drawable()
{
    return m_drawableList;
}

GameMessage::MessageType GameClient::Evaluate_Context_Command(
    Drawable *drawable, const Coord3D *pos, CommandTranslator::CommandEvaluateType type)
{
#ifdef GAME_DLL
    return Call_Method<GameMessage::MessageType,
        GameClient,
        Drawable *,
        const Coord3D *,
        CommandTranslator::CommandEvaluateType>(PICK_ADDRESS(0x004AD800, 0x0082BC7E), this, drawable, pos, type);
#else
    return GameMessage::MSG_INVALID;
#endif
}

void GameClient::Iterate_Drawables_In_Region(Region3D *region, void (*func)(Drawable *, void *), void *data)
{
    Drawable *next_draw;

    for (Drawable *draw = m_drawableList; draw != nullptr; draw = next_draw) {
        next_draw = draw->Get_Next();
        Coord3D pos = *draw->Get_Position();

        if (region == nullptr
            || (pos.x >= region->lo.x && pos.x <= region->hi.x && pos.y >= region->lo.y && pos.y <= region->hi.y
                && pos.z >= region->lo.z && pos.z <= region->hi.z)) {
            func(draw, data);
        }
    }
}

void GameClient::Destroy_Drawable(Drawable *drawable)
{
    g_theInGameUI->Disregard_Drawable(drawable);
    drawable->Remove_From_List(&m_drawableList);

    Object *obj = drawable->Get_Object();

    if (obj != nullptr) {
        captainslog_dbgassert(obj->Get_Drawable() == drawable, "Object/Drawable pointer mismatch!");
        obj->Friend_Bind_To_Drawable(nullptr);
    }

    Remove_Drawable_From_Lookup_Table(drawable);
    drawable->Delete_Instance();
}

void GameClient::Set_Time_Of_Day(TimeOfDayType time)
{
    for (Drawable *draw = First_Drawable(); draw != nullptr; draw = draw->Get_Next()) {
        draw->Set_Time_Of_Day(time);
    }
}

void GameClient::Release_Shadows()
{
    for (Drawable *draw = First_Drawable(); draw != nullptr; draw = draw->Get_Next()) {
        draw->Release_Shadows();
    }
}

void GameClient::Allocate_Shadows()
{
    for (Drawable *draw = First_Drawable(); draw != nullptr; draw = draw->Get_Next()) {
        draw->Allocate_Shadows();
    }
}

void GameClient::Preload_Assets(TimeOfDayType tod)
{
    // TODO memory debug logging
    for (Drawable *draw = Get_Drawable_List(); draw != nullptr; draw = draw->Get_Next()) {
        draw->Preload_Assets(tod);
    }

    for (ThingTemplate *thing = g_theThingFactory->First_Template(); thing != nullptr;
         thing = thing->Friend_Get_Next_Template()) {
        if (thing->Is_KindOf(KINDOF_PRELOAD) || g_theWriteableGlobalData->m_preloadEverything) {
            Drawable *draw = g_theThingFactory->New_Drawable(thing, DRAWABLE_STATUS_UNK);

            if (draw != nullptr) {
                draw->Preload_Assets(tod);
                g_theGameClient->Destroy_Drawable(draw);
            }
        }
    }

    for (unsigned int i = 0; i < g_debrisModelNamesGlobalHack.size(); i++) {
        g_theDisplay->Preload_Model_Assets(g_debrisModelNamesGlobalHack[i]);
    }

    g_debrisModelNamesGlobalHack.clear();
    g_theControlBar->Preload_Assets(tod);
    g_theParticleSystemManager->Preload_Assets(tod);

    char *preload_textures[39];
    preload_textures[0] = "ptspruce01.tga";
    preload_textures[1] = "exrktflame.tga";
    preload_textures[2] = "cvlimo3_d2.tga";
    preload_textures[3] = "exfthrowerstream.tga";
    preload_textures[4] = "uvrockbug_d1.tga";
    preload_textures[5] = "arcbackgroundc.tga";
    preload_textures[6] = "grade3.tga";
    preload_textures[7] = "framebasec.tga";
    preload_textures[8] = "gradec.tga";
    preload_textures[9] = "frametopc.tga";
    preload_textures[10] = "arcbackgrounda.tga";
    preload_textures[11] = "arcglow2.tga";
    preload_textures[12] = "framebasea.tga";
    preload_textures[13] = "gradea.tga";
    preload_textures[14] = "frametopa.tga";
    preload_textures[15] = "sauserinterface256_002.tga";
    preload_textures[16] = "sauserinterface256_001.tga";
    preload_textures[17] = "unitbackgrounda.tga";
    preload_textures[18] = "sauserinterface256_004.tga";
    preload_textures[19] = "sagentank.tga";
    preload_textures[20] = "sauserinterface256_005.tga";
    preload_textures[21] = "sagenair.tga";
    preload_textures[22] = "sauserinterface256_003.tga";
    preload_textures[23] = "sagenspec.tga";
    preload_textures[24] = "snuserinterface256_003.tga";
    preload_textures[25] = "snuserinterface256_002.tga";
    preload_textures[26] = "unitbackgroundc.tga";
    preload_textures[27] = "snuserinterface256_004.tga";
    preload_textures[28] = "sngenredarm.tga";
    preload_textures[29] = "snuserinterface256_001.tga";
    preload_textures[30] = "sngenspewea.tga";
    preload_textures[31] = "sngensecpol.tga";
    preload_textures[32] = "ciburn.tga";
    preload_textures[33] = "ptmaple02.tga";
    preload_textures[34] = "scuserinterface256_005.tga";
    preload_textures[35] = "scuserinterface256_002.tga";
    preload_textures[36] = "sauserinterface256_006.tga";
    preload_textures[37] = "pmcrates.tga";
    preload_textures[38] = "";

    for (int i = 0; *preload_textures[i] != '\0'; i++) {
        g_theDisplay->Preload_Texture_Assets(preload_textures[i]);
    }
}

void GameClient::Remove_From_Ray_Effects(Drawable *drawable)
{
    g_theRayEffects->Delete_Ray_Effect(drawable);
}

void GameClient::Get_Ray_Effect_Data(Drawable *drawable, RayEffectData *data)
{
    g_theRayEffects->Get_Ray_Effect_Data(drawable, data);
}

GameMessageDisposition GameClientMessageDispatcher::Translate_Game_Message(const GameMessage *msg)
{
    GameMessage::MessageType type = msg->Get_Type();
    if (type >= GameMessage::MSG_BEGIN_NETWORK_MESSAGES && type <= GameMessage::MSG_END_NETWORK_MESSAGES) {
        return KEEP_MESSAGE;
    }

    switch (type) {
        case GameMessage::MSG_NEW_GAME: // Fallthrough
        case GameMessage::MSG_CLEAR_GAME_DATA: // Fallthrough
        case GameMessage::MSG_FRAME_TICK: // Fallthrough
            return KEEP_MESSAGE;
        default:
            break;
    }

    return DESTROY_MESSAGE;
}

GameMessageDisposition CommandTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, CommandTranslator, const GameMessage *>(
        PICK_ADDRESS(0x005EC8D0, 0x007ECAD8), this, msg);
#else
    return GameMessageDisposition();
#endif
}

void GameClient::Add_Text_Bearing_Drawable(Drawable *drawable)
{
    if (drawable) {
        m_drawableTB.push_back(drawable);
    }
}

void GameClient::Remove_Drawable_From_Lookup_Table(Drawable *drawable)
{
    if (drawable != nullptr) {
        m_drawableLUT[drawable->Get_ID()] = nullptr;
    }
}

void GameClient::Add_Drawable_To_Lookup_Table(Drawable *drawable)
{
    if (drawable != nullptr) {
        DrawableID id = drawable->Get_ID();

        while (id >= m_drawableLUT.size()) {
            m_drawableLUT.resize(2 * m_drawableLUT.size(), nullptr);
        }

        m_drawableLUT[id] = drawable;
    }
}

void GameClient::Flush_Text_Bearing_Drawables()
{
    for (auto it = m_drawableTB.begin(); it != m_drawableTB.end(); it++) {
        (*it)->Draw_UI_Text();
    }

    m_drawableTB.clear();
}

void GameClient::Update_Fake_Drawables()
{
    for (Drawable *draw = Get_Drawable_List(); draw != nullptr; draw = draw->Get_Next()) {
        Object *obj = draw->Get_Object();

        if (obj != nullptr && obj->Is_KindOf(KINDOF_FS_FAKE)) {
            Relationship relationship = g_thePlayerList->Get_Local_Player()->Get_Relationship(obj->Get_Team());

            if (relationship == ALLIES || relationship == NEUTRAL) {
                draw->Set_Terrain_Decal(TERRAIN_DECAL_9);
            } else {
                draw->Set_Terrain_Decal(TERRAIN_DECAL_8);
            }
        }
    }
}

GameClient::DrawableTOCEntry *GameClient::Find_TOC_Entry_By_Name(Utf8String name)
{
    for (auto it = m_drawableTOC.begin(); it != m_drawableTOC.end(); it++) {
        if ((*it).name == name) {
            return &*it;
        }
    }

    return nullptr;
}

GameClient::DrawableTOCEntry *GameClient::Find_TOC_Entry_By_ID(unsigned short id)
{
    for (auto it = m_drawableTOC.begin(); it != m_drawableTOC.end(); it++) {
        if ((*it).id == id) {
            return &*it;
        }
    }

    return nullptr;
}

void GameClient::Add_TOC_Entry(Utf8String name, unsigned short id)
{
    DrawableTOCEntry entry;
    entry.name = name;
    entry.id = id;
    m_drawableTOC.push_back(entry);
}
