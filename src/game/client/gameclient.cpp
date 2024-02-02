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
#include "displaystringmanager.h"
#include "drawable.h"
#include "drawgroupinfo.h"
#include "eva.h"
#include "gamefont.h"
#include "gamemessage.h"
#include "gamewindowmanager.h"
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
#include "placeeventtranslator.h"
#include "rayeffect.h"
#include "selectionxlat.h"
#include "shell.h"
#include "snow.h"
#include "terrainvisual.h"
#include "videoplayer.h"
#include "w3ddisplay.h"
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
#ifdef GAME_DLL
    Call_Method<void, GameClient>(PICK_ADDRESS(0x004ACDF0, 0x0082B142), this);
#endif
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
#ifdef GAME_DLL
    Call_Method<void, GameClient, TimeOfDayType>(PICK_ADDRESS(0x004AD8B0, 0x0082BD61), this, tod);
#endif
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
