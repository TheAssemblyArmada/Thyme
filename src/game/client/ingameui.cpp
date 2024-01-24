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
#include "aiguard.h"
#include "behaviormodule.h"
#include "colorspace.h"
#include "controlbar.h"
#include "diplomacy.h"
#include "displaystringmanager.h"
#include "drawable.h"
#include "eva.h"
#include "gameclient.h"
#include "gamefont.h"
#include "gamelogic.h"
#include "gametext.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"
#include "globallanguage.h"
#include "ingamechat.h"
#include "lookatxlat.h"
#include "mouse.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"
#include "radar.h"
#include "recorder.h"
#include "scriptengine.h"
#include "selectionxlat.h"
#include "specialpower.h"
#include "terrainvisual.h"
#include "thingfactory.h"
#include "w3ddisplay.h"
#include "w3dview.h"
#include "windowlayout.h"

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
    int player_index, Utf8String const &power_name, ObjectID id, SpecialPowerTemplate const *power_template)
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
    int player_index, Utf8String const &power_name, ObjectID id, SpecialPowerTemplate const *power_template)
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
    int player_index, Utf8String const &power_name, ObjectID id, SpecialPowerTemplate const *power_template)
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

void InGameUI::Object_Changed_Team(Object const *obj, int old_player_index, int new_player_index)
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

void InGameUI::Hide_Object_Superweapon_Display_By_Script(Object const *obj)
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

void InGameUI::Show_Object_Superweapon_Display_By_Script(Object const *obj)
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

                if (timer_frame2 != 0 || !timer_info->m_countdown || this->m_namedTimerFlashDuration == 0.0f) {
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
#ifdef GAME_DLL
    Call_Method<void, InGameUI, Anim2DTemplate *, const Coord3D *, WorldAnimationOptions, float, float>(
        PICK_ADDRESS(0x00510A10, 0x00812ED9), this, anim, pos, options, time, z_rise);
#endif
}

bool InGameUI::Are_Selected_Objects_Controllable() const
{
    const std::list<Drawable *> *list = g_theInGameUI->Get_All_Selected_Drawables();

    for (auto it = list->begin(); it != list->end(); it++) {
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
    RadiusCursorType cursor, SpecialPowerTemplate const *power_template, WeaponSlotType weapon_slot)
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

void InGameUI::Create_Command_Hint(GameMessage const *msg)
{
    if (m_isScrolling || m_isSelecting || g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK) {
        return;
    }

    Drawable *draw = g_theGameClient->Find_Drawable_By_ID(m_mousedOverObjectID);
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

        if (shrouded == SHROUDED_UNK4) {
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

void InGameUI::Set_GUI_Command(CommandButton const *command)
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

void InGameUI::Place_Build_Available(ThingTemplate const *build, Drawable *build_drawable)
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
            captainslog_dbgassert(*m_placeIcon == nullptr, "Place_Build_Available, build icon array is not empty!");
            *m_placeIcon = ghost_drawable;
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
