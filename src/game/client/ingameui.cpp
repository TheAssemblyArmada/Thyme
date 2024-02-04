/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief In Game UI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ingameui.h"
#include "actionmanager.h"
#include "ai.h"
#include "aiguard.h"
#include "aipathfind.h"
#include "behaviormodule.h"
#include "colorspace.h"
#include "controlbar.h"
#include "diplomacy.h"
#include "displaystringmanager.h"
#include "drawable.h"
#include "eva.h"
#include "gadgetpushbutton.h"
#include "gadgetstatictext.h"
#include "gameclient.h"
#include "gamefont.h"
#include "gamelogic.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "globallanguage.h"
#include "ingamechat.h"
#include "lookatxlat.h"
#include "mobmemberslavedupdate.h"
#include "mouse.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"
#include "radar.h"
#include "recorder.h"
#include "scriptengine.h"
#include "selectionxlat.h"
#include "specialpower.h"
#include "stealthupdate.h"
#include "supplywarehousedockupdate.h"
#include "team.h"
#include "terrainvisual.h"
#include "thingfactory.h"
#include "videobuffer.h"
#include "videoplayer.h"
#include "videostream.h"
#include "w3ddisplay.h"
#include "w3dview.h"
#include "windowlayout.h"
#include <set>

static float LOGICFRAMES_PER_SECONDS_REAL = 30.0f;
static float SECONDS_PER_LOGICFRAME_REAL = 1.0f / LOGICFRAMES_PER_SECONDS_REAL;

#ifndef GAME_DLL
InGameUI *g_theInGameUI;
GameWindow *g_replayWindow;
#endif

const FieldParse InGameUI::s_fieldParseTable[] = {
    { "MaxSelectionSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_maxSelectCount) },
    { "MessageColor1", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_messageColor1) },
    { "MessageColor2", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_messageColor2) },
    { "MessagePosition", INI::Parse_ICoord2D, nullptr, offsetof(InGameUI, m_messagePosition) },
    { "MessageFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_messageFont) },
    { "MessagePointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_messagePointSize) },
    { "MessageBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_messageBold) },
    { "MessageDelayMS", INI::Parse_Int, nullptr, offsetof(InGameUI, m_messageDelayMS) },
    { "MilitaryCaptionColor", INI::Parse_RGBA_Color_Int, nullptr, offsetof(InGameUI, m_militaryCaptionColor) },
    { "MilitaryCaptionPosition", INI::Parse_ICoord2D, nullptr, offsetof(InGameUI, m_militaryCaptionPosition) },
    { "MilitaryCaptionTitleFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_militaryCaptionTitleFont) },
    { "MilitaryCaptionTitlePointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_militaryCaptionTitlePointSize) },
    { "MilitaryCaptionTitleBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_militaryCaptionTitleBold) },
    { "MilitaryCaptionFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_militaryCaptionFont) },
    { "MilitaryCaptionPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_militaryCaptionPointSize) },
    { "MilitaryCaptionBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_militaryCaptionBold) },
    { "MilitaryCaptionRandomizeTyping", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_militaryCaptionRandomizeTyping) },
    { "MilitaryCaptionSpeed", INI::Parse_Int, nullptr, offsetof(InGameUI, m_militaryCaptionSpeed) },
    { "MilitaryCaptionPosition", INI::Parse_ICoord2D, nullptr, offsetof(InGameUI, m_militaryCaptionPosition) },
    { "SuperweaponCountdownPosition", INI::Parse_Coord2D, nullptr, offsetof(InGameUI, m_superweaponPosition) },
    { "SuperweaponCountdownFlashDuration",
        INI::Parse_Duration_Real,
        nullptr,
        offsetof(InGameUI, m_superweaponFlashDuration) },
    { "SuperweaponCountdownFlashColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_superweaponFlashColor) },
    { "SuperweaponCountdownNormalFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_superweaponNormalFont) },
    { "SuperweaponCountdownNormalPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_superweaponNormalPointSize) },
    { "SuperweaponCountdownNormalBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_superweaponNormalBold) },
    { "SuperweaponCountdownReadyFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_superweaponReadyFont) },
    { "SuperweaponCountdownReadyPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_superweaponReadyPointSize) },
    { "SuperweaponCountdownReadyBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_superweaponReadyBold) },
    { "NamedTimerCountdownPosition", INI::Parse_Coord2D, nullptr, offsetof(InGameUI, m_namedTimerPosition) },
    { "NamedTimerCountdownFlashDuration", INI::Parse_Duration_Real, nullptr, offsetof(InGameUI, m_namedTimerFlashDuration) },
    { "NamedTimerCountdownFlashColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_namedTimerFlashColor) },
    { "NamedTimerCountdownNormalFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_namedTimerNormalFont) },
    { "NamedTimerCountdownNormalPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_namedTimerNormalPointSize) },
    { "NamedTimerCountdownNormalBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_namedTimerNormalBold) },
    { "NamedTimerCountdownNormalColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_namedTimerNormalColor) },
    { "NamedTimerCountdownReadyFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_namedTimerReadyFont) },
    { "NamedTimerCountdownReadyPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_namedTimerReadyPointSize) },
    { "NamedTimerCountdownReadyBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_namedTimerReadyBold) },
    { "NamedTimerCountdownReadyColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_namedTimerReadyColor) },
    { "FloatingTextTimeOut", INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(InGameUI, m_floatingTextTimeOut) },
    { "FloatingTextMoveUpSpeed", INI::Parse_Velocity_Real, nullptr, offsetof(InGameUI, m_floatingTextMoveUpSpeed) },
    { "FloatingTextVanishRate", INI::Parse_Velocity_Real, nullptr, offsetof(InGameUI, m_floatingTextMoveVanishRate) },
    { "PopupMessageColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_popupMessageColor) },
    { "DrawableCaptionFont", INI::Parse_AsciiString, nullptr, offsetof(InGameUI, m_drawableCaptionFont) },
    { "DrawableCaptionPointSize", INI::Parse_Int, nullptr, offsetof(InGameUI, m_drawableCaptionPointSize) },
    { "DrawableCaptionBold", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_drawableCaptionBold) },
    { "DrawableCaptionColor", INI::Parse_Color_Int, nullptr, offsetof(InGameUI, m_drawableCaptionColor) },
    { "DrawRMBScrollAnchor", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_drawRMBScrollAnchor) },
    { "MoveRMBScrollAnchor", INI::Parse_Bool, nullptr, offsetof(InGameUI, m_moveRMBScrollAnchor) },
    { "AttackDamageAreaRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_ATTACK_DAMAGE_AREA]) },
    { "AttackScatterAreaRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_ATTACK_SCATTER_AREA]) },
    { "AttackContinueAreaRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_ATTACK_CONTINUE_AREA]) },
    { "FriendlySpecialPowerRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_FRIENDLY_SPECIALPOWER]) },
    { "OffensiveSpecialPowerRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_OFFENSIVE_SPECIALPOWER]) },
    { "SuperweaponScatterAreaRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_SUPERWEAPON_SCATTER_AREA]) },
    { "GuardAreaRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_GUARD_AREA]) },
    { "EmergencyRepairRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_EMERGENCY_REPAIR]) },
    { "ParticleCannonRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_PARTICLECANNON]) },
    { "A10StrikeRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_A10STRIKE]) },
    { "CarpetBombRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_CARPETBOMB]) },
    { "DaisyCutterRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_DAISYCUTTER]) },
    { "ParadropRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_PARADROP]) },
    { "SpySatelliteRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_SPYSATELLITE]) },
    { "SpectreGunshipRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_SPECTREGUNSHIP]) },
    { "HelixNapalmBombRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_HELIX_NAPALM_BOMB]) },
    { "NuclearMissileRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_NUCLEARMISSILE]) },
    { "EMPPulseRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_EMPPULSE]) },
    { "ArtilleryRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_ARTILLERYBARRAGE]) },
    { "FrenzyRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_FRENZY]) },
    { "NapalmStrikeRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_NAPALMSTRIKE]) },
    { "ClusterMinesRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_CLUSTERMINES]) },
    { "ScudStormRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_SCUDSTORM]) },
    { "AnthraxBombRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_ANTHRAXBOMB]) },
    { "AmbushRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_AMBUSH]) },
    { "RadarRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_RADAR]) },
    { "SpyDroneRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_SPYDRONE]) },
    { "ClearMinesRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_CLEARMINES]) },
    { "AmbulanceRadiusCursor",
        RadiusDecalTemplate::Parse_Radius_Decal_Template,
        nullptr,
        offsetof(InGameUI, m_radiusDecalTemplate[RADIUS_CURSOR_AMBULANCE]) },
    { nullptr, nullptr, nullptr, 0 },
};

void Show_Replay_Controls()
{
    if (g_replayWindow != nullptr) {
        g_replayWindow->Win_Hide(!g_theGameLogic->Is_In_Replay_Game());
    }
}

void Hide_Replay_Controls()
{
    if (g_replayWindow != nullptr) {
        g_replayWindow->Win_Hide(true);
    }
}

void Toggle_Replay_Controls()
{
    if (g_replayWindow != nullptr) {
        g_replayWindow->Win_Hide(!(g_theGameLogic->Is_In_Replay_Game() && g_replayWindow->Win_Is_Hidden()));
    }
}

InGameUI::InGameUI() :
    m_superweaponDisplayEnabledByScript(false),
    m_inputEnabled(true),
    m_isDragSelecting(false),
    m_displayedMaxWarning(false),
    m_nextMoveHint(0),
    m_pendingGUICommand(nullptr),
    m_pendingPlaceType(nullptr),
    m_pendingPlaceSourceObjectID(INVALID_OBJECT_ID),
#ifndef GAME_DEBUG_STRUCTS
    m_preventLeftClickDeselectionInAlternateMouseModeForOneClick(false),
#endif
    m_placeAnchorInProgress(false),
    m_selectCount(0),
    m_maxSelectCount(-1),
    m_frameSelectionChanged(0),
    m_doubleClickCounter(0),
    m_videoBuffer(nullptr),
    m_videoStream(nullptr),
    m_cameoVideoBuffer(nullptr),
    m_cameoVideoStream(nullptr),
    m_superweaponFlashDuration(1.0f),
    m_superweaponNormalFont("Arial"),
    m_superweaponNormalPointSize(10),
    m_superweaponNormalBold(false),
    m_superweaponReadyFont("Arial"),
    m_superweaponReadyPointSize(10),
    m_superweaponReadyBold(false),
    m_superweaponLastFlashFrame(0),
    m_superweaponHidden(true),
    m_namedTimerFlashDuration(0.0f),
    m_namedTimerLastFlashFrame(0),
    m_namedTimerUsedFlashColor(true),
    m_showNamedTimers(true),
    m_namedTimerNormalFont("Arial"),
    m_namedTimerNormalPointSize(10),
    m_namedTimerNormalBold(false),
    m_namedTimerReadyFont("Arial"),
    m_namedTimerReadyPointSize(10),
    m_namedTimerReadyBold(false),
    m_drawableCaptionFont("Arial"),
    m_drawableCaptionPointSize(10),
    m_drawableCaptionBold(false),
    m_tooltipsDisabled(0),
    m_militarySubtitle(nullptr),
    m_isScrolling(false),
    m_isSelecting(false),
    m_mouseMode(MOUSEMODE_DEFAULT),
    m_mouseCursor(CURSOR_ARROW),
    m_mousedOverDrawableID(INVALID_DRAWABLE_ID),
    m_isQuitMenuVisible(false),
    m_messagesOn(true),
    m_messageFont("Arial"),
    m_messagePointSize(10),
    m_messageBold(false),
    m_messageDelayMS(5000),
    m_militaryCaptionTitleFont("Courier"),
    m_militaryCaptionTitlePointSize(12),
    m_militaryCaptionTitleBold(true),
    m_militaryCaptionFont("Courier"),
    m_militaryCaptionPointSize(12),
    m_militaryCaptionBold(false),
    m_militaryCaptionRandomizeTyping(false),
    m_militaryCaptionSpeed(1),
    m_radiusDecalType(RADIUS_CURSOR_NONE),
    m_floatingTextTimeOut(10),
    m_floatingTextMoveUpSpeed(1.0f),
    m_floatingTextMoveVanishRate(0.1f),
    m_popupMessageData(nullptr),
    m_waypointMode(false),
    m_forceToAttackMode(false),
    m_forceToMoveMode(false),
    m_attackMoveToMode(false),
    m_preferSelection(false),
    m_cameraRotateLeft(false),
    m_cameraRotateRight(false),
    m_cameraZoomIn(false),
    m_cameraDrawingTrackable(false),
    m_cameraZoomOut(false),
    m_drawRMBScrollAnchor(false),
    m_moveRMBScrollAnchor(false),
    m_noRadarEdgeSound(false),
    m_idleWorkerWin(nullptr),
    m_idleWorkerCount(-1),
    m_soloNexusSelectedDrawableID(INVALID_DRAWABLE_ID)
{
    m_radiusDecalPos.Zero();
    m_currentlyPlayingMovie.Clear();
    m_messageColor1 = Make_Color(255, 255, 255, 255);
    m_messageColor2 = Make_Color(180, 180, 180, 255);
    m_messagePosition.x = 10;
    m_messagePosition.y = 10;
    m_militaryCaptionColor.red = 200;
    m_militaryCaptionColor.green = 200;
    m_militaryCaptionColor.blue = 30;
    m_militaryCaptionColor.alpha = 255;
    m_militaryCaptionPosition.x = 10;
    m_militaryCaptionPosition.y = 380;
    m_popupMessageColor = Make_Color(255, 255, 255, 255);

    for (int i = 0; i < MAX_MOVE_HINTS; i++) {
        m_moveHint[i].pos.Zero();
        m_moveHint[i].source_id = 0;
        m_moveHint[i].frame = 0;
    }

    for (int i = 0; i < MAX_BUILD_PROGRESS; i++) {
        m_buildProgress[i].m_thingTemplate = nullptr;
        m_buildProgress[i].m_percentComplete = 0.0f;
        m_buildProgress[i].m_control = nullptr;
    }

    m_placeIcon = new Drawable *[g_theWriteableGlobalData->m_maxLineBuildObjects];

    for (int i = 0; i < g_theWriteableGlobalData->m_maxLineBuildObjects; i++) {
        m_placeIcon[i] = nullptr;
    }

    m_placeAnchorStart.x = 0;
    m_placeAnchorStart.y = 0;
    m_placeAnchorEnd.x = 0;
    m_placeAnchorEnd.y = 0;

    for (int i = 0; i < MAX_UI_MESSAGES; i++) {
        m_uiMessages[i].full_text.Clear();
        m_uiMessages[i].display_string = nullptr;
        m_uiMessages[i].timestamp = 0;
        m_uiMessages[i].color = 0;
    }

    g_replayWindow = nullptr;
    m_superweaponPosition.x = 0.7f;
    m_superweaponPosition.y = 0.7f;
    m_superweaponFlashColor = Make_Color(255, 255, 255, 255);
    m_namedTimerPosition.x = 0.05f;
    m_namedTimerPosition.y = 0.7f;
    m_namedTimerNormalColor = Make_Color(255, 255, 0, 255);
    m_namedTimerReadyColor = Make_Color(255, 0, 255, 255);
    m_namedTimerFlashColor = Make_Color(0, 255, 255, 255);
    m_drawableCaptionColor = Make_Color(255, 255, 255, 255);
    m_scrollAmt.x = 0;
    m_scrollAmt.y = 0;
    m_dragSelectRegion.hi.x = 0;
    m_dragSelectRegion.hi.y = 0;
    m_dragSelectRegion.lo.x = 0;
    m_dragSelectRegion.lo.y = 0;
}

InGameUI::~InGameUI()
{
    if (g_theControlBar != nullptr) {
        delete g_theControlBar;
        g_theControlBar = nullptr;
    }

    Remove_Military_Subtitle();
    Stop_Movie();
    Stop_Cameo_Movie();
    Place_Build_Available(nullptr, nullptr);
    Set_Radius_Cursor_None();
    Free_Message_Resources();
    delete[] m_placeIcon;
    m_placeIcon = nullptr;
    Clear_Floating_Text();
    Clear_World_Animations();
    Reset_Idle_Worker();
}

SuperweaponInfo::SuperweaponInfo(ObjectID object_id,
    unsigned int countdown_seconds,
    bool hidden,
    bool is_missing_science,
    bool is_ready,
    bool has_played_sound,
    const Utf8String &font_name,
    int font_size,
    bool font_bold,
    int color,
    const SpecialPowerTemplate *power_template) :
    m_name(0),
    m_time(0),
    m_color(color),
    m_powerTemplate(power_template),
    m_objectID(object_id),
    m_countdownSeconds(countdown_seconds),
    m_hidden(hidden),
    m_isMissingScience(is_missing_science),
    m_isReady(is_ready),
    m_hasPlayedSound(has_played_sound),
    m_refreshCountdown(false)
{
    m_name = g_theDisplayStringManager->New_Display_String();
    m_name->Reset();
    m_name->Set_Text(Utf16String::s_emptyString);

    m_time = g_theDisplayStringManager->New_Display_String();
    m_time->Reset();
    m_time->Set_Text(Utf16String::s_emptyString);

    Set_Font(font_name, font_size, font_bold);
}

SuperweaponInfo::~SuperweaponInfo()
{
    if (m_name != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_name);
        m_name = nullptr;
    }

    if (m_time != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_time);
        m_time = nullptr;
    }
}

void SuperweaponInfo::Set_Font(const Utf8String &name, int size, bool bold)
{
    m_name->Set_Font(g_theFontLibrary->Get_Font(name, g_theGlobalLanguage->Adjust_Font_Size(size), bold));
    m_time->Set_Font(g_theFontLibrary->Get_Font(name, g_theGlobalLanguage->Adjust_Font_Size(size), bold));
}

void SuperweaponInfo::Set_Text(Utf16String &name, Utf16String &time)
{
    m_name->Set_Text(name);
    m_time->Set_Text(time);
}

void SuperweaponInfo::Draw_Name(int x, int y, int color, int border_color)
{
    if (color == 0) {
        color = m_color;
    }

    m_name->Draw(x - m_name->Get_Width(-1), y, color, border_color);
}

void SuperweaponInfo::Draw_Time(int x, int y, int color, int border_color)
{
    if (color == 0) {
        color = m_color;
    }

    m_time->Draw(x, y, color, border_color);
}

float SuperweaponInfo::Get_Height() const
{
    return m_name->Get_Font()->m_height;
}

void InGameUI::CRC_Snapshot(Xfer *xfer) {}

void InGameUI::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 3;
    xfer->xferVersion(&version, 3);

    if (version >= 2) {
        xfer->xferInt(&m_namedTimerLastFlashFrame);
        xfer->xferBool(&m_namedTimerUsedFlashColor);
        xfer->xferBool(&m_showNamedTimers);

        if (xfer->Get_Mode() == XFER_SAVE) {
            int timer_count = m_namedTimers.size();
            xfer->xferInt(&timer_count);

            for (auto it = m_namedTimers.begin(); it != m_namedTimers.end(); it++) {
                xfer->xferAsciiString(&it->second->m_timerName);
                xfer->xferUnicodeString(&it->second->m_timerText);
                xfer->xferBool(&it->second->m_countdown);
            }
        } else {
            int timer_count;
            xfer->xferInt(&timer_count);

            for (int i = 0; i < timer_count; i++) {
                Utf8String timer_name;
                Utf16String timer_text;
                bool countdown;
                xfer->xferAsciiString(&timer_name);
                xfer->xferUnicodeString(&timer_text);
                xfer->xferBool(&countdown);
                Add_Named_Timer(timer_name, timer_text, countdown);
            }
        }
    }

    xfer->xferBool(&m_superweaponDisplayEnabledByScript);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (int player_index = 0; player_index < MAX_PLAYER_COUNT; player_index++) {
            for (auto it = m_superweapons[player_index].begin(); it != m_superweapons[player_index].end(); it++) {
                for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                    SuperweaponInfo *info = *it2;
                    xfer->xferInt(&player_index);
                    Utf8String template_name = info->Get_Special_Power_Template()->Get_Name();
                    xfer->xferAsciiString(&template_name);
                    xfer->xferObjectID(&info->m_objectID);
                    xfer->xferUnsignedInt(&info->m_countdownSeconds);
                    xfer->xferBool(&info->m_hidden);
                    xfer->xferBool(&info->m_isMissingScience);
                    xfer->xferBool(&info->m_isReady);
                    xfer->xferBool(&info->m_hasPlayedSound);
                }
            }
        }

        int last_player_index = -1;
        xfer->xferInt(&last_player_index);
    } else if (xfer->Get_Mode() == XFER_LOAD) {
        for (;;) {
            int player_index;
            xfer->xferInt(&player_index);

            if (player_index == -1) {
                break;
            }

            captainslog_relassert(player_index < MAX_PLAYER_COUNT, CODE_06, "SWInfo bad plyrindex");
            Utf8String template_name;
            xfer->xferAsciiString(&template_name);
            const SpecialPowerTemplate *power_template = g_theSpecialPowerStore->Find_Special_Power_Template(template_name);
            captainslog_relassert(power_template != nullptr, CODE_06, "power %s not found", template_name.Str());
            Utf8String power_name;
            ObjectID object_id;
            unsigned int countdown_seconds;
            bool hidden;
            bool is_missing_science;
            bool is_ready;
            bool has_played_sound;
            xfer->xferAsciiString(&power_name);
            xfer->xferObjectID(&object_id);
            xfer->xferUnsignedInt(&countdown_seconds);
            xfer->xferBool(&hidden);
            xfer->xferBool(&is_missing_science);
            xfer->xferBool(&is_ready);
            xfer->xferBool(&has_played_sound);

            SuperweaponInfo *superweapon_info = Find_SW_Info(player_index, power_name, object_id, power_template);

            if (superweapon_info != nullptr) {
                superweapon_info->m_countdownSeconds = countdown_seconds;
                superweapon_info->m_hidden = hidden;
                superweapon_info->m_isMissingScience = is_missing_science;
                superweapon_info->m_isReady = is_ready;
                superweapon_info->m_hasPlayedSound = has_played_sound;
            } else {
                Player *player = g_thePlayerList->Get_Nth_Player(player_index);
                superweapon_info = new SuperweaponInfo(object_id,
                    countdown_seconds,
                    hidden,
                    is_missing_science,
                    is_ready,
                    has_played_sound,
                    m_superweaponNormalFont,
                    m_superweaponNormalPointSize,
                    m_superweaponNormalBold,
                    player->Get_Color(),
                    power_template);
                m_superweapons[player_index][power_name].push_back(superweapon_info);
            }
            superweapon_info->m_refreshCountdown = true;
        }
    }
}

void InGameUI::Load_Post_Process() {}

SuperweaponInfo *InGameUI::Find_SW_Info(
    int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template)
{
    auto it = m_superweapons[player_index].find(power_name);

    if (it != m_superweapons[player_index].end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            if ((*it2)->m_objectID == id) {
                return *it2;
            }
        }
    }

    return nullptr;
}

void InGameUI::Add_Named_Timer(const Utf8String &timer_name, const Utf16String &text, bool countdown)
{
    NamedTimerInfo *info = new NamedTimerInfo();
    info->m_timerName = timer_name;
    info->m_color = m_namedTimerNormalColor;
    info->m_timerText = text;
    info->m_displayString = g_theDisplayStringManager->New_Display_String();
    info->m_displayString->Reset();
    info->m_displayString->Set_Font(g_theFontLibrary->Get_Font(
        m_namedTimerNormalFont, g_theGlobalLanguage->Adjust_Font_Size(m_namedTimerNormalPointSize), m_namedTimerNormalBold));
    info->m_displayString->Set_Text(Utf16String::s_emptyString);
    info->m_timestamp = -1;
    info->m_countdown = countdown;
    Remove_Named_Timer(timer_name);
    m_namedTimers[timer_name] = info;
}

void InGameUI::Remove_Named_Timer(const Utf8String &timer_name)
{
    auto it = m_namedTimers.find(timer_name);

    if (it != m_namedTimers.end()) {
        g_theDisplayStringManager->Free_Display_String(it->second->m_displayString);
        it->second->Delete_Instance();
        m_namedTimers.erase(it);
    }
}

void InGameUI::Set_Mouse_Cursor(MouseCursor cursor)
{
    if (g_theMouse != nullptr) {
        g_theMouse->Set_Cursor(cursor);

        if (m_mouseMode == MOUSEMODE_GUI_COMMAND && cursor != CURSOR_ARROW && cursor != CURSOR_SCROLL) {
            m_mouseCursor = cursor;
        }
    }
}

void InGameUI::Add_Superweapon(
    int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template)
{
    if (power_template != nullptr && !Find_SW_Info(player_index, power_name, id, power_template)) {
        Player *player = g_thePlayerList->Get_Nth_Player(player_index);
        bool is_missing_science = false;

        if (power_template->Get_Required_Science() != SCIENCE_INVALID) {
            if (!player->Has_Science(power_template->Get_Required_Science())) {
                is_missing_science = true;
            }
        }

        captainslog_debug("Adding superweapon UI timer");
        SuperweaponInfo *info = new SuperweaponInfo(id,
            -1,
            false,
            is_missing_science,
            false,
            false,
            m_superweaponNormalFont,
            m_superweaponNormalPointSize,
            m_superweaponNormalBold,
            player->Get_Color(),
            power_template);
        m_superweapons[player_index][power_name].push_back(info);
    }
}

bool InGameUI::Remove_Superweapon(
    int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template)
{
    captainslog_debug("Removing superweapon UI timer");
    auto it = m_superweapons[player_index].find(power_name);

    if (it != m_superweapons[player_index].end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            if ((*it2)->m_objectID == id) {
                SuperweaponInfo *info = *it2;
                it->second.erase(it2);
                info->Delete_Instance();

                if (it->second.size() == 0) {
                    m_superweapons[player_index].erase(it);
                }

                return true;
            }
        }
    }

    return false;
}

void InGameUI::Object_Changed_Team(const Object *obj, int old_player_index, int new_player_index)
{
    if (obj != nullptr && old_player_index >= 0 && new_player_index >= 0) {
        ObjectID id = obj->Get_ID();
        Utf8String name;

        for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
            SpecialPowerModuleInterface *power = (*module)->Get_Special_Power();

            if (power != nullptr) {
                const SpecialPowerTemplate *tmplate = power->Get_Special_Power_Template();
                name = tmplate->Get_Name();
                auto it = m_superweapons[old_player_index].find(name);
                bool added = false;

                if (it != m_superweapons[old_player_index].end()) {
                    for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                        if ((*it2)->m_objectID == id) {
                            Remove_Superweapon(old_player_index, name, id, tmplate);
                            Add_Superweapon(new_player_index, name, id, tmplate);
                            added = true;
                            break;
                        }
                    }
                }

                if (!added && g_theGameLogic->Get_Frame() == 0) {
                    if (!obj->Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)
                        && !obj->Is_KindOf(KINDOF_COMMANDCENTER)) {
                        Add_Superweapon(new_player_index, name, id, tmplate);
                    }
                }
            }
        }
    }
}

void InGameUI::Hide_Object_Superweapon_Display_By_Script(const Object *obj)
{
    ObjectID id = obj->Get_ID();

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        for (auto it = m_superweapons[i].begin(); it != m_superweapons[i].end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                if ((*it2)->m_objectID == id) {
                    (*it2)->m_hidden = true;
                }
            }
        }
    }
}

void InGameUI::Show_Object_Superweapon_Display_By_Script(const Object *obj)
{
    ObjectID id = obj->Get_ID();

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        for (auto it = m_superweapons[i].begin(); it != m_superweapons[i].end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                if ((*it2)->m_objectID == id) {
                    (*it2)->m_hidden = false;
                }
            }
        }
    }
}

void InGameUI::Set_Superweapon_Display_Enabled_By_Script(bool enable)
{
    m_superweaponDisplayEnabledByScript = enable == 0;
}

bool InGameUI::Get_Superweapon_Display_Enabled_By_Script()
{
    return m_superweaponDisplayEnabledByScript;
}

void InGameUI::Init()
{
    INI ini;
    ini.Load("Data\\INI\\InGameUI.ini", INI_LOAD_OVERWRITE, nullptr);

    if (g_theGlobalLanguage != nullptr) {
        if (g_theGlobalLanguage->Drawable_Caption_Font().Name().Is_Not_Empty()) {
            m_drawableCaptionFont = g_theGlobalLanguage->Drawable_Caption_Font().Name();
            m_drawableCaptionPointSize = g_theGlobalLanguage->Drawable_Caption_Font().Point_Size();
            m_drawableCaptionBold = g_theGlobalLanguage->Drawable_Caption_Font().Bold();
        }

        if (g_theGlobalLanguage->Message_Font().Name().Is_Not_Empty()) {
            m_messageFont = g_theGlobalLanguage->Message_Font().Name();
            m_messagePointSize = g_theGlobalLanguage->Message_Font().Point_Size();
            m_messageBold = g_theGlobalLanguage->Message_Font().Bold();
        }

        if (g_theGlobalLanguage->Military_Caption_Title_Font().Name().Is_Not_Empty()) {
            m_militaryCaptionTitleFont = g_theGlobalLanguage->Military_Caption_Title_Font().Name();
            m_militaryCaptionTitlePointSize = g_theGlobalLanguage->Military_Caption_Title_Font().Point_Size();
            m_militaryCaptionTitleBold = g_theGlobalLanguage->Military_Caption_Title_Font().Bold();
        }

        if (g_theGlobalLanguage->Military_Caption_Font().Name().Is_Not_Empty()) {
            m_militaryCaptionFont = g_theGlobalLanguage->Military_Caption_Font().Name();
            m_militaryCaptionPointSize = g_theGlobalLanguage->Military_Caption_Font().Point_Size();
            m_militaryCaptionBold = g_theGlobalLanguage->Military_Caption_Font().Bold();
        }

        if (g_theGlobalLanguage->Superweapon_Countdown_Normal_Font().Name().Is_Not_Empty()) {
            m_superweaponNormalFont = g_theGlobalLanguage->Superweapon_Countdown_Normal_Font().Name();
            m_superweaponNormalPointSize = g_theGlobalLanguage->Superweapon_Countdown_Normal_Font().Point_Size();
            m_superweaponNormalBold = g_theGlobalLanguage->Superweapon_Countdown_Normal_Font().Bold();
        }

        if (g_theGlobalLanguage->Superweapon_Countdown_Ready_Font().Name().Is_Not_Empty()) {
            m_superweaponReadyFont = g_theGlobalLanguage->Superweapon_Countdown_Ready_Font().Name();
            m_superweaponReadyPointSize = g_theGlobalLanguage->Superweapon_Countdown_Ready_Font().Point_Size();
            m_superweaponReadyBold = g_theGlobalLanguage->Superweapon_Countdown_Ready_Font().Bold();
        }

        if (g_theGlobalLanguage->Named_Timer_Countdown_Normal_Font().Name().Is_Not_Empty()) {
            m_namedTimerNormalFont = g_theGlobalLanguage->Named_Timer_Countdown_Normal_Font().Name();
            m_namedTimerNormalPointSize = g_theGlobalLanguage->Named_Timer_Countdown_Normal_Font().Point_Size();
            m_namedTimerNormalBold = g_theGlobalLanguage->Named_Timer_Countdown_Normal_Font().Bold();
        }

        if (g_theGlobalLanguage->Named_Timer_Countdown_Ready_Font().Name().Is_Not_Empty()) {
            m_namedTimerReadyFont = g_theGlobalLanguage->Named_Timer_Countdown_Ready_Font().Name();
            m_namedTimerReadyPointSize = g_theGlobalLanguage->Named_Timer_Countdown_Ready_Font().Point_Size();
            m_namedTimerReadyBold = g_theGlobalLanguage->Named_Timer_Countdown_Ready_Font().Bold();
        }
    }

    if (g_theDisplay != nullptr) {
        g_theTacticalView = Create_View();
        g_theTacticalView->Init();
        g_theDisplay->Attach_View(g_theTacticalView);
        g_theTacticalView->Set_Width(g_theDisplay->Get_Width());
        g_theTacticalView->Set_Height(g_theDisplay->Get_Height() * 0.76999998f);
    }

    g_theTacticalView->Set_Default_View(0.0f, 0.0f, 1.0f);
    Create_Control_Bar();
    Create_Replay_Control();
    g_theControlBar = new ControlBar();
    g_theControlBar->Init();
    m_windowLayoutList.clear();
    m_soloNexusSelectedDrawableID = INVALID_DRAWABLE_ID;
}

void InGameUI::Reset()
{
    m_isQuitMenuVisible = false;
    m_inputEnabled = true;
    g_theControlBar->Reset();
    g_theTacticalView->Set_Default_View(0.0f, 0.0f, 1.0f);
    Reset_In_Game_Chat();
    Stop_Movie();
    Set_GUI_Command(nullptr);
    Place_Build_Available(nullptr, nullptr);
    Free_Message_Resources();

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        for (auto it = m_superweapons[i].begin(); it != m_superweapons[i].end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                (*it2)->Delete_Instance();
            }

            it->second.clear();
        }

        m_superweapons[i].clear();
    }

    for (auto it = m_namedTimers.begin(); it != m_namedTimers.end(); it++) {
        g_theDisplayStringManager->Free_Display_String(it->second->m_displayString);
        it->second->Delete_Instance();
    }

    m_namedTimers.clear();
    m_namedTimerLastFlashFrame = 0;
    m_namedTimerUsedFlashColor = true;
    m_showNamedTimers = true;
    Remove_Military_Subtitle();
    Clear_Popup_Message_Data();
    m_superweaponLastFlashFrame = 0;
    m_superweaponHidden = true;
    m_superweaponDisplayEnabledByScript = false;
    Clear_Floating_Text();
    Clear_World_Animations();
    Reset_Idle_Worker();

    for (int i = 0; i < MAX_MOVE_HINTS; i++) {
        m_moveHint[i].pos.Zero();
        m_moveHint[i].source_id = 0;
        m_moveHint[i].frame = 0;
    }

    m_waypointMode = false;
    m_forceToAttackMode = false;
    m_forceToMoveMode = false;
    m_attackMoveToMode = false;
    m_preferSelection = false;
    m_noRadarEdgeSound = false;
    m_windowLayoutList.clear();
    m_tooltipsDisabled = 0;
    Update_Diplomacy_Briefing_Text(Utf8String::s_emptyString, true);
}

void InGameUI::Free_Message_Resources()
{
    for (int i = 0; i < MAX_UI_MESSAGES; i++) {
        m_uiMessages[i].full_text.Clear();

        if (m_uiMessages[i].display_string != nullptr) {
            g_theDisplayStringManager->Free_Display_String(m_uiMessages[i].display_string);
            m_uiMessages[i].display_string = nullptr;
        }

        m_uiMessages[i].timestamp = 0;
    }
}

void InGameUI::Create_Control_Bar()
{
    g_theWindowManager->Win_Create_From_Script("ControlBar.wnd", nullptr);
    Hide_Control_Bar(true);
}

void InGameUI::Create_Replay_Control()
{
    g_replayWindow = g_theWindowManager->Win_Create_From_Script("ReplayControl.wnd", nullptr);
}

void InGameUI::Clear_Floating_Text()
{
    for (auto it = m_floatingTextList.begin(); it != m_floatingTextList.end(); it++) {
        FloatingTextData *data = *it;
        m_floatingTextList.erase(it);
        data->Delete_Instance();
    }
}

void InGameUI::Clear_Popup_Message_Data()
{
    if (m_popupMessageData != nullptr) {
        if (m_popupMessageData->m_windowLayout != nullptr) {
            m_popupMessageData->m_windowLayout->Destroy_Windows();
            m_popupMessageData->m_windowLayout->Delete_Instance();
            m_popupMessageData->m_windowLayout = nullptr;
        }

        if (m_popupMessageData->m_pause) {
            g_theGameLogic->Set_Game_Paused(false, m_popupMessageData->m_pauseMusic);
        }

        m_popupMessageData->Delete_Instance();
        m_popupMessageData = nullptr;
    }
}

void InGameUI::Clear_World_Animations()
{
    for (auto it = m_worldAnimations.begin(); it != m_worldAnimations.end(); it++) {
        WorldAnimationData *anim = *it;

        if (anim != nullptr) {
            anim->m_anim->Delete_Instance();
            delete anim;
        }

        m_worldAnimations.erase(it);
    }
}

void InGameUI::Post_Draw()
{
    if (m_messagesOn) {
        int message_x = m_messagePosition.x;
        int message_y = m_messagePosition.y;

        for (int i = MAX_UI_MESSAGES - 1; i >= 0; i--) {
            if (m_uiMessages[i].display_string != nullptr) {
                unsigned char red;
                unsigned char green;
                unsigned char blue;
                unsigned char alpha;
                Get_Color_Components(m_uiMessages[i].color, &red, &green, &blue, &alpha);
                m_uiMessages[i].display_string->Draw(
                    message_x, message_y, m_uiMessages[i].color, Make_Color(0, 0, 0, alpha));
                message_y += m_uiMessages[i].display_string->Get_Font()->m_height;
            }
        }
    }

    if (m_militarySubtitle != nullptr) {
        ICoord2D position = m_militarySubtitle->position;
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
        Get_Color_Components(m_militarySubtitle->color, &red, &green, &blue, &alpha);
        int border_color = Make_Color(0, 0, 0, alpha);

        for (unsigned int j = 0; j <= m_militarySubtitle->current_display_string; j++) {
            m_militarySubtitle->display_strings[j]->Draw(position.x, position.y, m_militarySubtitle->color, border_color);
            int height;
            m_militarySubtitle->display_strings[j]->Get_Size(nullptr, &height);
            position.y += height;
        }

        if (m_militarySubtitle->block_drawn) {
            int height =
                m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Get_Font()->m_height;
            int width = height * 0.8f;
            g_theDisplay->Draw_Fill_Rect(
                m_militarySubtitle->block_pos.x, m_militarySubtitle->block_pos.y, width, height, m_militarySubtitle->color);
        }
    }

    if (g_theGameLogic->Get_Frame() != 0) {
        int superweapon_x = g_theDisplay->Get_Width() * m_superweaponPosition.x;
        int superweapon_y = g_theDisplay->Get_Height() * m_superweaponPosition.y;
        int view_height = g_theTacticalView->Get_Height() * 0.82f;
        bool draw_limit_reached = false;

        for (int k = 0; k < MAX_PLAYER_COUNT; k++) {
            int border_color = Make_Color(0, 0, 0, 255);

            for (auto it = m_superweapons[k].begin(); it != m_superweapons[k].end(); it++) {
                Utf8String name(it->first);

                for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                    SuperweaponInfo *info = *it2;
                    captainslog_dbgassert(info != nullptr, "No superweapon info!");

                    if (info != nullptr && !info->m_hidden && !info->m_isMissingScience) {
                        if (superweapon_y >= view_height) {
                            Utf16String str1;
                            str1 = U_CHAR("...");
                            info->Set_Text(str1, str1);
                            info->Set_Font(m_superweaponReadyFont, m_superweaponNormalPointSize, m_superweaponNormalBold);
                            info->Draw_Time(superweapon_x, superweapon_y, m_superweaponFlashColor, border_color);
                            draw_limit_reached = true;
                        }

                        Object *obj = g_theGameLogic->Find_Object_By_ID(info->m_objectID);

                        if (obj != nullptr) {
                            if (!obj->Get_Status(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                                SpecialPowerModuleInterface *module =
                                    obj->Get_Special_Power_Module(info->Get_Special_Power_Template());

                                if (module != nullptr) {
                                    bool is_ready = module->Is_Ready();
                                    int countdown_seconds;

                                    if (module->Get_Ready_Frame() >= g_theGameLogic->Get_Frame()) {
                                        countdown_seconds = (module->Get_Ready_Frame() - g_theGameLogic->Get_Frame()) / 30;
                                    } else {
                                        countdown_seconds = 0;
                                    }

                                    if (!is_ready || info->m_hasPlayedSound) {
                                        if (!is_ready) {
                                            info->m_hasPlayedSound = false;
                                        }
                                    } else {
                                        if (g_theGameLogic->Get_Frame() != 0) {
                                            SpecialPowerType type = module->Get_Special_Power_Template()->Get_Type();
                                            Player *player = g_thePlayerList->Get_Local_Player();

                                            switch (type) {
                                                case SPECIAL_PARTICLE_UPLINK_CANNON:
                                                case SUPW_SPECIAL_PARTICLE_UPLINK_CANNON:
                                                case LAZR_SPECIAL_PARTICLE_UPLINK_CANNON:
                                                    if (player == obj->Get_Controlling_Player()) {
                                                        g_theEva->Set_Should_Play(
                                                            EVA_MESSAGE_SUPERWEAPONREADY_OWN_PARTICLECANNON);
                                                    } else {
                                                        if (player->Get_Relationship(obj->Get_Team()) != ENEMIES) {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ALLY_PARTICLECANNON);
                                                        } else {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_PARTICLECANNON);
                                                        }
                                                    }
                                                    break;
                                                case SPECIAL_NEUTRON_MISSILE:
                                                case NUKE_SPECIAL_NEUTRON_MISSILE:
                                                case SUPW_SPECIAL_NEUTRON_MISSILE:
                                                    if (player == obj->Get_Controlling_Player()) {
                                                        g_theEva->Set_Should_Play(EVA_MESSAGE_SUPERWEAPONREADY_OWN_NUKE);
                                                    } else {
                                                        if (player->Get_Relationship(obj->Get_Team()) != ENEMIES) {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ALLY_NUKE);
                                                        } else {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_NUKE);
                                                        }
                                                    }
                                                    break;
                                                case SPECIAL_SCUD_STORM:
                                                    if (player == obj->Get_Controlling_Player()) {
                                                        g_theEva->Set_Should_Play(
                                                            EVA_MESSAGE_SUPERWEAPONREADY_OWN_SCUDSTORM);
                                                    } else {
                                                        if (player->Get_Relationship(obj->Get_Team()) != ENEMIES) {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ALLY_SCUDSTORM);
                                                        } else {
                                                            g_theEva->Set_Should_Play(
                                                                EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_SCUDSTORM);
                                                        }
                                                    }
                                                    break;
                                            }
                                        }

                                        info->m_hasPlayedSound = true;
                                    }

                                    if (!m_superweaponDisplayEnabledByScript && !draw_limit_reached) {
                                        if (countdown_seconds != info->m_countdownSeconds || is_ready != info->m_isReady
                                            || info->m_refreshCountdown) {
                                            if (is_ready) {
                                                info->Set_Font(m_superweaponReadyFont,
                                                    m_superweaponReadyPointSize,
                                                    m_superweaponReadyBold);
                                            } else if (info->m_countdownSeconds == 0) {
                                                info->Set_Font(m_superweaponNormalFont,
                                                    m_superweaponNormalPointSize,
                                                    m_superweaponNormalBold);
                                            }

                                            info->m_refreshCountdown = false;
                                            info->m_isReady = is_ready;
                                            info->m_countdownSeconds = countdown_seconds;
                                            int minutes = countdown_seconds / 60;
                                            int seconds = countdown_seconds % 60;

                                            Utf8String text;
                                            text.Format("GUI:%s", name.Str());
                                            Utf16String title;
                                            Utf16String time;
                                            title.Format(U_CHAR("%ls: "), g_theGameText->Fetch(text.Str()).Str());
                                            time.Format(U_CHAR("%d:%2.2d"), minutes, seconds);
                                            info->Set_Text(title, time);
                                        }

                                        if (!is_ready || m_superweaponFlashDuration == 0.0f) {
                                            info->Draw_Name(superweapon_x, superweapon_y, 0, border_color);
                                            info->Draw_Time(superweapon_x, superweapon_y, 0, border_color);
                                        } else {
                                            if (g_theGameLogic->Get_Frame()
                                                >= (unsigned int)m_superweaponFlashDuration + m_superweaponLastFlashFrame) {
                                                m_superweaponHidden = !m_superweaponHidden;
                                                m_superweaponLastFlashFrame = g_theGameLogic->Get_Frame();
                                            }

                                            info->Draw_Name(superweapon_x,
                                                superweapon_y,
                                                m_superweaponHidden ? 0 : m_superweaponFlashColor,
                                                border_color);
                                            info->Draw_Time(superweapon_x,
                                                superweapon_y,
                                                m_superweaponHidden ? 0 : m_superweaponFlashColor,
                                                border_color);
                                        }

                                        superweapon_y += info->Get_Height();
                                    }

                                    if (info->Get_Special_Power_Template()->Get_Shared_Synced_Timer()) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (g_theGameLogic->Get_Frame() != 0 && m_showNamedTimers) {
        bool is_right_aligned = m_namedTimerPosition.x >= 0.5f;
        int timer_x = g_theDisplay->Get_Width() * m_namedTimerPosition.x;
        int timer_y = g_theDisplay->Get_Height() * m_namedTimerPosition.y;
        int border_color = Make_Color(0, 0, 0, 255);

        for (auto it = m_namedTimers.begin(); it != m_namedTimers.end(); it++) {
            Utf8String timer_name(it->first);
            NamedTimerInfo *timer_info = it->second;
            captainslog_dbgassert(timer_info != nullptr, "No namedTimer info!");

            if (timer_info != nullptr) {
                Utf16String timer_time;
                int timer_value = g_theScriptEngine->Get_Counter(timer_name)->value;
                unsigned int timer_frame = g_theGameLogic->Get_Frame();

                if (timer_value > 0) {
                    timer_frame += timer_value;
                }

                int timer_frame2 = (timer_frame - g_theGameLogic->Get_Frame()) * SECONDS_PER_LOGICFRAME_REAL;

                if ((timer_info->m_countdown && timer_frame2 != timer_info->m_timestamp)
                    || (!timer_info->m_countdown && timer_value != timer_info->m_timestamp)) {
                    if (timer_frame2 != 0 || !timer_info->m_countdown) {
                        if (timer_info->m_timestamp == 0 || timer_info->m_countdown) {
                            timer_info->m_displayString->Set_Font(g_theFontLibrary->Get_Font(m_namedTimerNormalFont,
                                g_theGlobalLanguage->Adjust_Font_Size(m_namedTimerNormalPointSize),
                                m_namedTimerNormalBold));
                        }
                    } else {
                        if (timer_info->m_timestamp == 0 || timer_info->m_countdown) {
                            timer_info->m_displayString->Set_Font(g_theFontLibrary->Get_Font(m_namedTimerReadyFont,
                                g_theGlobalLanguage->Adjust_Font_Size(m_namedTimerReadyPointSize),
                                m_namedTimerReadyBold));
                        }
                    }

                    timer_info->m_timestamp = timer_frame2;
                    int minutes = timer_frame2 / 60;
                    int seconds = timer_frame2 % 60;

                    if (timer_info->m_countdown) {
                        if (seconds < 10) {
                            timer_time.Format(U_CHAR("%s %d:0%d"), timer_info->m_timerText.Str(), minutes, seconds);
                        } else {
                            timer_time.Format(U_CHAR("%s %d:%d"), timer_info->m_timerText.Str(), minutes, seconds);
                        }
                    } else {
                        timer_time.Format(U_CHAR("%s %d"), timer_info->m_timerText.Str(), timer_value);
                    }

                    timer_info->m_displayString->Set_Text(timer_time);
                }

                int timer_x_aligned = timer_x;

                if (is_right_aligned) {
                    timer_x_aligned -= timer_info->m_displayString->Get_Width(-1);
                }

                if (timer_frame2 != 0 || !timer_info->m_countdown || m_namedTimerFlashDuration == 0.0f) {
                    timer_info->m_displayString->Draw(timer_x_aligned, timer_y, timer_info->m_color, border_color);
                } else {
                    if (g_theGameLogic->Get_Frame()
                        >= (unsigned int)m_namedTimerFlashDuration + m_namedTimerLastFlashFrame) {
                        m_namedTimerUsedFlashColor = !m_namedTimerUsedFlashColor;
                        m_namedTimerLastFlashFrame = g_theGameLogic->Get_Frame();
                    }

                    timer_info->m_displayString->Draw(timer_x_aligned,
                        timer_y,
                        m_namedTimerUsedFlashColor ? timer_info->m_color : m_namedTimerFlashColor,
                        border_color);
                }

                timer_y -= timer_info->m_displayString->Get_Font()->m_height;
            }
        }
    }

    if (g_theLookAtTranslator != nullptr) {
        if (m_drawRMBScrollAnchor) {
            const ICoord2D *anchor = g_theLookAtTranslator->Get_RMB_Scroll_Anchor();

            if (anchor != nullptr) {
                static int mainColor = Make_Color(0, 255, 0, 255);
                static int dropColor = Make_Color(0, 0, 0, 255);

                g_theDisplay->Draw_Fill_Rect(anchor->x - 9, anchor->y - 3, 19, 7, dropColor);
                g_theDisplay->Draw_Fill_Rect(anchor->x - 3, anchor->y - 9, 7, 19, dropColor);
                g_theDisplay->Draw_Fill_Rect(anchor->x - 8, anchor->y - 2, 17, 5, mainColor);
                g_theDisplay->Draw_Fill_Rect(anchor->x - 2, anchor->y - 8, 5, 17, mainColor);
            }
        }
    }

    g_theControlBar->Draw_Special_Power_Shortcut_Multiplier_Text();
}

void InGameUI::Add_World_Animation(
    Anim2DTemplate *anim, const Coord3D *pos, WorldAnimationOptions options, float time, float z_rise)
{
    if (anim != nullptr && pos != nullptr && time > 0.0f) {
        WorldAnimationData *world = new WorldAnimationData();
        world->m_anim = new Anim2D(anim, g_theAnim2DCollection);
        world->m_frame = g_theGameLogic->Get_Frame() + time * 30.0f;
        world->m_options = options;
        world->m_pos = *pos;
        world->m_zRise = z_rise;
        m_worldAnimations.push_front(world);
    }
}

bool InGameUI::Are_Selected_Objects_Controllable() const
{
    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        return (*it)->Get_Object()->Is_Locally_Controlled();
    }

    return false;
}

void InGameUI::Parse_In_Game_UI_Definition(INI *ini)
{
    if (g_theInGameUI != nullptr) {
        ini->Init_From_INI(g_theInGameUI, g_theInGameUI->Get_Field_Parse());
    }
}

void InGameUI::Show_Named_Timer_Display(bool show)
{
    m_showNamedTimers = show;
}

void InGameUI::Set_Radius_Cursor(
    RadiusCursorType cursor, const SpecialPowerTemplate *power_template, WeaponSlotType weapon_slot)
{
    if (cursor != m_radiusDecalType) {
        m_radiusDecal.Clear();
        m_radiusDecalType = RADIUS_CURSOR_NONE;

        if (cursor != RADIUS_CURSOR_NONE) {
            Object *build_obj = nullptr;

            if (m_pendingGUICommand != nullptr
                && m_pendingGUICommand->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT) {
                if (g_thePlayerList != nullptr && g_thePlayerList->Get_Local_Player() != nullptr
                    && power_template != nullptr) {
                    build_obj = g_thePlayerList->Get_Local_Player()->Find_Most_Ready_Shortcut_Special_Power_Of_Type(
                        power_template->Get_Type());
                }
            } else {
                if (Get_Select_Count() == 0) {
                    return;
                }

                Drawable *drawable = Get_First_Selected_Drawable();

                if (drawable == nullptr) {
                    return;
                }

                build_obj = drawable->Get_Object();
            }

            if (build_obj != nullptr) {
                const Player *player = build_obj->Get_Controlling_Player();
                float range = 0.0f;
                const Weapon *weapon = nullptr;

                switch (cursor) {
                    case RADIUS_CURSOR_ATTACK_DAMAGE_AREA:
                        weapon = build_obj->Get_Weapon_In_Weapon_Slot(weapon_slot);

                        if (weapon != nullptr) {
                            range = weapon->Get_Primary_Damage_Radius(build_obj);
                        } else {
                            range = 0.0f;
                        }

                        break;
                    case RADIUS_CURSOR_ATTACK_SCATTER_AREA:
                        weapon = build_obj->Get_Weapon_In_Weapon_Slot(weapon_slot);

                        if (weapon != nullptr) {
                            range = weapon->Get_Scatter_Target_Scalar() + weapon->Get_Scatter_Radius();
                        } else {
                            range = 0.0f;
                        }

                        break;
                    case RADIUS_CURSOR_ATTACK_CONTINUE_AREA:
                    case RADIUS_CURSOR_CLEARMINES:
                        weapon = build_obj->Get_Weapon_In_Weapon_Slot(weapon_slot);

                        if (weapon != nullptr) {
                            range = weapon->Get_Continue_Attack_Range();
                        } else {
                            range = 0.0f;
                        }

                        break;
                    case RADIUS_CURSOR_GUARD_AREA:
                        range = AIGuardMachine::Get_Std_Guard_Range(build_obj);
                        break;
                    case RADIUS_CURSOR_EMERGENCY_REPAIR:
                    case RADIUS_CURSOR_FRIENDLY_SPECIALPOWER:
                    case RADIUS_CURSOR_OFFENSIVE_SPECIALPOWER:
                    case RADIUS_CURSOR_SUPERWEAPON_SCATTER_AREA:
                    case RADIUS_CURSOR_PARTICLECANNON:
                    case RADIUS_CURSOR_A10STRIKE:
                    case RADIUS_CURSOR_CARPETBOMB:
                    case RADIUS_CURSOR_DAISYCUTTER:
                    case RADIUS_CURSOR_PARADROP:
                    case RADIUS_CURSOR_SPYSATELLITE:
                    case RADIUS_CURSOR_SPECTREGUNSHIP:
                    case RADIUS_CURSOR_HELIX_NAPALM_BOMB:
                    case RADIUS_CURSOR_NUCLEARMISSILE:
                    case RADIUS_CURSOR_EMPPULSE:
                    case RADIUS_CURSOR_ARTILLERYBARRAGE:
                    case RADIUS_CURSOR_NAPALMSTRIKE:
                    case RADIUS_CURSOR_CLUSTERMINES:
                    case RADIUS_CURSOR_SCUDSTORM:
                    case RADIUS_CURSOR_ANTHRAXBOMB:
                    case RADIUS_CURSOR_AMBUSH:
                    case RADIUS_CURSOR_RADAR:
                    case RADIUS_CURSOR_SPYDRONE:
                    case RADIUS_CURSOR_FRENZY:
                    case RADIUS_CURSOR_AMBULANCE:
                        if (power_template != nullptr) {
                            range = power_template->Get_Radius_Cursor_Radius();
                        } else {
                            range = 0.0f;
                        }

                        break;
                    default:
                        break;
                }

                if (range > 0.0f) {
                    Coord3D pos;
                    pos.x = 0.0f;
                    pos.y = 0.0f;
                    pos.z = 0.0f;
                    m_radiusDecalTemplate[cursor].Create_Radius_Decal(&pos, range, player, &m_radiusDecal);
                    m_radiusDecalType = cursor;
                    Handle_Radius_Cursor();
                }
            }
        }
    }
}

void InGameUI::Handle_Radius_Cursor()
{
    if (!m_radiusDecal.Needs_Init()) {
        MouseIO *status = g_theMouse->Get_Mouse_Status();
        bool has_radar = true;

        if (!g_theRadar->Is_Radar_Forced()) {
            if (g_theRadar->Is_Radar_Hidden() || g_thePlayerList->Get_Local_Player() == nullptr
                || !g_thePlayerList->Get_Local_Player()->Has_Radar()) {
                has_radar = false;
            }
        }

        Coord3D world;

        if (!has_radar || !g_theRadar->Screen_Pixel_To_World(&status->pos, &world)) {
            g_theTacticalView->Screen_To_Terrain(&status->pos, &world);
        }

        if (g_theWriteableGlobalData->m_doubleClickAttackMovesEnabled && m_doubleClickCounter > 0) {
            m_radiusDecal.Set_Opacity(m_doubleClickCounter * 0.1f);
            m_radiusDecal.Set_Position(m_radiusDecalPos);
        } else {
            m_radiusDecal.Set_Position(world);
            m_radiusDecal.Update();
        }
    }
}

void InGameUI::Trigger_Double_Click_Attack_Move_Guard_Hint()
{
    m_doubleClickCounter = 11;
    MouseIO *status = g_theMouse->Get_Mouse_Status();
    g_theTacticalView->Screen_To_Terrain(&status->pos, &m_radiusDecalPos);
}

void InGameUI::Create_Command_Hint(const GameMessage *msg)
{
    if (m_isScrolling || m_isSelecting || g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK) {
        return;
    }

    Drawable *draw = g_theGameClient->Find_Drawable_By_ID(m_mousedOverDrawableID);
    GameMessage::MessageType type = msg->Get_Type();

    if (draw != nullptr
        && (type == GameMessage::MSG_DO_ATTACK_OBJECT_HINT || type == GameMessage::MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT)) {
        Object *obj = draw->Get_Object();
        int index;

        if (g_thePlayerList != nullptr) {
            index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            index = 0;
        }

#ifdef GAME_DEBUG_STRUCTS
        ObjectShroudStatus shrouded =
            obj != nullptr && g_theWriteableGlobalData->m_shroudOn ? obj->Get_Shrouded_Status(index) : SHROUDED_NONE;
#else
        ObjectShroudStatus shrouded = obj != nullptr ? obj->Get_Shrouded_Status(index) : SHROUDED_NONE;
#endif

        if (shrouded == SHROUDED_NEVERSEEN) {
            type = GameMessage::MSG_DO_MOVETO_HINT;
        }
    }

    Set_Radius_Cursor_None();

    if (g_theWriteableGlobalData->m_doubleClickAttackMovesEnabled && --m_doubleClickCounter > 0) {
        Set_Mouse_Cursor(CURSOR_FORCE_ATTACK_GROUND);
        Set_Radius_Cursor(RADIUS_CURSOR_GUARD_AREA, nullptr, WEAPONSLOT_PRIMARY);
        return;
    }

    GameWindow *parent = nullptr;
    MouseIO *status = g_theMouse->Get_Mouse_Status();
    bool is_blocking_window_under_cursor = false;

    if (status != nullptr && g_theWindowManager != nullptr) {
        parent = g_theWindowManager->Get_Window_Under_Cursor(status->pos.x, status->pos.y, false);
    }

    while (parent != nullptr) {
        if (parent->Win_Get_Input_Func() == Left_HUD_Input) {
            is_blocking_window_under_cursor = false;
            break;
        }

        if ((parent->Win_Get_Status() & WIN_STATUS_SEE_THRU) == 0) {
            is_blocking_window_under_cursor = true;
            break;
        }

        parent = parent->Win_Get_Parent();
    }

    Object *obj;

    if (draw != nullptr) {
        obj = draw->Get_Object();
    } else {
        obj = nullptr;
    }

    bool can_select = Can_Select_Drawable(draw, false);

    if (obj == nullptr) {
        can_select = false;
    }

    Drawable *unique_selected_draw = nullptr;
    Object *unique_selected_obj = nullptr;

    if (Get_Select_Count() == 1) {
        unique_selected_draw = Get_All_Selected_Drawables()->front();

        if (unique_selected_draw != nullptr) {
            unique_selected_obj = unique_selected_draw->Get_Object();
        } else {
            unique_selected_obj = nullptr;
        }
    }

    if (m_mouseMode != MOUSEMODE_DEFAULT) {
        if (m_mouseMode == MOUSEMODE_BUILD_PLACE) {
            if (is_blocking_window_under_cursor) {
                Set_Mouse_Cursor(CURSOR_ARROW);
                return;
            }

            if (type > GameMessage::MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT) {
                if (type != GameMessage::MSG_ADD_WAYPOINT) {
                    return;
                }
            } else {
                if (type == GameMessage::MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT
                    || type == GameMessage::MSG_DO_ATTACK_OBJECT_HINT) {
                    Set_Mouse_Cursor(CURSOR_INVALID_BUILD);
                    return;
                }

                if (type <= GameMessage::MSG_DOCK_HINT || type > GameMessage::MSG_DO_ATTACKMOVETO_HINT) {
                    return;
                }
            }

            Set_Mouse_Cursor(CURSOR_BUILD);
            return;
        }

        if (m_mouseMode != MOUSEMODE_GUI_COMMAND) {
            return;
        }

        if (is_blocking_window_under_cursor) {
            Set_Mouse_Cursor(CURSOR_ARROW);
            return;
        }

        if (m_pendingGUICommand != nullptr) {
            if (m_pendingGUICommand->Is_Context_Command() || m_pendingGUICommand->Get_Command() == GUI_COMMAND_SPECIAL_POWER
                || m_pendingGUICommand->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT) {
                Utf8String cursor_name;

                if (type == GameMessage::MSG_VALID_GUICOMMAND_HINT) {
                    cursor_name = m_pendingGUICommand->Get_Cursor_Name();
                } else {
                    cursor_name = m_pendingGUICommand->Get_Invalid_Cursor_Name();
                }

                MouseCursor cursor = g_theMouse->Get_Cursor_Index(cursor_name);

                if (cursor == CURSOR_INVALID) {
                    Set_Mouse_Cursor(CURSOR_TARGET);
                } else {
                    Set_Mouse_Cursor(cursor);
                }

                Set_Radius_Cursor(m_pendingGUICommand->Get_Radius_Cursor_Type(),
                    m_pendingGUICommand->Get_Special_Power(),
                    m_pendingGUICommand->Get_Weapon_Slot());
            } else if ((m_pendingGUICommand->Get_Options()
                           & (COMMAND_OPTION_CONTEXTMODE_COMMAND | COMMAND_OPTION_NEED_TARGET_POS
                               | COMMAND_OPTION_NEED_TARGET_OBJECT))
                != 0) {
                MouseCursor cursor = g_theMouse->Get_Cursor_Index(m_pendingGUICommand->Get_Cursor_Name());

                if (cursor == CURSOR_INVALID) {
                    Set_Mouse_Cursor(CURSOR_TARGET);
                } else {
                    Set_Mouse_Cursor(cursor);
                }

                Set_Radius_Cursor(m_pendingGUICommand->Get_Radius_Cursor_Type(),
                    m_pendingGUICommand->Get_Special_Power(),
                    m_pendingGUICommand->Get_Weapon_Slot());
            } else {
                Set_Radius_Cursor_None();
            }
        }
    } else {
        if (is_blocking_window_under_cursor
            || (unique_selected_obj != nullptr && !unique_selected_obj->Is_Locally_Controlled())) {
            Set_Mouse_Cursor(CURSOR_ARROW);
            return;
        }

        switch (type) {
            case GameMessage::MSG_DO_ATTACK_OBJECT_HINT:
                Set_Mouse_Cursor(CURSOR_ATTACK_OBJ);
                return;
            case GameMessage::MSG_UNK1:
            case GameMessage::MSG_DO_INVALID_HINT:
                Set_Mouse_Cursor(CURSOR_GENERIC_INVALID);
                return;
            case GameMessage::MSG_DO_FORCE_ATTACK_OBJECT_HINT:
                Set_Mouse_Cursor(CURSOR_FORCE_ATTACK_OBJ);
                return;
            case GameMessage::MSG_DO_FORCE_ATTACK_GROUND_HINT:
                Set_Mouse_Cursor(CURSOR_FORCE_ATTACK_GROUND);
                return;
            case GameMessage::MSG_GET_REPAIRED_HINT:
                Set_Mouse_Cursor(CURSOR_GET_REPAIRED);
                return;
            case GameMessage::MSG_GET_HEALED_HINT:
                Set_Mouse_Cursor(CURSOR_GET_HEALED);
                return;
            case GameMessage::MSG_DO_REPAIR_HINT:
                Set_Mouse_Cursor(CURSOR_DO_REPAIR);
                return;
            case GameMessage::MSG_RESUME_CONSTRUCTION_HINT:
                Set_Mouse_Cursor(CURSOR_RESUME_CONSTRUCTION);
                return;
            case GameMessage::MSG_ENTER_HINT:
                Set_Mouse_Cursor(CURSOR_ENTER_FRIENDLY);
                return;
            case GameMessage::MSG_DOCK_HINT:
                Set_Mouse_Cursor(CURSOR_DOCK);
                return;
            case GameMessage::MSG_DO_MOVETO_HINT:
                if (!can_select && unique_selected_obj != nullptr && unique_selected_obj->Is_Locally_Controlled()
                    && unique_selected_obj->Is_KindOf(KINDOF_STRUCTURE)) {
                    Set_Mouse_Cursor(CURSOR_GENERIC_INVALID);
                    return;
                }

                if (can_select && obj->Is_Locally_Controlled() && !obj->Is_KindOf(KINDOF_MINE)) {
                    Set_Mouse_Cursor(CURSOR_SELECT);
                    return;
                }

                if (g_theRadar->Is_Radar_Window(parent) && !g_theRadar->Is_Radar_Forced()) {
                    if (g_theRadar->Is_Radar_Hidden()) {
                        Set_Mouse_Cursor(CURSOR_ARROW);
                        return;
                    }

                    if (!g_thePlayerList->Get_Local_Player()->Has_Radar()) {
                        Set_Mouse_Cursor(CURSOR_ARROW);
                        return;
                    }
                }

                Set_Mouse_Cursor(CURSOR_MOVE);
                break;
            case GameMessage::MSG_DO_ATTACKMOVETO_HINT:
                if (can_select && obj->Is_Locally_Controlled()) {
                    Set_Mouse_Cursor(CURSOR_SELECT);
                    return;
                }

                Set_Mouse_Cursor(CURSOR_ATTACK_MOVE);
                return;
            case GameMessage::MSG_ADD_WAYPOINT_HINT:
                Set_Mouse_Cursor(CURSOR_WAYPOINT);
                return;
            case GameMessage::MSG_HIJACK_HINT:
            case GameMessage::MSG_SABOTAGE_HINT:
            case GameMessage::MSG_CONVERT_TO_CARBOMB_HINT:
                Set_Mouse_Cursor(CURSOR_ENTER_AGRESSIVE);
                return;
            case GameMessage::MSG_CAPTUREBUILDING_HINT:
                Set_Mouse_Cursor(CURSOR_CAPTURE_BUILDING);
                return;
            case GameMessage::MSG_DEFECTOR_HINT:
                Set_Mouse_Cursor(CURSOR_DEFECTOR);
                return;
            case GameMessage::MSG_SET_RALLY_POINT_HINT:
                if (can_select) {
                    Set_Mouse_Cursor(CURSOR_SELECT);
                    return;
                }

                Set_Mouse_Cursor(CURSOR_SET_RALLY_POINT);
                return;
            case GameMessage::MSG_UNK2:
                Set_Mouse_Cursor(CURSOR_PARTICLE_UPLINK_CANNON);
                return;
            case GameMessage::MSG_DO_SALVAGE_HINT:
                Set_Mouse_Cursor(CURSOR_MOVE);
                break;
            case GameMessage::MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT:
                Set_Mouse_Cursor(CURSOR_OUT_RANGE);
                return;
            case GameMessage::MSG_HACK_HINT:
                Set_Mouse_Cursor(CURSOR_HACK);
                return;
            default:
                return;
        }
    }
}

void InGameUI::Set_GUI_Command(const CommandButton *command)
{
    if (g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK) {
        return;
    }

    if (command != nullptr) {
        if ((command->Get_Options()
                & (COMMAND_OPTION_CONTEXTMODE_COMMAND | COMMAND_OPTION_NEED_TARGET_POS | COMMAND_OPTION_NEED_TARGET_OBJECT))
            == 0) {
            captainslog_dbgassert(
                "Set_GUI_Command: Command '%s' does not need additional user interaction", command->Get_Name().Str());
            m_pendingGUICommand = nullptr;
            m_mouseMode = MOUSEMODE_DEFAULT;
            return;
        }

        m_mouseMode = MOUSEMODE_GUI_COMMAND;
    } else {
        m_mouseMode = MOUSEMODE_DEFAULT;
    }

    m_pendingGUICommand = command;

    if (command != nullptr
        && (command->Get_Options()
               & (COMMAND_OPTION_CONTEXTMODE_COMMAND | COMMAND_OPTION_NEED_TARGET_POS | COMMAND_OPTION_NEED_TARGET_OBJECT))
            != 0
        && !command->Is_Context_Command()) {
        Set_Mouse_Cursor(CURSOR_ARROW);
        Set_Radius_Cursor(command->Get_Radius_Cursor_Type(), command->Get_Special_Power(), command->Get_Weapon_Slot());
    } else {
        if (g_theMouse != nullptr) {
            Set_Mouse_Cursor(CURSOR_ARROW);
        }

        Set_Radius_Cursor_None();
    }

    m_mouseCursor = g_theMouse->Get_Mouse_Cursor();
}

void InGameUI::Place_Build_Available(const ThingTemplate *build, Drawable *build_drawable)
{
    if (build != nullptr) {
        Set_Radius_Cursor_None();
    }

    if (m_pendingPlaceType != nullptr && build != nullptr) {
        Place_Build_Available(nullptr, nullptr);
    }

    m_pendingPlaceType = build;
    m_pendingPlaceSourceObjectID = INVALID_OBJECT_ID;
    Object *obj = nullptr;

    if (build_drawable != nullptr) {
        obj = build_drawable->Get_Object();
    }

    if (obj != nullptr) {
        m_pendingPlaceSourceObjectID = obj->Get_ID();
    }

    if (g_theMouse != nullptr) {
        if (build != nullptr) {
            m_mouseMode = MOUSEMODE_BUILD_PLACE;
            m_mouseCursor = CURSOR_TARGET;
            g_theMouse->Capture();
            Set_Mouse_Cursor(CURSOR_TARGET);
            Drawable *ghost_drawable = g_theThingFactory->New_Drawable(build, DRAWABLE_STATUS_8);

            if (obj != nullptr) {
                int color;

                if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
                    color = obj->Get_Controlling_Player()->Get_Night_Color();
                } else {
                    color = obj->Get_Controlling_Player()->Get_Color();
                }

                ghost_drawable->Set_Indicator_Color(color);
            }

            captainslog_dbgassert(
                ghost_drawable != nullptr, "Unable to create icon at cursor for placement '%s'", build->Get_Name().Str());
            ghost_drawable->Set_Orientation(build->Get_Placement_View_Angle());
            ghost_drawable->Set_Opacity(0.44999999f);
            captainslog_dbgassert(m_placeIcon[0] == nullptr, "Place_Build_Available, build icon array is not empty!");
            m_placeIcon[0] = ghost_drawable;
        } else {
            if (m_mouseMode == MOUSEMODE_BUILD_PLACE) {
                m_mouseMode = MOUSEMODE_DEFAULT;
                m_mouseCursor = CURSOR_ARROW;
            }

            g_theMouse->Release_Capture();
            Set_Mouse_Cursor(CURSOR_ARROW);
            Set_Placement_Start(nullptr);
            Destroy_Placement_Icons();

            if (obj != nullptr) {
                ProductionUpdateInterface *production = obj->Get_Production_Update_Interface();

                if (production != nullptr) {
                    production->Set_Special_Power_Construction_Command_Button(nullptr);
                }
            }
        }
    }
}

void InGameUI::Destroy_Placement_Icons()
{
    for (int i = 0; i < g_theWriteableGlobalData->m_maxLineBuildObjects; i++) {
        if (m_placeIcon[i] != nullptr) {
            g_theTerrainVisual->Remove_Faction_Bib_Drawable(m_placeIcon[i]);
            g_theGameClient->Destroy_Drawable(m_placeIcon[i]);
            m_placeIcon[i] = nullptr;
        }
    }

    g_theTerrainVisual->Remove_All_Bibs();
}

void InGameUI::Recreate_Control_Bar()
{
    GameWindow *window =
        g_theWindowManager->Win_Get_Window_From_Id(nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd"));

    if (window != nullptr) {
        window->Delete_Instance();
    }

    m_idleWorkerWin = nullptr;
    Create_Control_Bar();

    if (g_theControlBar != nullptr) {
        delete g_theControlBar;
        g_theControlBar = new ControlBar();
        g_theControlBar->Init();
    }
}

void InGameUI::Update()
{
    if (m_videoStream != nullptr && m_videoBuffer != nullptr && m_videoStream->Is_Frame_Ready()) {
        m_videoStream->Decompress_Frame();
        m_videoStream->Render_Frame(m_videoBuffer);
        m_videoStream->Next_Frame();

        if (m_videoStream->Frame_Index() == 0) {
            Stop_Movie();
        }
    }

    if (m_cameoVideoStream != nullptr && m_cameoVideoBuffer != nullptr && m_cameoVideoStream->Is_Frame_Ready()) {
        m_cameoVideoStream->Decompress_Frame();
        m_cameoVideoStream->Render_Frame(m_cameoVideoBuffer);
        m_cameoVideoStream->Next_Frame();

        if (m_cameoVideoStream->Frame_Index() == 0) {
            Stop_Movie();
        }
    }

    unsigned int frame = g_theGameLogic->Get_Frame();
    unsigned int message_timeout = m_messageDelayMS / 30 / 1000;

    for (int i = MAX_UI_MESSAGES - 1; i >= 0; i--) {
        if (frame - m_uiMessages[i].timestamp > message_timeout) {
            unsigned char red;
            unsigned char green;
            unsigned char blue;
            unsigned char alpha;
            Get_Color_Components(m_uiMessages[i].color, &red, &green, &blue, &alpha);
            int alpha_adjust = GameMath::Fast_To_Int_Truncate((frame - m_uiMessages[i].timestamp) * 0.01f);

            if (alpha - alpha_adjust >= 0) {
                alpha -= alpha_adjust;
            } else {
                alpha = 0;
            }

            m_uiMessages[i].color = Make_Color(red, green, blue, alpha);

            if (alpha == 0) {
                Remove_Message_At_Index(i);
            }
        }
    }

    if (m_militarySubtitle != nullptr) {
        if (g_theScriptEngine->Is_Time_Frozen_Script()) {
            m_militarySubtitle->lifetime--;
            m_militarySubtitle->block_begin_frame--;
            m_militarySubtitle->increment_on_frame--;
        }

        if (m_militarySubtitle->lifetime >= frame) {
            if (m_militarySubtitle->block_begin_frame + 9 < frame) {
                m_militarySubtitle->block_begin_frame = frame;
                m_militarySubtitle->block_drawn = !m_militarySubtitle->block_drawn;
            }

            if (m_militarySubtitle->increment_on_frame < frame) {
                unichar_t c = m_militarySubtitle->subtitle.Get_Char(m_militarySubtitle->index);

                if (c == U_CHAR('\n')) {
                    int y_size;
                    m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Get_Size(
                        nullptr, &y_size);
                    m_militarySubtitle->block_pos.y += y_size;

                    if (++m_militarySubtitle->current_display_string >= 4) {
                        m_militarySubtitle->index = m_militarySubtitle->subtitle.Get_Length();

                        captainslog_dbgassert(false, "You're Only Allowed to use %d lines of subtitle text", 4);
                    } else {
                        m_militarySubtitle->block_pos.x = m_militarySubtitle->position.x;
                        m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string] =
                            g_theDisplayStringManager->New_Display_String();
                        m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Reset();

                        m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Set_Font(
                            g_theFontLibrary->Get_Font(m_militaryCaptionFont,
                                g_theGlobalLanguage->Adjust_Font_Size(m_militaryCaptionPointSize),
                                m_militaryCaptionBold));
                        m_militarySubtitle->block_drawn = true;
                        m_militarySubtitle->increment_on_frame =
                            g_theGlobalLanguage->Get_Military_Caption_Delay_MS() * 30.0f / 1000.0f + frame;
                    }
                } else {
                    int x_size;
                    m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Add_Char(c);
                    m_militarySubtitle->display_strings[m_militarySubtitle->current_display_string]->Get_Size(
                        &x_size, nullptr);
                    m_militarySubtitle->block_pos.x = x_size + m_militarySubtitle->position.x;

                    static AudioEventRTS click("MilitarySubtitlesTyping");
                    g_theAudio->Add_Audio_Event(&click);

                    if (g_theGlobalLanguage != nullptr) {
                        m_militarySubtitle->increment_on_frame = g_theGlobalLanguage->Get_Military_Caption_Speed() + frame;
                    } else {
                        m_militarySubtitle->increment_on_frame = m_militaryCaptionSpeed + frame;
                    }
                }

                m_militarySubtitle->index++;

                if (m_militarySubtitle->index >= (unsigned int)m_militarySubtitle->subtitle.Get_Length()) {
                    m_militarySubtitle->increment_on_frame = m_militarySubtitle->lifetime + 1;
                }
            }
        } else {
            unsigned char red;
            unsigned char green;
            unsigned char blue;
            unsigned char alpha;
            Get_Color_Components(m_militarySubtitle->color, &red, &green, &blue, &alpha);
            int alpha_adjust = GameMath::Fast_To_Int_Truncate((frame - m_militarySubtitle->lifetime) * 0.1f);

            if (alpha - alpha_adjust >= 0) {
                alpha -= alpha_adjust;
                m_militarySubtitle->color = Make_Color(red, green, blue, alpha);
            } else {
                Remove_Military_Subtitle();
            }
        }
    }

    static const NameKeyType moneyWindowKey = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:MoneyDisplay");
    static const NameKeyType powerWindowKey = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PowerWindow");
    GameWindow *money_window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, moneyWindowKey);
    GameWindow *power_window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, powerWindowKey);

    Player *player = nullptr;

    if (g_theControlBar->Is_Observer()) {
        player = g_theControlBar->Get_Observer_Player();
    } else {
        player = g_thePlayerList->Get_Local_Player();
    }

    if (player != nullptr) {
        static int lastMoney = -1;
        int money = player->Get_Money()->Count_Money();

        if (lastMoney != money) {
            Utf16String money_display;
            money_display.Format(g_theGameText->Fetch("GUI:ControlBarMoneyDisplay"), money);
            Gadget_Static_Text_Set_Text(money_window, money_display);
            lastMoney = money;
        }

        money_window->Win_Hide(false);
        power_window->Win_Hide(false);
    } else {
        money_window->Win_Hide(true);
        power_window->Win_Hide(true);
    }

    Update_Floating_Text();
    g_theControlBar->Update();
    Update_Idle_Worker();

    for (auto it = m_windowLayoutList.begin(); it != m_windowLayoutList.end(); it++) {
        (*it)->Run_Update(nullptr);
    }

    if (m_cameraRotateLeft && !m_cameraRotateRight) {
        g_theTacticalView->Set_Angle(g_theTacticalView->Get_Angle() - g_theWriteableGlobalData->m_keyboardCameraRotateSpeed);
    }

    if (m_cameraRotateRight && !m_cameraRotateLeft) {
        g_theTacticalView->Set_Angle(g_theTacticalView->Get_Angle() + g_theWriteableGlobalData->m_keyboardCameraRotateSpeed);
    }

    if (m_cameraZoomIn && !m_cameraZoomOut) {
        g_theTacticalView->Zoom_In();
    }

    if (m_cameraZoomOut && !m_cameraZoomIn) {
        g_theTacticalView->Zoom_Out();
    }
}

void InGameUI::Remove_Message_At_Index(int index)
{
    m_uiMessages[index].full_text.Clear();

    if (m_uiMessages[index].display_string != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_uiMessages[index].display_string);
        m_uiMessages[index].display_string = nullptr;
    }

    m_uiMessages[index].timestamp = 0;
}

void InGameUI::Update_Floating_Text()
{
    unsigned int frame = g_theGameLogic->Get_Frame();
    static unsigned int lastLogicFrameUpdate = frame;

    if (lastLogicFrameUpdate != frame) {
        lastLogicFrameUpdate = frame;

        for (auto it = m_floatingTextList.begin(); it != m_floatingTextList.end();) {
            FloatingTextData *data = *it;
            data->m_frameCount++;

            if (frame <= (unsigned int)data->m_frameTimeOut) {
                it++;
                continue;
            }

            unsigned char red;
            unsigned char green;
            unsigned char blue;
            unsigned char alpha;
            Get_Color_Components(data->m_color, &red, &green, &blue, &alpha);
            int alpha_adjust = GameMath::Fast_To_Int_Truncate((frame - data->m_frameTimeOut) * m_floatingTextMoveVanishRate);

            if (alpha - alpha_adjust >= 0) {
                alpha -= alpha_adjust;
            } else {
                alpha = 0;
            }

            data->m_color = Make_Color(red, green, blue, alpha);

            if (alpha != 0) {
                it++;
                continue;
            } else {
                it = m_floatingTextList.erase(it);
                data->Delete_Instance();
            }
        }
    }
}

void InGameUI::Popup_Message(
    const Utf8String &message, int width_percent, int height_percent, int width, bool pause, bool pause_music)
{
    Popup_Message(message, width_percent, height_percent, width, m_popupMessageColor, pause, pause_music);
}

void InGameUI::Popup_Message(
    const Utf8String &message, int width_percent, int height_percent, int width, int color, bool pause, bool pause_music)
{
    if (m_popupMessageData != nullptr) {
        Clear_Popup_Message_Data();
    }

    Update_Diplomacy_Briefing_Text(message, false);
    Utf16String text = g_theGameText->Fetch(message);
    m_popupMessageData = new PopupMessageData();
    m_popupMessageData->m_message = text;

    if (width_percent > 100) {
        width_percent = 100;
    }

    if (width_percent < 0) {
        width_percent = 0;
    }

    if (height_percent > 100) {
        height_percent = 100;
    }

    if (height_percent < 0) {
        height_percent = 0;
    }

    m_popupMessageData->m_xPos = g_theDisplay->Get_Width() * (width_percent / 100.0f);
    m_popupMessageData->m_yPos = g_theDisplay->Get_Height() * (height_percent / 100.0f);

    if (width < 50) {
        width = 50;
    }

    m_popupMessageData->m_width = width;
    m_popupMessageData->m_color = color;
    m_popupMessageData->m_pause = pause;
    m_popupMessageData->m_pauseMusic = pause_music;
    m_popupMessageData->m_windowLayout = g_theWindowManager->Win_Create_Layout("InGamePopupMessage.wnd");
    m_popupMessageData->m_windowLayout->Run_Init(nullptr);
}

void InGameUI::Message_Color(const RGBColor *color, Utf16String message, ...)
{
    va_list va;
    va_start(va, message);
    Utf16String formatted_message;
    formatted_message.Format_VA(message, va);
    Add_Message_Text(formatted_message, color);
    va_end(va);
}

void InGameUI::Message(Utf16String message, ...)
{
    va_list va;
    va_start(va, message);
    Utf16String formatted_message;
    formatted_message.Format_VA(message, va);
    Add_Message_Text(formatted_message, nullptr);
    va_end(va);
}

void InGameUI::Message(Utf8String message, ...)
{
    va_list va;
    va_start(va, message);
    Utf16String text = g_theGameText->Fetch(message.Str());
    Utf16String formatted_message;
    formatted_message.Format_VA(text, va);
    Add_Message_Text(formatted_message, nullptr);
    va_end(va);
}

void InGameUI::Add_Message_Text(const Utf16String &formatted_message, const RGBColor *rgb_color)
{
    int color1 = m_messageColor1;
    int color2 = m_messageColor2;

    if (rgb_color != nullptr) {
        color1 = Make_Color(0, 0, 0, 255) | rgb_color->Get_As_Int();
        color2 = Make_Color(0, 0, 0, 255) | rgb_color->Get_As_Int();
    }

    m_uiMessages[MAX_UI_MESSAGES - 1].full_text.Clear();

    if (m_uiMessages[MAX_UI_MESSAGES - 1].display_string != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_uiMessages[MAX_UI_MESSAGES - 1].display_string);
        m_uiMessages[MAX_UI_MESSAGES - 1].display_string = nullptr;
    }

    m_uiMessages[5].timestamp = 0;

    for (int i = MAX_UI_MESSAGES - 1; i >= 1; i--) {
        m_uiMessages[i] = m_uiMessages[i - 1];
    }

    m_uiMessages[0].full_text = formatted_message;
    m_uiMessages[0].timestamp = g_theGameLogic->Get_Frame();
    m_uiMessages[0].display_string = g_theDisplayStringManager->New_Display_String();
    m_uiMessages[0].display_string->Set_Font(
        g_theFontLibrary->Get_Font(m_messageFont, g_theGlobalLanguage->Adjust_Font_Size(m_messagePointSize), m_messageBold));
    m_uiMessages[0].display_string->Set_Text(m_uiMessages[0].full_text);

    if (m_uiMessages[1].display_string != nullptr && m_uiMessages[1].color != color2) {
        m_uiMessages[0].color = color2;
    } else {
        m_uiMessages[0].color = color1;
    }
}

void InGameUI::Military_Subtitle(const Utf8String &subtitle, int duration)
{
    Remove_Military_Subtitle();
    Update_Diplomacy_Briefing_Text(subtitle, false);
    Utf16String text = g_theGameText->Fetch(subtitle);

    if (text.Is_Empty() || duration <= 0) {
        captainslog_dbgassert(false,
            "Trying to create a military subtitle but either title is empty (%ls) or duration is <= 0 (%d)",
            text.Str(),
            duration);
    } else {
        unsigned int frame = g_theGameLogic->Get_Frame();
        unsigned int lifetime = (duration * 30.0f / 1000.0f) + frame;
        g_theInGameUI->Disable_Tooltips_Until(lifetime);
        float width = g_theDisplay->Get_Width() / 800.0f;
        float height = g_theDisplay->Get_Height() / 600.0f;
        m_militarySubtitle = new MilitarySubtitleData();
        m_militarySubtitle->subtitle.Set(text);
        m_militarySubtitle->block_drawn = true;
        m_militarySubtitle->block_begin_frame = frame;
        m_militarySubtitle->lifetime = lifetime;
        m_militarySubtitle->position.x = m_militaryCaptionPosition.x * width;
        m_militarySubtitle->block_pos.x = m_militarySubtitle->position.x;
        m_militarySubtitle->position.y = m_militaryCaptionPosition.y * height;
        m_militarySubtitle->block_pos.y = m_militarySubtitle->position.y;
        m_militarySubtitle->increment_on_frame =
            (g_theGlobalLanguage->Get_Military_Caption_Delay_MS() * 30.0f / 1000.0f) + frame;
        m_militarySubtitle->index = 0;

        for (int i = 1; i < 4; i++) {
            m_militarySubtitle->display_strings[i] = nullptr;
        }

        m_militarySubtitle->current_display_string = 0;
        m_militarySubtitle->display_strings[0] = g_theDisplayStringManager->New_Display_String();
        m_militarySubtitle->display_strings[0]->Reset();
        m_militarySubtitle->display_strings[0]->Set_Font(g_theFontLibrary->Get_Font(m_militaryCaptionTitleFont,
            g_theGlobalLanguage->Adjust_Font_Size(m_militaryCaptionTitlePointSize),
            m_militaryCaptionTitleBold));
        m_militarySubtitle->color = Make_Color(m_militaryCaptionColor.red,
            m_militaryCaptionColor.green,
            m_militaryCaptionColor.blue,
            m_militaryCaptionColor.alpha);
    }
}

void InGameUI::Remove_Military_Subtitle()
{
    if (m_militarySubtitle != nullptr) {
        g_theInGameUI->Clear_Tooltips_Disabled();

        for (unsigned int i = 0; i <= m_militarySubtitle->current_display_string; i++) {
            g_theDisplayStringManager->Free_Display_String(m_militarySubtitle->display_strings[i]);
            m_militarySubtitle->display_strings[i] = nullptr;
        }

        delete m_militarySubtitle;
        m_militarySubtitle = nullptr;
    }
}

void InGameUI::Display_Cant_Build_Message(LegalBuildCode code)
{
    switch (code) {
        case LBC_RESTRICTED_TERRAIN:
            g_theInGameUI->Message("GUI:CantBuildRestrictedTerrain");
            break;
        case LBC_NOT_FLAT_ENOUGH:
            g_theInGameUI->Message("GUI:CantBuildNotFlatEnough");
            break;
        case LBC_OBJECTS_IN_THE_WAY:
            g_theInGameUI->Message("GUI:CantBuildObjectsInTheWay");
            break;
        case LBC_NO_CLEAR_PATH:
            g_theInGameUI->Message("GUI:CantBuildNoClearPath");
            break;
        case LBC_SHROUD:
            g_theInGameUI->Message("GUI:CantBuildShroud");
            break;
        case LBC_TOO_CLOSE_TO_SUPPLIES:
            g_theInGameUI->Message("GUI:CantBuildTooCloseToSupplies");
            break;
        default:
            g_theInGameUI->Message("GUI:CantBuildThere");
            break;
    }
}

void InGameUI::Begin_Area_Select_Hint(const GameMessage *msg)
{
    m_isDragSelecting = true;
    m_dragSelectRegion = msg->Get_Argument(0)->region;
}

void InGameUI::End_Area_Select_Hint(const GameMessage *msg)
{
    m_isDragSelecting = false;
}

void InGameUI::Create_Move_Hint(const GameMessage *msg)
{
    for (int i = 0; i < MAX_MOVE_HINTS; i++) {
        if (m_moveHint[i].source_id == msg->Get_Argument(0)->integer) {
            if (m_moveHint[i].frame != 0) {
                Expire_Hint(MOVE_HINT, i);
            }
        }
    }

    Drawable *drawable;
    Object *object;

    if (Get_Select_Count() != 1
        || ((drawable = Get_First_Selected_Drawable()) == nullptr ? (object = nullptr) : (object = drawable->Get_Object()),
            object == nullptr || !object->Is_KindOf(KINDOF_IMMOBILE))) {
        m_moveHint[m_nextMoveHint].frame = g_theGameLogic->Get_Frame();
        m_moveHint[m_nextMoveHint].pos = msg->Get_Argument(0)->position;
    }

    if (++m_nextMoveHint == MAX_MOVE_HINTS) {
        m_nextMoveHint = 0;
    }
}

void InGameUI::Expire_Hint(HintType type, unsigned int hint_index)
{
    if (type != MOVE_HINT) {
        captainslog_dbgassert(false, "undefined hint type");
    } else if (hint_index < MAX_MOVE_HINTS) {
        m_moveHint[hint_index].source_id = 0;
        m_moveHint[hint_index].frame = 0;
    }
}

void InGameUI::Create_Attack_Hint(const GameMessage *msg) {}

void InGameUI::Create_Force_Attack_Hint(const GameMessage *msg) {}

void InGameUI::Create_Mouseover_Hint(const GameMessage *msg)
{
    if (!m_isScrolling && !m_isSelecting) {
        GameWindow *parent = nullptr;
        MouseIO *status = g_theMouse->Get_Mouse_Status();
        bool is_blocking_window_under_cursor = false;

        if (status != nullptr && g_theWindowManager != nullptr) {
            parent = g_theWindowManager->Get_Window_Under_Cursor(status->pos.x, status->pos.y, false);
        }

        while (parent != nullptr) {
            if (parent->Win_Get_Input_Func() == Left_HUD_Input) {
                is_blocking_window_under_cursor = false;
                break;
            }

            if ((parent->Win_Get_Status() & WIN_STATUS_SEE_THRU) == 0) {
                is_blocking_window_under_cursor = true;
                break;
            }

            parent = parent->Win_Get_Parent();
        }

        if (is_blocking_window_under_cursor) {
            Set_Mouse_Cursor(CURSOR_ARROW);
        } else {
            DrawableID old_moused_over_id = m_mousedOverDrawableID;

            if (msg->Get_Type() == GameMessage::MSG_MOUSEOVER_DRAWABLE_HINT) {
                g_theMouse->Set_Cursor_Tooltip(Utf16String::s_emptyString, -1, nullptr, 1.0f);
                m_mousedOverDrawableID = DrawableID::INVALID_DRAWABLE_ID;
                Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(msg->Get_Argument(0)->drawableID);
                Object *object;

                if (drawable != nullptr) {
                    object = drawable->Get_Object();
                } else {
                    object = nullptr;
                }

                if (object != nullptr) {
                    if (object->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
                        static const NameKeyType key_MobMemberSlavedUpdate =
                            g_theNameKeyGenerator->Name_To_Key("MobMemberSlavedUpdate");
                        MobMemberSlavedUpdate *update =
                            static_cast<MobMemberSlavedUpdate *>(object->Find_Update_Module(key_MobMemberSlavedUpdate));

                        if (update != nullptr) {
                            Object *slaver = g_theGameLogic->Find_Object_By_ID(update->Get_Slaver_ID());

                            if (slaver != nullptr) {
                                Drawable *slaver_draw = slaver->Get_Drawable();

                                if (slaver_draw != nullptr) {
                                    m_mousedOverDrawableID = slaver_draw->Get_ID();
                                }
                            }
                        }
                    } else {
                        m_mousedOverDrawableID = drawable->Get_ID();
                    }

                    if (g_theWriteableGlobalData->m_constantDebugUpdate) {
                        m_mousedOverDrawableID = drawable->Get_ID();
                    }

                    const Player *player = nullptr;
                    const ThingTemplate *tmplate = object->Get_Template();
                    ContainModuleInterface *contain = object->Get_Contain();

                    if (contain != nullptr) {
                        player = contain->Get_Apparent_Controlling_Player(g_thePlayerList->Get_Local_Player());
                    }

                    if (player == nullptr) {
                        player = object->Get_Controlling_Player();
                    }

                    bool is_disguised = false;

                    if (object->Is_KindOf(KINDOF_DISGUISER)) {
                        StealthUpdate *stealth = object->Get_Stealth_Update();

                        if (stealth != nullptr) {
                            if (stealth->Has_Disguised_Template()) {
                                Player *local_player = g_thePlayerList->Get_Local_Player();
                                Player *nth_player = g_thePlayerList->Get_Nth_Player(stealth->Get_Player_Index());

                                if (player->Get_Relationship(local_player->Get_Default_Team()) != ALLIES
                                    && local_player->Is_Player_Active()) {
                                    player = nth_player;
                                    ThingTemplate *disguised = stealth->Get_Disguised_Template();

                                    if (disguised != nullptr) {
                                        tmplate = disguised;
                                        is_disguised = true;
                                    }
                                }
                            }
                        }
                    }

                    Utf16String display_name(tmplate->Get_Display_Name());
                    Utf16String display_name_2(tmplate->Get_Display_Name());

                    if (display_name.Is_Empty()) {
                        Utf8String template_string;
                        template_string.Format("ThingTemplate:%s", tmplate->Get_Name().Str());
                        display_name = g_theGameText->Fetch(template_string);
                    }

#ifdef GAME_DEBUG_STRUCTS
                    if (g_theWriteableGlobalData->m_debugAI) {
                        Team *team = object->Get_Team();
                        Utf8String object_name = object->Get_Name();
                        Utf8String team_name;
                        Utf8String state_name;
                        AIUpdateInterface *ai_update = object->Get_AI_Update_Interface();

                        if (ai_update != nullptr) {
                            if (ai_update->Get_Path() != nullptr) {
                                g_theAI->Get_Pathfinder()->Set_Debug_Path(ai_update->Get_Path());
                            }

                            state_name = ai_update->Get_Current_State_Name();

                            if (ai_update->Get_Attack_Priority_Info() != nullptr) {
                                state_name.Concat(" AttackPriority=");
                                state_name += ai_update->Get_Attack_Priority_Info()->Get_Name();
                            }
                        }

                        if (team != nullptr) {
                            team_name = team->Get_Name();
                        }

                        if (object_name.Is_Empty()) {
                            if (!team_name.Is_Empty()) {
                                display_name.Format(U_CHAR("%hs: %s"), team_name.Str(), display_name.Str());
                            }
                        } else if (team_name.Is_Empty()) {
                            display_name.Format(U_CHAR("%hs: %s"), object_name.Str(), display_name.Str());
                        } else {
                            display_name.Format(
                                U_CHAR("%hs(%hs): %s"), team_name.Str(), object_name.Str(), display_name.Str());
                        }

                        display_name.Format(U_CHAR("%s - %hs"), display_name.Str(), state_name.Str());
                    }
#endif

                    Utf16String tooltip;
                    static const NameKeyType warehouseModuleKey =
                        g_theNameKeyGenerator->Name_To_Key("SupplyWarehouseDockUpdate");
                    SupplyWarehouseDockUpdate *warehouse =
                        static_cast<SupplyWarehouseDockUpdate *>(object->Find_Update_Module(warehouseModuleKey));

                    if (warehouse != nullptr) {
                        tooltip.Format(g_theGameText->Fetch("TOOLTIP:SupplyWarehouse"),
                            g_theWriteableGlobalData->m_valuesPerSupplyBox * warehouse->Get_Boxes_Stored());
                        display_name.Concat(tooltip);
                    }

                    if (player != nullptr) {
                        Utf16String player_display;

                        if (g_theRecorder->Is_Multiplayer() && player->Is_Playable_Side()) {
                            player_display.Format(
                                U_CHAR("%s\n%s"), display_name.Str(), player->Get_Player_Display_Name().Str());
                        } else {
                            player_display = display_name;
                        }

                        int index;

                        if (g_thePlayerList != nullptr) {
                            index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
                        } else {
                            index = 0;
                        }

                        int x;
                        int y;
                        g_thePartitionManager->World_To_Cell(object->Get_Position()->x, object->Get_Position()->y, &x, &y);

                        if (g_thePartitionManager->Get_Shroud_Status_For_Player(index, x, y) == SHROUD_STATUS_CLEAR) {
                            RGBColor color;

                            if (is_disguised) {
                                color.Set_From_Int(player->Get_Color());
                            } else {
                                color.Set_From_Int(drawable->Get_Object()->Get_Indicator_Color());

                                const Object *obj = drawable->Get_Object();

                                if (obj != nullptr) {
                                    ContainModuleInterface *contain2 = obj->Get_Contain();

                                    if (contain2 != nullptr) {
                                        if (contain2->Is_Garrisonable()) {
                                            const Player *controlling_player = contain2->Get_Apparent_Controlling_Player(
                                                g_thePlayerList->Get_Local_Player());

                                            if (controlling_player != nullptr) {
                                                color.Set_From_Int(controlling_player->Get_Color());
                                            }
                                        }
                                    }
                                }
                            }

                            if (display_name_2.Compare(g_theGameText->Fetch("OBJECT:Prop")) != 0) {
                                g_theMouse->Set_Cursor_Tooltip(player_display, -1, &color, 1.0f);
                            }
                        }
                    }
                }
            } else {
                m_mousedOverDrawableID = INVALID_DRAWABLE_ID;
            }

            if (old_moused_over_id != m_mousedOverDrawableID) {
                g_theMouse->Reset_Tooltip_Delay();
            }

            if (m_mouseMode != MOUSEMODE_DEFAULT || m_isScrolling || m_isSelecting || g_theInGameUI->Get_Select_Count() != 0
                || (g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK
                    && g_theLookAtTranslator->Has_Mouse_Moved_Recently())) {
                if (m_mouseMode >= MOUSEMODE_GUI_COMMAND) {
                    Set_Mouse_Cursor(m_mouseCursor);
                }
            } else {
                if (m_mousedOverDrawableID == INVALID_DRAWABLE_ID) {
                    Set_Mouse_Cursor(CURSOR_ARROW);
                    return;
                }

                Drawable *moused_over_drawable = g_theGameClient->Find_Drawable_By_ID(m_mousedOverDrawableID);
                Object *moused_over_object = moused_over_drawable ? moused_over_drawable->Get_Object() : nullptr;
                bool can_select = Can_Select_Drawable(moused_over_drawable, false);

                if (moused_over_object == nullptr) {
                    can_select = false;
                }

                if (can_select && moused_over_object->Is_Locally_Controlled()) {
                    Set_Mouse_Cursor(CURSOR_SELECT);
                } else {
                    Set_Mouse_Cursor(CURSOR_ARROW);
                }
            }
        }
    }
}

void InGameUI::Create_Garrison_Hint(const GameMessage *msg)
{
    Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(msg->Get_Argument(0)->drawableID);

    if (drawable != nullptr) {
        drawable->On_Selected();
    }
}

void InGameUI::Set_Scrolling(bool is_scrolling)
{
    if (m_isScrolling != is_scrolling) {
        if (is_scrolling) {
            g_theMouse->Capture();
            Set_Mouse_Cursor(CURSOR_SCROLL);
            g_theTacticalView->Set_Camera_Lock(INVALID_OBJECT_ID);
            g_theTacticalView->Set_Camera_Lock_Drawable(nullptr);
        } else {
            Set_Mouse_Cursor(CURSOR_ARROW);
            g_theMouse->Release_Capture();
        }

        m_isScrolling = is_scrolling;
    }
}

bool InGameUI::Is_Scrolling()
{
    return m_isScrolling;
}

void InGameUI::Set_Selecting(bool is_selecting)
{
    if (m_isSelecting != is_selecting) {
        m_isSelecting = is_selecting;
    }
}

bool InGameUI::Is_Selecting()
{
    return m_isSelecting;
}

void InGameUI::Set_Scroll_Amount(Coord2D amt)
{
    m_scrollAmt = amt;
}

Coord2D InGameUI::Get_Scroll_Amount()
{
    return m_scrollAmt;
}

const CommandButton *InGameUI::Get_GUI_Command()
{
    return m_pendingGUICommand;
}

const ThingTemplate *InGameUI::Get_Pending_Place_Type()
{
    return m_pendingPlaceType;
}

ObjectID InGameUI::Get_Pending_Place_Source_Object_ID()
{
    return m_pendingPlaceSourceObjectID;
}

#ifndef GAME_DEBUG_STRUCTS
bool InGameUI::Get_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click()
{
    return m_preventLeftClickDeselectionInAlternateMouseModeForOneClick;
}

void InGameUI::Set_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click(bool prevent)
{
    m_preventLeftClickDeselectionInAlternateMouseModeForOneClick = prevent;
}
#endif

void InGameUI::Set_Placement_Start(const ICoord2D *start)
{
    if (start != nullptr) {
        m_placeAnchorStart.x = start->x;
        m_placeAnchorStart.y = start->y;
        m_placeAnchorEnd.x = start->x;
        m_placeAnchorEnd.y = start->y;
        m_placeAnchorInProgress = true;
    } else {
        m_placeAnchorInProgress = false;
    }
}

void InGameUI::Set_Placement_End(const ICoord2D *end)
{
    if (end != nullptr) {
        m_placeAnchorEnd.x = end->x;
        m_placeAnchorEnd.y = end->y;
    }
}

bool InGameUI::Is_Placement_Anchored()
{
    return m_placeAnchorInProgress;
}

void InGameUI::Get_Placement_Points(ICoord2D *start, ICoord2D *end)
{
    if (start != nullptr) {
        *start = m_placeAnchorStart;
    }

    if (end != nullptr) {
        *end = m_placeAnchorEnd;
    }
}

float InGameUI::Get_Placement_Angle()
{
    if (m_placeIcon[0] != nullptr) {
        return m_placeIcon[0]->Get_Orientation();
    } else {
        return 0.0f;
    }
}

void InGameUI::Select_Drawable(Drawable *drawable)
{
    if (!drawable->Is_Selected()) {
        m_frameSelectionChanged = g_theGameLogic->Get_Frame();
        drawable->Friend_Set_Selected();
        m_selectedDrawables.push_front(drawable);
        Increment_Select_Count();
        Evaluate_Solo_Nexus(drawable);
        g_theControlBar->On_Drawable_Selected(drawable);
    }
}

void InGameUI::Evaluate_Solo_Nexus(Drawable *drawable)
{
    m_soloNexusSelectedDrawableID = INVALID_DRAWABLE_ID;
    Object *object;

    if (drawable == nullptr || (object = drawable->Get_Object()) == nullptr || object->Is_KindOf(KINDOF_MOB_NEXUS)
        || object->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
        short count = 0;

        for (auto it = m_selectedDrawables.begin(); it != m_selectedDrawables.end(); it++) {
            Drawable *draw = *it;
            Object *obj = draw->Get_Object();

            if (obj != nullptr) {
                if (obj->Is_KindOf(KINDOF_MOB_NEXUS)) {
                    if (++count != 1) {
                        m_soloNexusSelectedDrawableID = INVALID_DRAWABLE_ID;
                        return;
                    }

                    m_soloNexusSelectedDrawableID = draw->Get_ID();
                } else if (obj->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
                    m_soloNexusSelectedDrawableID = INVALID_DRAWABLE_ID;
                    return;
                }
            }
        }
    }
}

void InGameUI::Deselect_Drawable(Drawable *drawable)
{
    if (drawable->Is_Selected()) {
        m_frameSelectionChanged = g_theGameLogic->Get_Frame();
        drawable->Friend_Clear_Selected();
        auto it = std::find(m_selectedDrawables.begin(), m_selectedDrawables.end(), drawable);

        captainslog_dbgassert(it != m_selectedDrawables.end(),
            "Deselect_Drawable: Drawable not found in the selected drawable list '%s'\n",
            drawable->Get_Template()->Get_Name().Str());
        m_selectedDrawables.erase(it);
        Decrement_Select_Count();
        Evaluate_Solo_Nexus(nullptr);
        g_theControlBar->On_Drawable_Deselected(drawable);
    }
}

void InGameUI::Deselect_All_Drawables(bool post_msg)
{
    const std::list<Drawable *> *drawables = Get_All_Selected_Drawables();

    for (auto it = drawables->begin(); it != drawables->end();) {
        Deselect_Drawable(*(it++));
    }

    m_selectedDrawables.clear();
    m_soloNexusSelectedDrawableID = INVALID_DRAWABLE_ID;

    if (post_msg) {
        GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_DESTROY_SELECTED_GROUP);
        msg->Append_Bool_Arg(true);
    }
}

const std::list<Drawable *> *InGameUI::Get_All_Selected_Drawables()
{
    return &m_selectedDrawables;
}

const std::list<Drawable *> *InGameUI::Get_All_Selected_Local_Drawables()
{
    m_selectedLocalDrawables.clear();

    for (auto it = m_selectedDrawables.begin(); it != m_selectedDrawables.end(); it++) {
        Drawable *drawable = *it;

        if (drawable != nullptr && drawable->Get_Object() != nullptr) {
            if (drawable->Get_Object()->Is_Locally_Controlled()) {
                m_selectedLocalDrawables.push_back(drawable);
            }
        }
    }

    return &m_selectedLocalDrawables;
}

Drawable *InGameUI::Get_First_Selected_Drawable()
{
    if (m_selectedDrawables.empty()) {
        return nullptr;
    } else {
        return m_selectedDrawables.front();
    }
}

bool InGameUI::Is_Drawable_Selected(DrawableID id_to_check)
{
    for (auto it = m_selectedDrawables.begin(); it != m_selectedDrawables.end(); it++) {
        if ((*it)->Get_ID() == id_to_check) {
            return true;
        }
    }

    return false;
}

bool InGameUI::Is_Any_Selected_KindOf(KindOfType kindof)
{
    for (auto it = m_selectedDrawables.begin(); it != m_selectedDrawables.end(); it++) {
        Drawable *drawable = *it;

        if (drawable != nullptr && drawable->Is_KindOf(kindof)) {
            return true;
        }
    }

    return false;
}

bool InGameUI::Is_All_Selected_KindOf(KindOfType kindof)
{
    for (auto it = m_selectedDrawables.begin(); it != m_selectedDrawables.end(); it++) {
        Drawable *drawable = *it;

        if (drawable != nullptr && !drawable->Is_KindOf(kindof)) {
            return false;
        }
    }

    return true;
}

WorldAnimationData::WorldAnimationData() : m_anim(nullptr), m_frame(0), m_options(WORLD_ANIMATION_UNK2), m_zRise(0.0f)
{
    m_pos.Zero();
}

void InGameUI::Set_Input_Enabled(bool enable)
{
    if (!enable) {
        Set_Selecting(false);
    }

    bool old_enabled = m_inputEnabled;
    m_inputEnabled = enable;

    if (old_enabled && !enable) {
        Set_Force_To_Attack_Mode(false);
        Set_Force_To_Move_Mode(false);
        Set_Waypoint_Mode(false);
        Set_Prefer_Selection(false);
        Set_Camera_Rotating_Left(false);
        Set_Camera_Rotating_Right(false);
        Set_Camera_Zooming_In(false);
        Set_Camera_Zooming_Out(false);
    }
}

void InGameUI::Disregard_Drawable(Drawable *draw)
{
    Deselect_Drawable(draw);
}

void InGameUI::Pre_Draw()
{
    Handle_Build_Placements();
    Handle_Radius_Cursor();
    Draw_Floating_Text();
    Update_And_Draw_World_Animations();
}

RGBColor illegalBuildColor{ 1.0f, 0.0f, 0.0f };

void InGameUI::Handle_Build_Placements()
{
    if (m_pendingPlaceType != nullptr) {
        float orientation = m_placeIcon[0]->Get_Orientation();
        ICoord2D pos;

        if (Is_Placement_Anchored()) {
            ICoord2D start;
            ICoord2D end;
            Get_Placement_Points(&start, &end);
            pos = start;

            if (start.x != end.x || start.y != end.y) {
                Coord3D start_terrain;
                Coord3D end_terrain;
                g_theTacticalView->Screen_To_Terrain(&start, &start_terrain);
                g_theTacticalView->Screen_To_Terrain(&end, &end_terrain);
                Coord2D terrain;
                terrain.x = end_terrain.x - start_terrain.x;
                terrain.y = end_terrain.y - start_terrain.y;
                orientation = terrain.To_Angle();
            }
        } else {
            pos = g_theMouse->Get_Mouse_Status()->pos;
        }

        Coord3D pos_terrain;
        g_theTacticalView->Screen_To_Terrain(&pos, &pos_terrain);
        m_placeIcon[0]->Set_Position(&pos_terrain);
        m_placeIcon[0]->Set_Orientation(orientation);

        if ((g_theGameClient->Get_Frame() & 1) != 0) {
            g_theTerrainVisual->Remove_All_Bibs();
            Object *obj = g_theGameLogic->Find_Object_By_ID(Get_Pending_Place_Source_Object_ID());
            LegalBuildCode legal = g_theBuildAssistant->Is_Location_Legal_To_Build(
                &pos_terrain, m_pendingPlaceType, orientation, 95, obj, nullptr);

            if (legal != LBC_OK) {
                m_placeIcon[0]->Color_Tint(&illegalBuildColor);
            } else {
                m_placeIcon[0]->Color_Tint(nullptr);
            }

            if (legal != LBC_OK) {
                g_theTerrainVisual->Add_Faction_Bib_Drawable(m_placeIcon[0], legal != LBC_OK, 0.0f);
            } else {
                g_theTerrainVisual->Remove_Faction_Bib_Drawable(m_placeIcon[0]);
            }
        }

        if (Is_Placement_Anchored() && g_theBuildAssistant->Is_Line_Build_Template(m_pendingPlaceType)) {
            ICoord2D start;
            ICoord2D end;
            Get_Placement_Points(&start, &end);
            Coord3D start_terrain;
            Coord3D end_terrain;
            g_theTacticalView->Screen_To_Terrain(&start, &start_terrain);
            g_theTacticalView->Screen_To_Terrain(&end, &end_terrain);
            float major_radius = m_pendingPlaceType->Get_Template_Geometry_Info().Get_Major_Radius();
            float major_diameter = major_radius + major_radius;
            BuildAssistant::TileBuildInfo *info = g_theBuildAssistant->Build_Tiled_Locations(m_pendingPlaceType,
                orientation,
                &start_terrain,
                &end_terrain,
                major_diameter,
                g_theWriteableGlobalData->m_maxLineBuildObjects,
                g_theGameLogic->Find_Object_By_ID(g_theInGameUI->Get_Pending_Place_Source_Object_ID()));

            for (int i = 0; i < info->tiles_used; i++) {
                if (m_placeIcon[i] == nullptr) {
                    m_placeIcon[i] = g_theThingFactory->New_Drawable(m_pendingPlaceType, DRAWABLE_STATUS_8);
                }
            }

            for (int i = info->tiles_used; i < g_theWriteableGlobalData->m_maxLineBuildObjects; i++) {
                if (m_placeIcon[i] != nullptr) {
                    g_theGameClient->Destroy_Drawable(m_placeIcon[i]);
                    m_placeIcon[i] = nullptr;
                }
            }

            for (int i = 0; i < info->tiles_used; i++) {
                m_placeIcon[i]->Set_Position(&info->positions[i]);
                m_placeIcon[i]->Set_Opacity(0.45f);
                m_placeIcon[i]->Set_Orientation(orientation);
            }
        }
    }
}

void InGameUI::Draw_Floating_Text()
{
    for (auto it = m_floatingTextList.begin(); it != m_floatingTextList.end(); it++) {
        FloatingTextData *data = *it;
        int index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        int x;
        int y;
        g_thePartitionManager->World_To_Cell(data->m_pos3D.x, data->m_pos3D.y, &x, &y);
        ICoord2D screen;

        if (g_theTacticalView->World_To_Screen_Tri(&data->m_pos3D, &screen) && data->m_dString != nullptr
            && g_thePartitionManager->Get_Shroud_Status_For_Player(index, x, y) == SHROUD_STATUS_CLEAR) {
            screen.y -= data->m_frameCount * m_floatingTextMoveUpSpeed;

            unsigned char red;
            unsigned char green;
            unsigned char blue;
            unsigned char alpha;
            Get_Color_Components(data->m_color, &red, &green, &blue, &alpha);
            int color = Make_Color(0, 0, 0, alpha);

            int width;
            data->m_dString->Get_Size(&width, nullptr);
            data->m_dString->Draw(screen.x - width / 2, screen.y, data->m_color, color);
        }
    }
}

void InGameUI::Update_And_Draw_World_Animations()
{
    for (auto it = m_worldAnimations.begin(); it != m_worldAnimations.end(); it++) {
        WorldAnimationData *data = *it;

        if (data == nullptr || g_theGameLogic->Is_Game_Paused()) {
            goto label1;
        }

        if (g_theGameLogic->Get_Frame() >= (unsigned int)data->m_frame
            || ((data->m_options & 2) == 0 && (data->m_anim->Get_Status() & Anim2D::STATUS_ANIM_COMPLETE) != 0)) {
            data->m_anim->Delete_Instance();
            delete data;
            it = m_worldAnimations.erase(it);
        } else {
            if (data->m_zRise != 0.0f) {
                data->m_pos.z = data->m_zRise / 30.0f + data->m_pos.z;
            }
        label1:
            int index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();

            if (g_thePartitionManager->Get_Shroud_Status_For_Player(index, &data->m_pos) == SHROUD_STATUS_CLEAR) {
                if ((data->m_options & 1) != 0) {
                    unsigned int new_frame = data->m_frame - g_theGameLogic->Get_Frame();

                    if (new_frame < 30) {
                        data->m_anim->Set_Alpha(new_frame / 30.0f);
                    }
                }

                ICoord2D screen;

                if (g_theTacticalView->World_To_Screen_Tri(&data->m_pos, &screen)) {
                    unsigned int width = data->m_anim->Get_Current_Frame_Width();
                    unsigned int height = data->m_anim->Get_Current_Frame_Height();
                    float zoom = g_theTacticalView->Get_Max_Zoom() / g_theTacticalView->Get_Zoom();
                    width *= zoom;
                    height *= zoom;
                    screen.x -= width >> 1;
                    screen.y -= height >> 1;
                    data->m_anim->Draw(screen.x, screen.y, width, height);
                }
            }
        }
    }
}

void InGameUI::Play_Movie(const Utf8String &movie_name)
{
    Stop_Movie();
    m_videoStream = g_theVideoPlayer->Open(movie_name);

    if (m_videoStream != nullptr) {
        m_currentlyPlayingMovie = movie_name;
        m_videoBuffer = g_theDisplay->Create_VideoBuffer();

        if (m_videoBuffer == nullptr || !m_videoBuffer->Allocate(m_videoStream->Width(), m_videoStream->Height())) {
            Stop_Movie();
        }
    }
}

void InGameUI::Stop_Movie()
{
    if (m_videoBuffer != nullptr) {
        delete m_videoBuffer;
        m_videoBuffer = nullptr;
    }

    if (m_videoStream != nullptr) {
        delete m_videoStream;
        m_videoStream = nullptr;
    }

    if (!m_currentlyPlayingMovie.Is_Empty()) {
        m_currentlyPlayingMovie = Utf8String::s_emptyString;
    }
}

VideoBuffer *InGameUI::Video_Buffer()
{
    return m_videoBuffer;
}

void InGameUI::Play_Cameo_Movie(const Utf8String &movie_name)
{
    Stop_Cameo_Movie();
    m_cameoVideoStream = g_theVideoPlayer->Open(movie_name);

    if (m_cameoVideoStream != nullptr) {
        m_currentlyPlayingMovie = movie_name;
        m_cameoVideoBuffer = g_theDisplay->Create_VideoBuffer();

        if (m_cameoVideoBuffer != nullptr
            && m_cameoVideoBuffer->Allocate(m_cameoVideoStream->Width(), m_cameoVideoStream->Height())) {
            GameWindow *hud = g_theWindowManager->Win_Get_Window_From_Id(
                nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:RightHUD"));
            hud->Win_Get_Instance_Data()->Set_VideoBuffer(m_cameoVideoBuffer);
        } else {
            Stop_Cameo_Movie();
        }
    }
}

void InGameUI::Stop_Cameo_Movie()
{
    GameWindow *hud =
        g_theWindowManager->Win_Get_Window_From_Id(nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:RightHUD"));
    hud->Win_Get_Instance_Data()->Set_VideoBuffer(nullptr);

    if (m_cameoVideoBuffer != nullptr) {
        delete m_cameoVideoBuffer;
        m_cameoVideoBuffer = nullptr;
    }

    if (m_cameoVideoStream != nullptr) {
        delete m_cameoVideoStream;
        m_cameoVideoStream = nullptr;
    }
}

VideoBuffer *InGameUI::Cameo_Video_Buffer()
{
    return m_cameoVideoBuffer;
}

DrawableID InGameUI::Get_Moused_Over_Drawable_ID()
{
    return m_mousedOverDrawableID;
}

int InGameUI::Select_Units_Matching_Current_Selection()
{
    int ret = Select_Matching_Across_Screen();

    if (ret == -1) {
        return -1;
    }

    if (ret != 0) {
        return ret;
    }

    return Select_Matching_Across_Map();
}

int InGameUI::Select_Matching_Across_Screen()
{
    ICoord2D origin;
    g_theTacticalView->Get_Origin(&origin.x, &origin.y);

    ICoord2D size;
    size.x = g_theTacticalView->Get_Width();
    size.y = g_theTacticalView->Get_Height();

    IRegion2D region;
    Build_Region(&origin, &size, &region);
    int ret = Select_Matching_Across_Region(&region);

    if (ret == -1) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:NothingSelected"));
    } else if (ret != 0) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossScreen"));
    }

    return ret;
}

int InGameUI::Select_Matching_Across_Map()
{
    int ret = Select_Matching_Across_Region(nullptr);

    if (ret == -1) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:NothingSelected"));
    } else if (ret != 0) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossMap"));
    } else {
        Drawable *draw = g_theInGameUI->Get_First_Selected_Drawable();

        if (draw == nullptr || draw->Get_Object() == nullptr || !draw->Get_Object()->Is_KindOf(KINDOF_STRUCTURE)) {
            g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossMap"));
        }
    }

    return ret;
}

struct SimilarUnitSelectionData
{
    const ThingTemplate *tmplate;
    std::list<Drawable *> selected_units;
    bool is_carbomb;
};

bool Similar_Unit_Selection(Drawable *draw, void *user_data)
{
    SimilarUnitSelectionData *data = static_cast<SimilarUnitSelectionData *>(user_data);
    const ThingTemplate *tmplate = data->tmplate;

    if (draw != nullptr) {
        Object *obj = draw->Get_Object();

        if (obj == nullptr) {
            return false;
        }

        bool is_equivalent = obj->Get_Template()->Is_Equivalent_To(tmplate);

        if (data->is_carbomb && !is_equivalent) {
            is_equivalent = obj->Get_Status(OBJECT_STATUS_IS_CARBOMB);
        }

        if (is_equivalent && obj->Is_Locally_Controlled() && !obj->Is_Contained()) {
            if (!obj->Get_Drawable()->Is_Selected() && obj->Is_Mass_Selectable() && !obj->Is_Outside_Map()) {
                if (g_theInGameUI->Get_Max_Select_Count() <= 0
                    || g_theInGameUI->Get_Select_Count() < g_theInGameUI->Get_Max_Select_Count()) {
                    g_theInGameUI->Select_Drawable(draw);
                    g_theInGameUI->Set_Displayed_Max_Warning(false);
                    data->selected_units.push_back(draw);
                    return true;
                }

                if (!g_theInGameUI->Get_Displayed_Max_Warning()) {
                    g_theInGameUI->Set_Displayed_Max_Warning(true);
                    Utf16String message;
                    message.Format(g_theGameText->Fetch("GUI:MaxSelectionSize"), g_theInGameUI->Get_Max_Select_Count());
                    g_theInGameUI->Message(message);
                }
            }
        }
    }

    return false;
}

int InGameUI::Select_Matching_Across_Region(IRegion2D *region)
{
    const std::list<Drawable *> *list = Get_All_Selected_Drawables();
    std::set<const ThingTemplate *> templates;

    bool is_carbomb = false;

    for (auto it = list->begin(); it != list->end(); it++) {
        Drawable *draw = *it;

        if (draw != nullptr) {
            if (draw->Get_Object() != nullptr) {
                if (draw->Get_Object()->Is_Locally_Controlled()) {
                    templates.insert(draw->Get_Object()->Get_Template());
                }

                if (draw->Get_Object()->Get_Status(OBJECT_STATUS_IS_CARBOMB)) {
                    is_carbomb = true;
                }
            }
        }
    }

    if (templates.size() != 0) {
        SimilarUnitSelectionData data;
        int count = 0;

        for (auto it = templates.begin(); it != templates.end(); it++) {
            data.tmplate = *it;
            data.is_carbomb = is_carbomb;

            if (region != nullptr) {
                count += g_theTacticalView->Iterate_Drawables_In_Region(region, Similar_Unit_Selection, &data);
            } else {
                for (Drawable *drawable = g_theGameClient->First_Drawable(); drawable != nullptr;
                     drawable = drawable->Get_Next()) {
                    count += Similar_Unit_Selection(drawable, &data);
                }
            }

            Set_Displayed_Max_Warning(false);
        }

        if (count > 0) {
            GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND);
            msg->Append_Bool_Arg(false);

            for (auto it = data.selected_units.begin(); it != data.selected_units.end(); it++) {
                Drawable *drawable = *it;

                if (drawable != nullptr && drawable->Get_Object() != nullptr) {
                    msg->Append_ObjectID_Arg(drawable->Get_Object()->Get_ID());
                }
            }
        }

        return count;
    } else {
        return -1;
    }
}

int InGameUI::Select_All_Units_By_Type(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    int ret = Select_All_Units_By_Type_Across_Screen(must_be_set, must_be_clear);

    if (ret == -1) {
        return ret;
    }

    if (ret != 0) {
        return ret;
    }

    return Select_All_Units_By_Type_Across_Map(must_be_set, must_be_clear);
}

int InGameUI::Select_All_Units_By_Type_Across_Screen(
    BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    ICoord2D origin;
    g_theTacticalView->Get_Origin(&origin.x, &origin.y);

    ICoord2D size;
    size.x = g_theTacticalView->Get_Width();
    size.y = g_theTacticalView->Get_Height();

    IRegion2D region;
    Build_Region(&origin, &size, &region);
    int ret = Select_All_Units_By_Type_Across_Region(&region, must_be_set, must_be_clear);

    if (ret == -1) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:NothingSelected"));
    } else if (ret != 0) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossScreen"));
    }

    return ret;
}

int InGameUI::Select_All_Units_By_Type_Across_Map(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    int ret = Select_All_Units_By_Type_Across_Region(nullptr, must_be_set, must_be_clear);

    if (ret == -1) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:NothingSelected"));
    } else if (ret != 0) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossMap"));
    } else {
        Drawable *draw = g_theInGameUI->Get_First_Selected_Drawable();

        if (draw == nullptr || draw->Get_Object() == nullptr || !draw->Get_Object()->Is_KindOf(KINDOF_STRUCTURE)) {
            g_theInGameUI->Message(g_theGameText->Fetch("GUI:SelectedAcrossMap"));
        }
    }

    return ret;
}

struct KindOfUnitSelectionData
{
    BitFlags<KINDOF_COUNT> must_be_set;
    BitFlags<KINDOF_COUNT> must_be_clear;
    std::list<Drawable *> selected_units;
};

bool KindOf_Unit_Selection(Drawable *draw, void *user_data)
{
    KindOfUnitSelectionData *data = static_cast<KindOfUnitSelectionData *>(user_data);

    if (draw != nullptr) {
        Object *obj = draw->Get_Object();

        if (obj == nullptr) {
            return false;
        }

        if (obj->Is_KindOf_Multi(data->must_be_set, data->must_be_clear) && obj->Is_Locally_Controlled()
            && !obj->Is_Contained()) {
            if (!obj->Get_Drawable()->Is_Selected() && !obj->Is_Effectively_Dead() && obj->Is_Mass_Selectable()
                && !obj->Is_Outside_Map()) {
                if (g_theInGameUI->Get_Max_Select_Count() <= 0
                    || g_theInGameUI->Get_Select_Count() < g_theInGameUI->Get_Max_Select_Count()) {
                    g_theInGameUI->Select_Drawable(draw);
                    g_theInGameUI->Set_Displayed_Max_Warning(false);
                    data->selected_units.push_back(draw);
                    return true;
                }

                if (!g_theInGameUI->Get_Displayed_Max_Warning()) {
                    g_theInGameUI->Set_Displayed_Max_Warning(true);
                    Utf16String message;
                    message.Format(g_theGameText->Fetch("GUI:MaxSelectionSize"), g_theInGameUI->Get_Max_Select_Count());
                    g_theInGameUI->Message(message);
                }
            }
        }
    }

    return false;
}

int InGameUI::Select_All_Units_By_Type_Across_Region(
    IRegion2D *region, BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    KindOfUnitSelectionData data;
    int count = 0;
    int previous_count = Get_All_Selected_Drawables()->size();
    data.must_be_set = must_be_set;
    data.must_be_clear = must_be_clear;

    if (region != nullptr) {
        g_theTacticalView->Iterate_Drawables_In_Region(region, KindOf_Unit_Selection, &data);
        count += data.selected_units.size();
    } else {
        for (Drawable *drawable = g_theGameClient->First_Drawable(); drawable != nullptr; drawable = drawable->Get_Next()) {
            if (KindOf_Unit_Selection(drawable, &data)) {
                count++;
            }
        }
    }

    if (count > 0) {
        GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_CREATE_SELECTED_GROUP);
        msg->Append_Bool_Arg(previous_count != 0);

        for (auto it = data.selected_units.begin(); it != data.selected_units.end(); it++) {
            Drawable *drawable = *it;

            if (drawable != nullptr && drawable->Get_Object() != nullptr) {
                msg->Append_ObjectID_Arg(drawable->Get_Object()->Get_ID());
            }
        }
    }

    return count;
}

void InGameUI::Build_Region(const ICoord2D *anchor, const ICoord2D *dest, IRegion2D *region)
{
    if (anchor->x >= dest->x) {
        region->lo.x = dest->x;
        region->hi.x = anchor->x;
    } else {
        region->lo.x = anchor->x;
        region->hi.x = dest->x;
    }

    if (anchor->y >= dest->y) {
        region->lo.y = dest->y;
        region->hi.y = anchor->y;
    } else {
        region->lo.y = anchor->y;
        region->hi.y = dest->y;
    }
}

FloatingTextData::FloatingTextData() : m_color(0), m_frameTimeOut(0), m_frameCount(0)
{
    m_pos3D.Zero();
    m_text.Clear();
    m_dString = g_theDisplayStringManager->New_Display_String();
}

FloatingTextData::~FloatingTextData()
{
    if (m_dString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_dString);
        m_dString = nullptr;
    }
}

void InGameUI::Add_Floating_Text(const Utf16String &text, const Coord3D *pos, int color)
{
    if (g_theGameLogic->Get_Draw_Icon_UI()) {
        FloatingTextData *data = new FloatingTextData();
        data->m_frameCount = 0;
        data->m_color = color;
        data->m_pos3D.x = pos->x;
        data->m_pos3D.z = pos->z;
        data->m_pos3D.y = pos->y;
        data->m_text = text;
        data->m_dString->Set_Text(text);

        if (m_floatingTextTimeOut != 0) {
            data->m_frameTimeOut = m_floatingTextTimeOut + g_theGameLogic->Get_Frame();
        } else {
            data->m_frameTimeOut = g_theGameLogic->Get_Frame() + 10;
        }

        m_floatingTextList.push_front(data);
    }
}

void InGameUI::Add_Idle_Worker(Object *obj)
{
    if (obj != nullptr) {
        if (!Find_Idle_Worker(obj)) {
            m_idleWorkerLists[obj->Get_Controlling_Player()->Get_Player_Index()].push_back(obj);
        }
    }
}

void InGameUI::Remove_Idle_Worker(Object *obj, int slot)
{
    if (obj != nullptr && slot < MAX_PLAYER_COUNT && !m_idleWorkerLists[slot].empty()) {
        for (auto it = m_idleWorkerLists[slot].begin(); it != m_idleWorkerLists[slot].end(); it++) {
            if (*it == obj) {
                m_idleWorkerLists[slot].erase(it);
                return;
            }
        }
    }
}

void InGameUI::Select_Next_Idle_Worker()
{
    int index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();

    if (m_idleWorkerLists[index].empty()) {
        captainslog_dbgassert(false,
            "InGameUI::Select_Next_Idle_Worker We're trying to select a worker when our list is empty for player %ls",
            g_thePlayerList->Get_Local_Player()->Get_Player_Display_Name().Str());
    } else {
        Object *obj = nullptr;

        if (Get_Select_Count() == 1) {
            Drawable *draw = g_theInGameUI->Get_First_Selected_Drawable();

            for (auto it = m_idleWorkerLists[index].begin(); it != m_idleWorkerLists[index].end(); it++) {
                if (*it == draw->Get_Object()) {
                    it++;

                    if (it != m_idleWorkerLists[index].end()) {
                        obj = *it;
                    } else {
                        obj = *m_idleWorkerLists[index].begin();
                    }

                    break;
                }
            }

            if (obj == nullptr) {
                obj = *m_idleWorkerLists[index].begin();
            }
        } else {
            obj = *m_idleWorkerLists[index].begin();
        }

        captainslog_dbgassert(obj != nullptr, "InGameUI::Select_Next_Idle_Worker Could not select the next IDLE worker");

        if (obj != nullptr) {
            Object *contain = obj->Get_Contained_By();

            if (contain != nullptr) {
                obj = contain;
            }

            Deselect_All_Drawables(true);
            GameMessage *msg = g_theMessageStream->Append_Message(GameMessage::MSG_CREATE_SELECTED_GROUP);
            msg->Append_Bool_Arg(true);
            msg->Append_ObjectID_Arg(obj->Get_ID());
            Select_Drawable(obj->Get_Drawable());
            g_theTacticalView->Look_At(obj->Get_Position());
        }
    }
}

void InGameUI::Disable_Tooltips_Until(unsigned int frame)
{
    if (frame > m_tooltipsDisabled) {
        m_tooltipsDisabled = frame;
    }
}

void InGameUI::Clear_Tooltips_Disabled()
{
    m_tooltipsDisabled = 0;
}

bool InGameUI::Are_Tooltips_Disabled()
{
    return g_theGameLogic->Get_Frame() < m_tooltipsDisabled;
}

int InGameUI::Get_Idle_Worker_Count()
{
    return m_idleWorkerLists[g_thePlayerList->Get_Local_Player()->Get_Player_Index()].size();
}

Object *InGameUI::Find_Idle_Worker(Object *obj)
{
    if (obj == nullptr) {
        return nullptr;
    }

    int index = obj->Get_Controlling_Player()->Get_Player_Index();

    if (m_idleWorkerLists[index].empty()) {
        return nullptr;
    }

    for (auto it = m_idleWorkerLists[index].begin(); it != m_idleWorkerLists[index].end(); it++) {
        if (*it == obj) {
            return *it;
        }
    }

    return nullptr;
}

void InGameUI::Show_Idle_Worker_Layout()
{
    if (m_idleWorkerWin != nullptr) {
        m_idleWorkerWin->Win_Enable(true);
        m_idleWorkerCount = Get_Idle_Worker_Count();
    } else {
        m_idleWorkerWin = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonIdleWorker"));
        captainslog_dbgassert(
            m_idleWorkerWin != nullptr, "InGameUI::Show_Idle_Worker_Layout could not find IdleWorker.wnd to load ");
    }
}

void InGameUI::Hide_Idle_Worker_Layout()
{
    if (m_idleWorkerWin != nullptr) {
        Gadget_Button_Set_Text(m_idleWorkerWin, Utf16String::s_emptyString);
        m_idleWorkerWin->Win_Enable(false);
        m_idleWorkerCount = -1;
    }
}

void InGameUI::Update_Idle_Worker()
{
    int count = Get_Idle_Worker_Count();

    if (count > 0 && m_idleWorkerCount != count && Get_Input_Enabled()) {
        Show_Idle_Worker_Layout();
    }

    if (count <= 0 && (m_idleWorkerWin != nullptr || Get_Input_Enabled())) {
        Hide_Idle_Worker_Layout();
    }
}

void InGameUI::Reset_Idle_Worker()
{
    if (m_idleWorkerWin != nullptr) {
        Gadget_Button_Set_Text(m_idleWorkerWin, Utf16String::s_emptyString);
    }

    m_idleWorkerCount = -1;

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        m_idleWorkerLists[i].clear();
    }
}

#ifdef GAME_DEBUG_STRUCTS
void InGameUI::Debug_Add_Floating_Text(const Utf8String &text, const Coord3D *pos, int color)
{
    // todo debug stuff
}
#endif

WindowMsgHandledType Idle_Worker_System(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message == GWM_INPUT_FOCUS) {
        if (data_1 == 1) {
            *reinterpret_cast<bool *>(data_2) = false;
        }
    } else if (message != GBM_SELECTED) {
        return MSG_IGNORED;
    }

    static const NameKeyType buttonSelectID =
        g_theNameKeyGenerator->Name_To_Key("IdleWorker.wnd:ButtonSelectNextIdleWorker");
    GameWindow *win = reinterpret_cast<GameWindow *>(data_1);

    if (win != nullptr && win->Win_Get_Window_Id() == buttonSelectID) {
        g_theInGameUI->Select_Next_Idle_Worker();
    }

    return MSG_HANDLED;
}

void InGameUI::Register_Window_Layout(WindowLayout *layout)
{
    Unregister_Window_Layout(layout);
    m_windowLayoutList.push_back(layout);
}

void InGameUI::Unregister_Window_Layout(WindowLayout *layout)
{
    for (auto it = m_windowLayoutList.begin(); it != m_windowLayoutList.end(); it++) {
        if (*it == layout) {
            m_windowLayoutList.erase(it);
            return;
        }
    }
}

void InGameUI::Reset_Camera()
{
    ViewLocation loc;
    g_theTacticalView->Get_Location(&loc);
    g_theTacticalView->Reset_Camera(loc.Get_Pos(), 1, 0.0f, 0.0f);
}

bool InGameUI::Can_Selected_Objects_Non_Attack_Interact_With_Object(
    const Object *object_to_interact_with, SelectionRules rule)
{
    for (int i = ACTIONTYPE_ATTACK_OBJECT; i < NUM_ACTIONTYPES; i++) {
        if (i != ACTIONTYPE_ATTACK_OBJECT
            && Can_Selected_Objects_Do_Action(static_cast<ActionType>(i), object_to_interact_with, rule, false)) {
            return true;
        }
    }

    return false;
}

CanAttackResult InGameUI::Get_Can_Selected_Objects_Attack(
    ActionType action, const Object *object_to_interact_with, SelectionRules rule, bool force_to_attack) const
{
    if ((object_to_interact_with == nullptr && action != ACTIONTYPE_SET_RALLY_POINT)
        || (object_to_interact_with != nullptr && action == ACTIONTYPE_SET_RALLY_POINT)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();
    int count = 0;
    CanAttackResult any_can_attack = ATTACK_RESULT_CANNOT_ATTACK;
    CanAttackResult all_can_attack = ATTACK_RESULT_CAN_ATTACK;

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *draw = *it;
        count++;

        if (action != ACTIONTYPE_ATTACK_OBJECT) {
            captainslog_dbgassert(false,
                "Called InGameUI::Get_Can_Selected_Objects_Attack() with actiontype %d. Only accepts attack "
                "types! Should you be calling InGameUI::Can_Selected_Objects_Do_Action() instead?",
                action);
            return ATTACK_RESULT_UNREACHABLE;
        }

        CanAttackResult attack = g_theActionManager->Get_Can_Attack_Object(draw->Get_Object(),
            object_to_interact_with,
            COMMANDSOURCE_PLAYER,
            static_cast<AbleToAttackType>(force_to_attack));

        if (attack > any_can_attack) {
            any_can_attack = attack;
        }

        if (attack < all_can_attack) {
            all_can_attack = attack;
        }
    }

    if (count <= 0) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    if (rule != SELECTION_ANY) {
        return all_can_attack;
    }

    return any_can_attack;
}

bool InGameUI::Can_Selected_Objects_Do_Action(
    ActionType action, const Object *object_to_interact_with, SelectionRules rule, bool additional_checking) const
{
    if ((object_to_interact_with == nullptr && action != ACTIONTYPE_SET_RALLY_POINT)
        || (object_to_interact_with != nullptr && action == ACTIONTYPE_SET_RALLY_POINT)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();
    int count = 0;
    int qualify = 0;

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *draw = *it;
        count++;
        bool can_do = false;

        switch (action) {
            case ACTIONTYPE_NONE:
                return true;
            case ACTIONTYPE_ATTACK_OBJECT:
                captainslog_dbgassert(false,
                    "Called InGameUI::Can_Selected_Objects_Do_Action() with ACTIONTYPE_ATTACK_OBJECT. You must use "
                    "InGameUI::Get_Can_Selected_Objects_Attack() instead.");
                return false;
            case ACTIONTYPE_GET_REPAIRED_AT:
                can_do = g_theActionManager->Can_Get_Repaired_At(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_DOCK_AT:
                can_do = g_theActionManager->Can_Dock_At(draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_GET_HEALED_AT:
                can_do =
                    g_theActionManager->Can_Get_Healed_At(draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);

                if (can_do) {
                    ContainModuleInterface *contain = object_to_interact_with->Get_Contain();

                    if (contain != nullptr) {
                        if (contain->Is_Heal_Contain()) {
                            can_do = false;
                        }
                    }
                }

                break;
            case ACTIONTYPE_REPAIR_OBJECT: {
                ObjectID id = object_to_interact_with->Get_Sole_Healing_Benefactor();
                can_do =
                    g_theActionManager->Can_Repair_Object(draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER)
                    && (id == INVALID_OBJECT_ID || draw->Get_Object()->Get_ID() == id);
                break;
            }
            case ACTIONTYPE_RESUME_CONSTRUCTION:
                can_do = g_theActionManager->Can_Resume_Construction_Of(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_ENTER_OBJECT:
                can_do = g_theActionManager->Can_Enter_Object(draw->Get_Object(),
                    object_to_interact_with,
                    COMMANDSOURCE_PLAYER,
                    !additional_checking ? CAN_ENTER_1 : CAN_ENTER_0);
                break;
            case ACTIONTYPE_HIJACK_VEHICLE:
                can_do = g_theActionManager->Can_Hijack_Vehicle(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_CONVERT_OBJECT_TO_CARBOMB:
                can_do = g_theActionManager->Can_Convert_Object_To_Car_Bomb(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_CAPTURE_BUILDING_VIA_HACKING:
                can_do = g_theActionManager->Can_Capture_Building(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_DISABLE_VEHICLE_VIA_HACKING:
                can_do = g_theActionManager->Can_Disable_Vehicle_Via_Hacking(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER, true);
                break;
            case ACTIONTYPE_STEAL_CASH_VIA_HACKING:
                can_do = g_theActionManager->Can_Steal_Cash_Via_Hacking(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_DISABLE_BUILDING_VIA_HACKING:
                can_do = g_theActionManager->Can_Disable_Building_Via_Hacking(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_MAKE_OBJECT_DEFECTOR:
                can_do = g_theActionManager->Can_Make_Object_Defector(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            case ACTIONTYPE_SET_RALLY_POINT: {
                Object *obj = draw->Get_Object();

                if (obj != nullptr) {
                    can_do = obj->Is_KindOf(KINDOF_AUTO_RALLYPOINT) && obj->Is_Locally_Controlled();
                } else {
                    can_do = false;
                }

                break;
            }
            case ACTIONTYPE_COMBAT_DROP:
                can_do = g_theActionManager->Can_Enter_Object(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER, CAN_ENTER_2);
                break;
            case ACTIONTYPE_SABOTAGE_BUILDING:
                can_do = g_theActionManager->Can_Sabotage_Building(
                    draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER);
                break;
            default:
                break;
        }

        if (!can_do) {
            continue;
        }

        if (rule == SELECTION_ANY) {
            return true;
        }

        qualify++;
    }

    return rule == SELECTION_ALL && count > 0 && qualify == count;
}

bool InGameUI::Can_Selected_Objects_Do_Special_Power(CommandButton *command,
    const Object *object_to_interact_with,
    Coord3D *position,
    SelectionRules rule,
    unsigned int options,
    Object *custom_object) const
{
    const SpecialPowerTemplate *power = command->Get_Special_Power();
    bool needs_target_pos = (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) != 0;
    bool needs_target_object = (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_OBJECT) != 0;

    if (needs_target_object && object_to_interact_with == nullptr) {
        return false;
    }

    if (needs_target_pos && position == nullptr) {
        return false;
    }

    Drawable *custom_drawable;

    if (custom_object != nullptr) {
        custom_drawable = custom_object->Get_Drawable();
    } else {
        custom_drawable = nullptr;
    }

    std::list<Drawable *> custom_drawables;

    if (custom_drawable != nullptr) {
        custom_drawables.push_back(custom_drawable);
    }

    const std::list<Drawable *> *drawables;

    if (custom_drawables.size() != 0) {
        drawables = &custom_drawables;
    } else {
        drawables = g_theInGameUI->Get_All_Selected_Drawables();
    }

    int count = 0;
    int qualify = 0;

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *drawable = *it;
        count++;

        if (needs_target_object || needs_target_pos) {
            if (needs_target_object) {
                if (g_theActionManager->Can_Do_Special_Power_At_Object(
                        drawable->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER, power, options, true)) {
                    if (rule == SELECTION_ANY) {
                        return true;
                    }

                    qualify++;
                }
            } else if (needs_target_pos) {
                if (g_theActionManager->Can_Do_Special_Power_At_Location(drawable->Get_Object(),
                        position,
                        COMMANDSOURCE_PLAYER,
                        power,
                        object_to_interact_with,
                        options,
                        true)) {
                    if (rule == SELECTION_ANY) {
                        return true;
                    }

                    qualify++;
                }
            }
        } else {
            if (g_theActionManager->Can_Do_Special_Power(drawable->Get_Object(), power, COMMANDSOURCE_PLAYER, options, 1)) {
                if (rule == SELECTION_ANY) {
                    return true;
                }

                qualify++;
            }
        }
    }

    return rule == SELECTION_ALL && count > 0 && qualify == count;
}

bool InGameUI::Can_Selected_Objects_Override_Special_Power_Destination(
    const Coord3D *pos, SelectionRules rule, SpecialPowerType type) const
{
    int count = 0;
    int qualify = 0;
    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        count++;

        if (g_theActionManager->Can_Override_Special_Power_Destination(
                (*it)->Get_Object(), pos, type, COMMANDSOURCE_PLAYER)) {
            if (rule == SELECTION_ANY) {
                return true;
            }

            qualify++;
        }
    }

    return rule == SELECTION_ALL && count > 0 && qualify == count;
}

bool InGameUI::Can_Selected_Objects_Effectively_Use_Weapon(
    CommandButton *command, const Object *object_to_interact_with, Coord3D *position, SelectionRules rule) const
{
    WeaponSlotType wslot = command->Get_Weapon_Slot();
    bool needs_target_pos = (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) != 0;
    bool needs_target_object = (command->Get_Options() & COMMAND_OPTION_NEED_TARGET_OBJECT) != 0;

    if (needs_target_object && object_to_interact_with == nullptr) {
        return false;
    }

    if (needs_target_pos && position == nullptr) {
        return false;
    }

    const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();
    int count = 0;
    int qualify = 0;

    for (auto it = drawables->begin(); it != drawables->end(); it++) {
        Drawable *draw = *it;
        if (needs_target_object || needs_target_pos) {
            if (needs_target_object) {
                if (g_theActionManager->Can_Fire_Weapon_At_Object(
                        draw->Get_Object(), object_to_interact_with, COMMANDSOURCE_PLAYER, wslot)) {
                    if (rule == SELECTION_ANY) {
                        return true;
                    }

                    qualify++;
                }
            } else if (needs_target_pos) {
                if (g_theActionManager->Can_Fire_Weapon_At_Location(
                        draw->Get_Object(), position, COMMANDSOURCE_PLAYER, wslot, object_to_interact_with)) {
                    if (rule == SELECTION_ANY) {
                        return true;
                    }

                    qualify++;
                }
            }
        } else {
            if (g_theActionManager->Can_Fire_Weapon(draw->Get_Object(), wslot, COMMANDSOURCE_PLAYER)) {
                if (rule == SELECTION_ANY) {
                    return true;
                }

                qualify++;
            }
        }
    }

    return rule == SELECTION_ALL && count > 0 && qualify == count;
}
