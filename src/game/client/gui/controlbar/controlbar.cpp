/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "controlbar.h"
#include "actionmanager.h"
#include "animatewindowmanager.h"
#include "behaviormodule.h"
#include "captainslog.h"
#include "colorspace.h"
#include "controlbarscheme.h"
#include "drawable.h"
#include "gadgetprogressbar.h"
#include "gadgetpushbutton.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "gamewindowmanager.h"
#include "gamewindowtransitions.h"
#include "hotkey.h"
#include "image.h"
#include "multiplayersettings.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "scriptengine.h"
#include "specialpower.h"
#include "stealthupdate.h"
#include "thingfactory.h"
#include "windowlayout.h"
#include "windowvideomanager.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ControlBar *g_theControlBar;
#endif

ControlBar::ContainEntry ControlBar::s_containData[CommandSet::MAX_COMMAND_BUTTONS];
const Image *ControlBar::s_rankEliteIcon;
const Image *ControlBar::s_rankHeroicIcon;
const Image *ControlBar::s_rankVeteranIcon;

FieldParse CommandSet::s_commandSetFieldParseTable[] = {
    { "1", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(0), offsetof(CommandSet, m_command) },
    { "2", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(1), offsetof(CommandSet, m_command) },
    { "3", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(2), offsetof(CommandSet, m_command) },
    { "4", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(3), offsetof(CommandSet, m_command) },
    { "5", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(4), offsetof(CommandSet, m_command) },
    { "6", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(5), offsetof(CommandSet, m_command) },
    { "7", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(6), offsetof(CommandSet, m_command) },
    { "8", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(7), offsetof(CommandSet, m_command) },
    { "9", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(8), offsetof(CommandSet, m_command) },
    { "10", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(9), offsetof(CommandSet, m_command) },
    { "11", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(10), offsetof(CommandSet, m_command) },
    { "12", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(11), offsetof(CommandSet, m_command) },
    { "13", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(12), offsetof(CommandSet, m_command) },
    { "14", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(13), offsetof(CommandSet, m_command) },
    { "15", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(14), offsetof(CommandSet, m_command) },
    { "16", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(15), offsetof(CommandSet, m_command) },
    { "17", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(16), offsetof(CommandSet, m_command) },
    { "18", &CommandSet::Parse_Command_Button, reinterpret_cast<void *>(17), offsetof(CommandSet, m_command) },
    { nullptr, nullptr, nullptr, 0 }
};

CommandSet::CommandSet(Utf8String const &name) : m_name(name), m_nextCommandSet(nullptr)
{
    for (int i = 0; i < MAX_COMMAND_BUTTONS; i++) {
        m_command[i] = nullptr;
    }
}

CommandSet::~CommandSet() {}

const CommandButton *CommandSet::Get_Command_Button(int button_index) const
{
    const CommandButton *button;
    if (g_theGameLogic != nullptr && g_theGameLogic->Find_Control_Bar_Override(m_name, button_index, button)) {
        return button;
    }

    return m_command[button_index];
}

void CommandSet::Friend_Add_To_List(CommandSet **next)
{
    m_nextCommandSet = *next;
    *next = this;
}

void CommandSet::Parse_Command_Button(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    const CommandButton *button = g_theControlBar->Find_Command_Button(name);

    captainslog_relassert(button != nullptr,
        CODE_06,
        "[LINE: %d - FILE: '%s'] Unknown command '%s' found in command set",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        name);
    const CommandButton **buttons = static_cast<const CommandButton **>(store);
    size_t index = reinterpret_cast<size_t>(user_data);
    captainslog_dbgassert(index < MAX_COMMAND_BUTTONS, "Parse_Command_Button: button index '%d' out of range", index);
    buttons[index] = button;
}

const char *s_theCommandOptionNames[] = { "NEED_TARGET_ENEMY_OBJECT",
    "NEED_TARGET_NEUTRAL_OBJECT",
    "NEED_TARGET_ALLY_OBJECT",
    "unused-reserved",
    "ALLOW_SHRUBBERY_TARGET",
    "NEED_TARGET_POS",
    "NEED_UPGRADE",
    "NEED_SPECIAL_POWER_SCIENCE",
    "OK_FOR_MULTI_SELECT",
    "CONTEXTMODE_COMMAND",
    "CHECK_LIKE",
    "ALLOW_MINE_TARGET",
    "ATTACK_OBJECTS_POSITION",
    "OPTION_ONE",
    "OPTION_TWO",
    "OPTION_THREE",
    "NOT_QUEUEABLE",
    "SINGLE_USE_COMMAND",
    "---DO-NOT-USE---",
    "SCRIPT_ONLY",
    "IGNORES_UNDERPOWERED",
    "USES_MINE_CLEARING_WEAPONSET",
    "CAN_USE_WAYPOINTS",
    "MUST_BE_STOPPED",
    nullptr };

static LookupListRec s_theWeaponSlotTypeNamesLookupList[] = { { "PRIMARY", WEAPONSLOT_PRIMARY },
    { "SECONDARY", WEAPONSLOT_SECONDARY },
    { "TERTIARY", WEAPONSLOT_TERTIARY },
    { nullptr, 0 } };

static LookupListRec s_commandButtonMappedBorderTypeNames[] = { { "NONE", COMMAND_BUTTON_BORDER_NONE },
    { "BUILD", COMMAND_BUTTON_BORDER_BUILD },
    { "UPGRADE", COMMAND_BUTTON_BORDER_UPGRADE },
    { "ACTION", COMMAND_BUTTON_BORDER_ACTION },
    { "SYSTEM", COMMAND_BUTTON_BORDER_SYSTEM },
    { nullptr, 0 } };

const char *s_theRadiusCursorNames[] = { "NONE",
    "ATTACK_DAMAGE_AREA",
    "ATTACK_SCATTER_AREA",
    "ATTACK_CONTINUE_AREA",
    "GUARD_AREA",
    "EMERGENCY_REPAIR",
    "FRIENDLY_SPECIALPOWER",
    "OFFENSIVE_SPECIALPOWER",
    "SUPERWEAPON_SCATTER_AREA",
    "PARTICLECANNON",
    "A10STRIKE",
    "CARPETBOMB",
    "DAISYCUTTER",
    "PARADROP",
    "SPYSATELLITE",
    "SPECTREGUNSHIP",
    "HELIX_NAPALM_BOMB",
    "NUCLEARMISSILE",
    "EMPPULSE",
    "ARTILLERYBARRAGE",
    "NAPALMSTRIKE",
    "CLUSTERMINES",
    "SCUDSTORM",
    "ANTHRAXBOMB",
    "AMBUSH",
    "RADAR",
    "SPYDRONE",
    "FRENZY",
    "CLEARMINES",
    "AMBULANCE",
    nullptr };

const char *s_theGuiCommandNames[] = { "NONE",
    "DOZER_CONSTRUCT",
    "DOZER_CONSTRUCT_CANCEL",
    "UNIT_BUILD",
    "CANCEL_UNIT_BUILD",
    "PLAYER_UPGRADE",
    "OBJECT_UPGRADE",
    "CANCEL_UPGRADE",
    "ATTACK_MOVE",
    "GUARD",
    "GUARD_WITHOUT_PURSUIT",
    "GUARD_FLYING_UNITS_ONLY",
    "STOP",
    "WAYPOINTS",
    "EXIT_CONTAINER",
    "EVACUATE",
    "EXECUTE_RAILED_TRANSPORT",
    "BEACON_DELETE",
    "SET_RALLY_POINT",
    "SELL",
    "FIRE_WEAPON",
    "SPECIAL_POWER",
    "PURCHASE_SCIENCE",
    "HACK_INTERNET",
    "TOGGLE_OVERCHARGE",
    "COMBATDROP",
    "SWITCH_WEAPON",
    "HIJACK_VEHICLE",
    "CONVERT_TO_CARBOMB",
    "SABOTAGE_BUILDING",
    "PLACE_BEACON",
    "SPECIAL_POWER_FROM_SHORTCUT",
    "SPECIAL_POWER_CONSTRUCT",
    "SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT",
    "SELECT_ALL_UNITS_OF_TYPE",
    nullptr };

FieldParse CommandButton::s_commandButtonFieldParseTable[] = {
    { "Command", &CommandButton::Parse_Command, nullptr, offsetof(CommandButton, m_command) },
    { "Options", &INI::Parse_Bitstring32, s_theCommandOptionNames, offsetof(CommandButton, m_options) },
    { "Object", &INI::Parse_Thing_Template, nullptr, offsetof(CommandButton, m_thingTemplate) },
    { "Upgrade", &INI::Parse_Upgrade_Template, nullptr, offsetof(CommandButton, m_upgradeTemplate) },
    { "WeaponSlot", &INI::Parse_Lookup_List, s_theWeaponSlotTypeNamesLookupList, offsetof(CommandButton, m_weaponSlot) },
    { "MaxShotsToFire", &INI::Parse_Int, nullptr, offsetof(CommandButton, m_maxShotsToFire) },
    { "Science", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(CommandButton, m_sciences) },
    { "SpecialPower", &INI::Parse_Special_Power_Template, nullptr, offsetof(CommandButton, m_specialPower) },
    { "TextLabel", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_textLabel) },
    { "DescriptLabel", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_descriptLabel) },
    { "PurchasedLabel", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_purchasedLabel) },
    { "ConflictingLabel", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_conflictingLabel) },
    { "ButtonImage", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_buttonImageString) },
    { "CursorName", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_cursorName) },
    { "InvalidCursorName", &INI::Parse_AsciiString, nullptr, offsetof(CommandButton, m_invalidCursorName) },
    { "ButtonBorderType",
        &INI::Parse_Lookup_List,
        s_commandButtonMappedBorderTypeNames,
        offsetof(CommandButton, m_commandButtonBorder) },
    { "RadiusCursorType", &INI::Parse_Index_List, s_theRadiusCursorNames, offsetof(CommandButton, m_radiusCursorType) },
    { "UnitSpecificSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(CommandButton, m_unitSpecificSound) },
    { nullptr, nullptr, nullptr, 0 }
};

CommandButton::CommandButton() :
    m_command(GUI_COMMAND_NONE),
    m_nextCommandButton(nullptr),
    m_options(0),
    m_thingTemplate(nullptr),
    m_upgradeTemplate(nullptr),
    m_specialPower(nullptr),
    m_radiusCursorType(RADIUS_CURSOR_NONE),
    m_weaponSlot(WEAPONSLOT_PRIMARY),
    m_maxShotsToFire(0x7FFFFFFF),
    m_commandButtonBorder(COMMAND_BUTTON_BORDER_NONE),
    m_unk(0),
    m_buttonImage(nullptr),
    m_cameoFlashTime(0)
{
}

CommandButton::~CommandButton() {}

bool CommandButton::Is_Valid_Object_Target(const Player *player, const Object *obj) const
{
    if (player == nullptr || obj == nullptr) {
        return false;
    }

    return Is_Valid_Relationship_Target(player->Get_Relationship(obj->Get_Team()));
}

bool CommandButton::Is_Valid_Object_Target(const Object *obj1, const Object *obj2) const
{
    if (obj1 == nullptr || obj2 == nullptr) {
        return false;
    }

    return Is_Valid_Relationship_Target(obj1->Get_Relationship(obj2));
}

bool CommandButton::Is_Valid_Relationship_Target(Relationship r) const
{
    unsigned int options = 0;

    if (r == ENEMIES) {
        options = COMMAND_OPTION_NEED_TARGET_ENEMY_OBJECT;
    } else if (r == ALLIES) {
        options = COMMAND_OPTION_NEED_TARGET_ALLY_OBJECT;
    } else if (r == NEUTRAL) {
        options = COMMAND_OPTION_NEED_TARGET_NEUTRAL_OBJECT;
    }

    return (options & m_options) != 0;
}

void CommandButton::Parse_Command(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();

    for (int i = 0; s_theGuiCommandNames[i] != nullptr; i++) {
        if (strcasecmp(s_theGuiCommandNames[i], name) == 0) {
            *reinterpret_cast<GUICommand *>(store) = static_cast<GUICommand>(i);
            return;
        }
    }

    throw CODE_06;
}

bool CommandButton::Is_Valid_To_Use_On(Object *obj, const Object *obj2, const Coord3D *pos, CommandSourceType type) const
{
    if (m_upgradeTemplate != nullptr) {
        ProductionUpdateInterface *interface = obj->Get_Production_Update_Interface();

        if (interface != nullptr) {
            for (const ProductionEntry *entry = interface->First_Production(); entry;
                 entry = interface->Next_Production(entry)) {
                if (entry->Get_Production_Object() != nullptr) {
                    return false;
                }
            }

            return obj->Affected_By_Upgrade(m_upgradeTemplate) && !obj->Has_Upgrade(m_upgradeTemplate);
        } else {
            return false;
        }
    } else {
        if ((m_options & COMMAND_OPTION_NEED_TARGET_OBJECT) != 0 && obj2 == nullptr) {
            return false;
        }

        Coord3D loc;

        if (pos != nullptr) {
            loc.Set(pos);
        }

        if ((m_options & COMMAND_OPTION_NEED_TARGET_POS) != 0 && pos == nullptr) {
            if (obj2 == nullptr) {
                return false;
            }

            loc.Set(obj2->Get_Position());
        }

        if ((m_options & COMMAND_OPTION_NEED_TARGET_OBJECT) != 0) {
            return g_theActionManager->Can_Do_Special_Power_At_Object(obj, obj2, type, m_specialPower, m_options, false);
        } else if ((m_options & COMMAND_OPTION_NEED_TARGET_POS) != 0) {
            return g_theActionManager->Can_Do_Special_Power_At_Location(
                obj, &loc, type, m_specialPower, nullptr, m_options, false);
        } else {
            return g_theActionManager->Can_Do_Special_Power(obj, m_specialPower, type, m_options, false);
        }
    }
}

bool CommandButton::Is_Ready(const Object *obj) const
{
    SpecialPowerModuleInterface *module = obj->Get_Special_Power_Module(m_specialPower);

    if (module != nullptr && module->Get_Percent_Ready() == 1.0f) {
        return true;
    }

    return m_upgradeTemplate != nullptr && obj->Affected_By_Upgrade(m_upgradeTemplate)
        && !obj->Has_Upgrade(m_upgradeTemplate);
}

bool CommandButton::Is_Valid_Object_Target(const Drawable *drawable, const Drawable *drawable2) const
{
    const Object *obj2;

    if (drawable2 != nullptr) {
        obj2 = drawable2->Get_Object();
    } else {
        obj2 = nullptr;
    }

    const Object *obj;

    if (drawable != nullptr) {
        obj = drawable->Get_Object();
    } else {
        obj = nullptr;
    }

    return Is_Valid_Object_Target(obj, obj2);
}

bool CommandButton::Is_Context_Command() const
{
    return (m_options & COMMAND_OPTION_CONTEXTMODE_COMMAND) != 0;
}

void CommandButton::Copy_Images_From(const CommandButton *button, bool set_dirty)
{
    if (m_buttonImage != button->Get_Button_Image()) {
        m_buttonImage = button->Get_Button_Image();

        if (set_dirty) {
            g_theControlBar->Mark_UI_Dirty();
        }
    }
}

void CommandButton::Copy_Button_Text_From(const CommandButton *button, bool conflicting_label, bool set_dirty)
{
    bool is_dirty = false;

    if (conflicting_label) {
        if (button->Get_Conflicting_Label().Is_Not_Empty()) {
            if (m_textLabel.Compare(button->Get_Conflicting_Label()) != 0) {
                m_textLabel = button->Get_Conflicting_Label();
                is_dirty = true;
            }
        }
    } else {
        if (button->Get_Text_Label().Is_Not_Empty()) {
            if (m_textLabel.Compare(button->Get_Text_Label()) != 0) {
                m_textLabel = button->Get_Text_Label();
                is_dirty = true;
            }
        }
    }

    if (button->Get_Descript_Label().Is_Not_Empty()) {
        if (m_descriptLabel.Compare(button->Get_Descript_Label()) != 0) {
            m_descriptLabel = button->Get_Descript_Label();
            is_dirty = true;
        }
    }

    if (set_dirty && is_dirty) {
        g_theControlBar->Mark_UI_Dirty();
    }
}

void CommandButton::Cache_Button_Image()
{
    if (g_theMappedImageCollection != nullptr && m_buttonImageString.Is_Not_Empty()) {
        m_buttonImage = g_theMappedImageCollection->Find_Image_By_Name(m_buttonImageString);
        captainslog_dbgassert(m_buttonImage != nullptr,
            "CommandButton: %s is looking for button image %s but can't find it. Skipping...",
            m_name.Str(),
            m_buttonImageString.Str());
        m_buttonImageString.Clear();
    }
}

void CommandButton::Friend_Add_To_List(CommandButton **next)
{
    m_nextCommandButton = *next;
    *next = this;
}

ControlBar::ControlBar() :
    m_videoManager(nullptr),
    m_controlBarAnimateWindowManager(nullptr),
    m_specialPowerShortcutAnimateWindowManager(nullptr),
    m_unkAnimateWindowManager(nullptr),
    m_parentXPosition(0),
    m_parentYPosition(0),
    m_controlBarConfig(0),
    m_UIDirty(false),
    m_commandButtons(nullptr),
    m_commandSets(nullptr),
    m_controlBarSchemeManager(nullptr),
    m_currentSelectedDrawable(nullptr),
    m_currContext(CB_CONTEXT_NONE),
    m_rallyPointDrawable(nullptr),
    m_displayedConstructPercent(-1.0f),
    m_oclTimerFrame(0),
    m_displayedQueueCount(0),
    m_lastRecordedInventoryCount(0),
    m_rightHUDWindow(nullptr),
    m_rightHUDCameoWindow(nullptr),
    m_unitSelectedWindow(nullptr),
    m_popupCommunicator(nullptr),
    m_generalsPointsLayout(nullptr),
    m_specialPowerShortcutButtonCount(0),
    m_specialPowerShortcutBarLayout(nullptr),
    m_specialPowerShortcutBarParent(nullptr),
    m_cameoFlash(false),
    m_unk1(false),
    m_unk2(0),
    m_unk3(0),
    m_unk4(0),
    m_unk5(0),
    m_unk6(0),
    m_unk7(0),
    m_unk8(0),
    m_unk9(0),
    m_unkWindow(nullptr),
    m_unk10(0),
    m_buildUpClockColor(Make_Color(0, 0, 0, 100)),
    m_isObserver(false),
    m_observerPlayer(nullptr),
    m_controlBarPopupDescriptionLayout(nullptr),
    m_buildTooltipLayoutVisible(false),
    m_unkColor(Make_Color(0, 0, 0, 100)),
    m_barButtonGenStarOnIcon(nullptr),
    m_barButtonGenStarOffIcon(nullptr),
    m_toggleButtonUpInImage(nullptr),
    m_toggleButtonUpOnImage(nullptr),
    m_toggleButtonUpPushedImage(nullptr),
    m_toggleButtonDownInImage(nullptr),
    m_toggleButtonDownOnImage(nullptr),
    m_toggleButtonDownPushedImage(nullptr),
    m_unk11(0),
    m_genArrowImage(nullptr),
    m_generalButtonEnableImage(nullptr),
    m_generalButtonHilitedImage(nullptr),
    m_starHilight(false),
    m_sciencePurchasePoints(0),
    m_foregroundMarkerPosX(0),
    m_foregroundMarkerPosY(0),
    m_backgroundMarkerPosX(0),
    m_backgroundMarkerPosY(0),
    m_triggerRadarAttackGlow(false),
    m_radarAttackGlowCounter(0),
    m_radarAttackGlowWindow(nullptr)
{
    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        m_commonCommands[i] = nullptr;
    }

    for (int i = 0; i < CONTEXT_PARENT_COUNT; i++) {
        m_contextParent[i] = nullptr;
    }

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        m_commandWindows[i] = nullptr;
    }

    for (int i = 0; i < UNIT_UPGRADE_WINDOW_COUNT; i++) {
        m_unitUpgradeWindows[i] = nullptr;
    }

    for (int i = 0; i < RANK_1_BUTTON_COUNT; i++) {
        m_rank1Buttons[i] = nullptr;
    }

    for (int i = 0; i < RANK_3_BUTTON_COUNT; i++) {
        m_rank3Buttons[i] = nullptr;
    }

    for (int i = 0; i < RANK_8_BUTTON_COUNT; i++) {
        m_rank8Buttons[i] = nullptr;
    }

    for (int i = 0; i < SPECIAL_POWER_SHORTCUT_BUTTON_COUNT; i++) {
        m_specialPowerShortcutButtons[i] = 0;
        m_specialPowerShortcutButtonParents[i] = 0;
        m_specialPowerShortcutButtonUnk[i] = 0;
    }

    Reset_Build_Queue_Data();
    Reset_Contain_Data();
    Update_Command_Bar_Border_Colors(0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF);

#ifdef GAME_DEBUG_STRUCTS
    m_uiDirtyCounter = 0;
    m_uiDirtyFrame = 0;
#endif
}

ControlBar::~ControlBar()
{
    if (m_generalsPointsLayout != nullptr) {
        m_generalsPointsLayout->Destroy_Windows();
        m_generalsPointsLayout->Delete_Instance();
    }

    m_generalsPointsLayout = nullptr;
    m_genArrowImage = nullptr;

    if (m_videoManager != nullptr) {
        delete m_videoManager;
    }

    m_videoManager = nullptr;

    if (m_specialPowerShortcutAnimateWindowManager != nullptr) {
        delete m_specialPowerShortcutAnimateWindowManager;
    }

    m_specialPowerShortcutAnimateWindowManager = nullptr;

    if (m_controlBarAnimateWindowManager != nullptr) {
        delete m_controlBarAnimateWindowManager;
    }

    m_controlBarAnimateWindowManager = nullptr;

    if (m_unkAnimateWindowManager != nullptr) {
        delete m_unkAnimateWindowManager;
    }

    m_unkAnimateWindowManager = nullptr;

    if (m_controlBarSchemeManager != nullptr) {
        delete m_controlBarSchemeManager;
    }

    m_controlBarSchemeManager = nullptr;

    while (m_commandSets != nullptr) {
        CommandSet *next = m_commandSets->Get_Next_Command_Set();
        m_commandSets->Delete_Instance();
        m_commandSets = next;
    }

    while (m_commandButtons != nullptr) {
        CommandButton *next = m_commandButtons->Get_Next();
        m_commandButtons->Delete_Instance();
        m_commandButtons = next;
    }

    if (m_controlBarPopupDescriptionLayout != nullptr) {
        m_controlBarPopupDescriptionLayout->Destroy_Windows();
        m_controlBarPopupDescriptionLayout->Delete_Instance();
        m_controlBarPopupDescriptionLayout = nullptr;
    }

    if (m_specialPowerShortcutBarLayout != nullptr) {
        m_specialPowerShortcutBarLayout->Destroy_Windows();
        m_specialPowerShortcutBarLayout->Delete_Instance();
        m_specialPowerShortcutBarLayout = nullptr;
    }

    m_radarAttackGlowWindow = nullptr;

    if (m_rightHUDCameoWindow != nullptr && m_rightHUDCameoWindow->Win_Get_User_Data() != nullptr) {
        delete m_rightHUDCameoWindow->Win_Get_User_Data();
    }
}

void ControlBar::Update_Command_Bar_Border_Colors(int build, int action, int upgrade, int system)
{
    m_commandButtonBorderBuildColor = build;
    m_commandButtonBorderActionColor = action;
    m_commandButtonBorderUpgradeColor = upgrade;
    m_commandButtonBorderSystemColor = system;
}

void Command_Button_Tooltip(GameWindow *window, WinInstanceData *instance, unsigned int mouse)
{
    g_theControlBar->Show_Build_Tooltip_Layout(window);
}

void ControlBar::Init()
{
    INI ini;
    m_unk1 = false;
    ini.Load("Data\\INI\\Default\\CommandButton.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\CommandButton.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\CommandSet.ini", INI_LOAD_OVERWRITE, nullptr);
    Post_Process_Commands();
    m_controlBarSchemeManager = new ControlBarSchemeManager();
    m_controlBarSchemeManager->Init();

    if (g_theWindowManager != nullptr) {
        m_contextParent[0] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ControlBarParent"));
        m_contextParent[0]->Win_Get_Position(&m_parentXPosition, &m_parentYPosition);
        m_generalsPointsLayout = g_theWindowManager->Win_Create_Layout("GeneralsExpPoints.wnd");
        m_generalsPointsLayout->Hide(true);

        m_contextParent[1] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("GeneralsExpPoints.wnd:GenExpParent"));
        m_contextParent[5] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:UnderConstructionWindow"));
        m_contextParent[8] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:OCLTimerWindow"));
        m_contextParent[4] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BeaconWindow"));
        m_contextParent[2] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:CommandWindow"));
        m_contextParent[3] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ProductionQueueWindow"));
        m_contextParent[7] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ObserverPlayerListWindow"));
        m_contextParent[6] = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ObserverPlayerInfoWindow"));

        Utf8String str;

        for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
            str.Format("ControlBar.wnd:ButtonCommand%02d", i + 1);
            m_commandWindows[i] = g_theWindowManager->Win_Get_Window_From_Id(
                m_contextParent[2], g_theNameKeyGenerator->Name_To_Key(str.Str()));

            if (m_commandWindows[i] != nullptr) {
                m_commandWindows[i]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
            }
        }

        for (int i = 0; i < RANK_1_BUTTON_COUNT; i++) {
            str.Format("GeneralsExpPoints.wnd:ButtonRank1Number%d", i);
            m_rank1Buttons[i] = g_theWindowManager->Win_Get_Window_From_Id(
                m_contextParent[1], g_theNameKeyGenerator->Name_To_Key(str.Str()));
            m_rank1Buttons[i]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        }

        for (int i = 0; i < RANK_3_BUTTON_COUNT; i++) {
            str.Format("GeneralsExpPoints.wnd:ButtonRank3Number%d", i);
            m_rank3Buttons[i] = g_theWindowManager->Win_Get_Window_From_Id(
                m_contextParent[1], g_theNameKeyGenerator->Name_To_Key(str.Str()));
            m_rank3Buttons[i]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        }

        for (int i = 0; i < RANK_8_BUTTON_COUNT; i++) {
            str.Format("GeneralsExpPoints.wnd:ButtonRank8Number%d", i);
            m_rank8Buttons[i] = g_theWindowManager->Win_Get_Window_From_Id(
                m_contextParent[1], g_theNameKeyGenerator->Name_To_Key(str.Str()));
            m_rank8Buttons[i]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        }

        m_rightHUDWindow = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:RightHUD"));
        m_unitSelectedWindow = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:WinUnitSelected"));
        m_rightHUDCameoWindow = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:CameoWindow"));

        for (int i = 0; i < UNIT_UPGRADE_WINDOW_COUNT; i++) {
            str.Format("ControlBar.wnd:UnitUpgrade%d", i + 1);
            m_unitUpgradeWindows[i] =
                g_theWindowManager->Win_Get_Window_From_Id(m_rightHUDWindow, g_theNameKeyGenerator->Name_To_Key(str.Str()));
            m_unitUpgradeWindows[i]->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        }

        m_popupCommunicator = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PopupCommunicator"));

        Set_Control_Command(m_popupCommunicator, Find_Command_Button("NonCommand_Communicator"));
        m_popupCommunicator->Win_Set_Tooltip_Func(Command_Button_Tooltip);

        GameWindow *button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonOptions"));

        if (button != nullptr) {
            Set_Control_Command(button, Find_Command_Button("NonCommand_Options"));
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonIdleWorker"));

        if (button != nullptr) {
            Set_Control_Command(button, Find_Command_Button("NonCommand_IdleWorker"));
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonPlaceBeacon"));

        if (button != nullptr) {
            Set_Control_Command(button, Find_Command_Button("NonCommand_Beacon"));
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonGeneral"));

        if (button != nullptr) {
            Set_Control_Command(button, Find_Command_Button("NonCommand_GeneralsExperience"));
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonLarge"));

        if (button != nullptr) {
            Set_Control_Command(button, Find_Command_Button("NonCommand_UpDown"));
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PowerWindow"));

        if (button != nullptr) {
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:MoneyDisplay"));

        if (button != nullptr) {
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:GeneralsExp"));

        if (button != nullptr) {
            button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
        }

        m_radarAttackGlowWindow = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:WinUAttack"));

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BackgroundMarker"));
        button->Win_Get_Screen_Position(&m_foregroundMarkerPosX, &m_foregroundMarkerPosY);

        button = g_theWindowManager->Win_Get_Window_From_Id(
            nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BackgroundMarker"));
        button->Win_Get_Screen_Position(&m_backgroundMarkerPosX, &m_backgroundMarkerPosY);

        if (m_videoManager == nullptr) {
            m_videoManager = new WindowVideoManager();
        }

        if (m_controlBarAnimateWindowManager == nullptr) {
            m_controlBarAnimateWindowManager = new AnimateWindowManager();
        }

        if (m_unkAnimateWindowManager == nullptr) {
            m_unkAnimateWindowManager = new AnimateWindowManager();
        }

        if (m_specialPowerShortcutAnimateWindowManager == nullptr) {
            m_specialPowerShortcutAnimateWindowManager = new AnimateWindowManager();
        }

        m_controlBarPopupDescriptionLayout = g_theWindowManager->Win_Create_Layout("ControlBarPopupDescription.wnd");

        if (m_controlBarPopupDescriptionLayout != nullptr) {
            m_controlBarPopupDescriptionLayout->Hide(true);
            m_controlBarPopupDescriptionLayout->Set_Update(Control_Bar_Popup_Description_Update_Func);
        }

        const Image *image;

        if (g_theMappedImageCollection != nullptr) {
            image = g_theMappedImageCollection->Find_Image_By_Name("BarButtonGenStarON");
        } else {
            image = nullptr;
        }

        m_barButtonGenStarOnIcon = image;

        if (g_theMappedImageCollection != nullptr) {
            image = g_theMappedImageCollection->Find_Image_By_Name("BarButtonGenStarOFF");
        } else {
            image = nullptr;
        }

        m_barButtonGenStarOffIcon = image;
        m_starHilight = true;
        m_sciencePurchasePoints = -1;

        if (g_theMappedImageCollection != nullptr) {
            image = g_theMappedImageCollection->Find_Image_By_Name("SSChevron1L");
        } else {
            image = nullptr;
        }

        s_rankVeteranIcon = image;

        if (g_theMappedImageCollection != nullptr) {
            image = g_theMappedImageCollection->Find_Image_By_Name("SSChevron2L");
        } else {
            image = nullptr;
        }

        s_rankEliteIcon = image;

        if (g_theMappedImageCollection != nullptr) {
            image = g_theMappedImageCollection->Find_Image_By_Name("SSChevron3L");
        } else {
            image = nullptr;
        }

        s_rankHeroicIcon = image;
        Init_Observer_Controls();
        Switch_To_Context(CB_CONTEXT_NONE, nullptr);
    }
}

void ControlBar::Get_Background_Marker_Pos(int *x, int *y)
{
    *x = m_backgroundMarkerPosX;
    *y = m_backgroundMarkerPosY;
}

void ControlBar::Post_Process_Commands()
{
    for (CommandButton *button = m_commandButtons; button != nullptr; button = button->Get_Next()) {
        button->Cache_Button_Image();
    }
}

void ControlBar::Set_Control_Command(GameWindow *button, const CommandButton *command_button)
{
    if (button->Win_Get_Input_Func() == Gadget_Push_Button_Input) {
        if (command_button != nullptr) {
            if ((command_button->Get_Options() & COMMAND_OPTION_CHECK_LIKE) != 0) {
                Gadget_Button_Enable_Check_Like(button, true, false);
            } else {
                Gadget_Button_Enable_Check_Like(button, false, false);
            }

            if (command_button->Get_Button_Image() != nullptr) {
                Gadget_Button_Set_Enabled_Image(button, command_button->Get_Button_Image());
            }

            if (command_button->Get_Text_Label().Is_Empty() && command_button->Get_Sciences()->empty()) {
                Gadget_Button_Set_Text(button, U_CHAR(""));
            } else {
                button->Win_Set_Tooltip_Func(Command_Button_Tooltip);
            }

            Gadget_Button_Set_Data(button, const_cast<CommandButton *>(command_button));
            Set_Command_Bar_Border(button, command_button->Get_Border());

            if (g_theHotKeyManager != nullptr) {
                Utf8String key = g_theHotKeyManager->Search_Hot_Key(command_button->Get_Text_Label());

                if (key.Is_Not_Empty()) {
                    g_theHotKeyManager->Add_Hot_Key(button, key);
                }
            }

            Gadget_Button_Set_Alt_Sound(button, "GUICommandBarClick");
        } else {
            captainslog_dbgassert(false, "Set_Control_Command: NULL commandButton passed in");
        }
    } else {
        captainslog_dbgassert(false, "Set_Control_Command: Window is not a button");
    }
}

void ControlBar::Set_Command_Bar_Border(GameWindow *button, CommandButtonMappedBorderType type)
{
    if (button != nullptr) {
        switch (type) {
            case COMMAND_BUTTON_BORDER_BUILD:
                Gadget_Button_Set_Border(button, m_commandButtonBorderBuildColor, true);
                break;
            case COMMAND_BUTTON_BORDER_UPGRADE:
                Gadget_Button_Set_Border(button, m_commandButtonBorderUpgradeColor, true);
                break;
            case COMMAND_BUTTON_BORDER_ACTION:
                Gadget_Button_Set_Border(button, m_commandButtonBorderActionColor, true);
                break;
            case COMMAND_BUTTON_BORDER_SYSTEM:
                Gadget_Button_Set_Border(button, m_commandButtonBorderSystemColor, true);
                break;
            default:
                Gadget_Button_Set_Border(button, 0xFFFFFF, false);
                break;
        }
    }
}

void ControlBar::Reset()
{
    Hide_Special_Power_Shortcut();
    m_rallyPointDrawable = nullptr;

    if (m_radarAttackGlowWindow != nullptr) {
        m_radarAttackGlowWindow->Win_Enable(true);
    }

    m_triggerRadarAttackGlow = false;
    m_radarAttackGlowCounter = 0;
    m_displayedConstructPercent = -1.0f;
    m_oclTimerFrame = 0;
    m_isObserver = false;
    m_observerPlayer = nullptr;

    if (m_controlBarPopupDescriptionLayout != nullptr) {
        m_controlBarPopupDescriptionLayout->Hide(true);
    }

    m_buildTooltipLayoutVisible = false;

    if (m_controlBarAnimateWindowManager != nullptr) {
        m_controlBarAnimateWindowManager->Reset();
    }

    if (m_specialPowerShortcutAnimateWindowManager != nullptr) {
        m_specialPowerShortcutAnimateWindowManager->Reset();
    }

    if (m_unkAnimateWindowManager != nullptr) {
        m_unkAnimateWindowManager->Reset();
    }

    if (m_videoManager != nullptr) {
        m_videoManager->Reset();
    }

    Switch_To_Context(CB_CONTEXT_NONE, nullptr);
    m_unk1 = false;

    if (m_unkWindow != nullptr) {
        g_theWindowManager->Win_Destroy(m_unkWindow);
        m_unkWindow = nullptr;
    }

    CommandSet *next_set;

    for (CommandSet *set = m_commandSets; set != nullptr; set = next_set) {
        bool is_head = false;
        next_set = set->Get_Next_Command_Set();

        if (set == m_commandSets) {
            is_head = true;
        }

        if (!set->Delete_Overrides() && is_head) {
            m_commandSets = next_set;
        }
    }

    CommandButton *next_button;

    for (CommandButton *button = m_commandButtons; button != nullptr; button = next_button) {
        bool is_head = false;
        next_button = button->Get_Next();

        if (button == m_commandButtons) {
            is_head = true;
        }

        if (!button->Delete_Overrides() && is_head) {
            m_commandButtons = next_button;
        }
    }

    if (g_theTransitionHandler != nullptr) {
        g_theTransitionHandler->Remove("ControlBarArrow", false);
    }

    m_genArrowImage = nullptr;
    m_sciencePurchasePoints = -1;
    m_starHilight = true;
}

void ControlBar::Update()
{
    Get_Star_Image();
    Update_Radar_Attack_Glow();

    if (m_controlBarSchemeManager != nullptr) {
        m_controlBarSchemeManager->Update();
    }

    if (m_videoManager != nullptr) {
        m_videoManager->Update();
    }

    if (m_controlBarAnimateWindowManager != nullptr) {
        m_controlBarAnimateWindowManager->Update();
    }

    if (m_controlBarAnimateWindowManager != nullptr) {
        if (m_controlBarAnimateWindowManager->Is_Finished()) {
            if (m_controlBarAnimateWindowManager->Is_Reversed()) {
                GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
                    nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ControlBarParent"));

                if (window != nullptr) {
                    if (!window->Win_Is_Hidden()) {
                        window->Win_Hide(true);
                    }
                }
            }
        }
    }

    if (m_specialPowerShortcutAnimateWindowManager != nullptr) {
        m_specialPowerShortcutAnimateWindowManager->Update();
    }

    if (m_specialPowerShortcutAnimateWindowManager != nullptr && m_specialPowerShortcutBarParent != nullptr
        && m_specialPowerShortcutAnimateWindowManager->Is_Finished()
        && m_specialPowerShortcutAnimateWindowManager->Is_Reversed() && m_specialPowerShortcutBarParent != nullptr
        && !m_specialPowerShortcutBarParent->Win_Is_Hidden()) {
        m_specialPowerShortcutBarParent->Win_Hide(true);
    }

    if (!m_controlBarPopupDescriptionLayout->Is_Hidden()) {
        m_controlBarPopupDescriptionLayout->Run_Update(nullptr);
        m_buildTooltipLayoutVisible = false;
    }

    Update_Special_Power_Shortcut();

    if (m_isObserver) {
        if (!(g_theGameLogic->Get_Frame() % 15)) {
            Populate_Observer_Info_Window();
        }

        Drawable *drawable = nullptr;

        if (g_theInGameUI->Get_Select_Count() <= 1) {
            drawable = g_theInGameUI->Get_All_Selected_Drawables()->front();
        } else {
            drawable = g_theGameClient->Find_Drawable_By_ID(g_theInGameUI->Get_Solo_Nexus_Selected_Drawable_ID());
        }

        Object *obj;

        if (drawable != nullptr) {
            obj = drawable->Get_Object();
        } else {
            obj = nullptr;
        }

        Set_Portrait_By_Object(obj);
    } else {
        if (m_cameoFlash) {
            for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                GameWindow *window = m_commandWindows[i];

                if (window != nullptr) {
                    CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(window));

                    if (button != nullptr) {
                        if (button->Get_Cameo_Flash_Time() > 0 && !(g_theGameClient->Get_Frame() & 10)) {
                            if (button->Get_Cameo_Flash_Time() % 2) {
                                button->Set_Cameo_Flash_Time(button->Get_Cameo_Flash_Time() - 1);
                                window->Win_Clear_Status(WIN_STATUS_FLASHING);

                                if (button->Get_Cameo_Flash_Time() == 0) {
                                    Set_Cameo_Flash(false);
                                }
                            } else {
                                button->Set_Cameo_Flash_Time(button->Get_Cameo_Flash_Time() - 1);
                                window->Win_Set_Status(WIN_STATUS_FLASHING);
                            }
                        }
                    }
                }
            }
        }

        if (!m_contextParent[1]->Win_Is_Hidden()) {
            Update_Context_Purchase_Science();
        }

        if (m_UIDirty) {
            Evaluate_Context_UI();
            Populate_Special_Power_Shortcut(g_thePlayerList->Get_Local_Player());
            Repopulate_Build_Tooltip_Layout();
        }

        if (g_thePlayerList != nullptr) {
            if (g_thePlayerList->Get_Local_Player() != nullptr) {
                if (g_thePlayerList->Get_Local_Player()->Get_Player_Template() != nullptr) {
                    ThingTemplate *thing = g_theThingFactory->Find_Template(
                        g_thePlayerList->Get_Local_Player()->Get_Player_Template()->Get_Beacon_Name(), true);
                    int count;
                    g_thePlayerList->Get_Local_Player()->Count_Objects_By_Thing_Template(1, &thing, false, &count, true);
                    static const NameKeyType s_beaconPlacementButtonID =
                        g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonPlaceBeacon");
                    GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, s_beaconPlacementButtonID);

                    if (window != nullptr) {
                        if (count >= g_theMultiplayerSettings->Get_Max_Beacons_Per_Player()) {
                            window->Win_Enable(false);
                        } else {
                            window->Win_Enable(true);
                        }
                    }
                }
            }
        }

        if (m_currContext == CB_CONTEXT_MULTI_SELECT) {
            Update_Context_Multi_Select();
        } else if (m_currentSelectedDrawable != nullptr) {
            Object *object = nullptr;

            if (m_currentSelectedDrawable != nullptr) {
                object = m_currentSelectedDrawable->Get_Object();
            }

            if (object != nullptr) {
                switch (m_currContext) {
                    case CB_CONTEXT_COMMAND:
                        Update_Context_Command();
                        break;
                    case CB_CONTEXT_STRUCTURE_INVENTORY:
                        Update_Context_Structure_Inventory();
                        break;
                    case CB_CONTEXT_BEACON:
                        Update_Context_Beacon();
                        break;
                    case CB_CONTEXT_UNDER_CONSTRUCTION:
                        Update_Context_Under_Construction();
                        break;
                    case CB_CONTEXT_OCL_TIMER:
                        Update_Context_OCL_Timer();
                        break;
                    default:
                        return;
                }
            } else {
                Switch_To_Context(CB_CONTEXT_NONE, nullptr);
            }
        } else {
            captainslog_dbgassert(m_currContext == CB_CONTEXT_NONE,
                "ControlBar::Update no selection, but we're not showing the default NONE context");
        }
    }
}

const Image *ControlBar::Get_Star_Image()
{
    if (m_sciencePurchasePoints <= g_thePlayerList->Get_Local_Player()->Get_Science_Purchase_Points()
        && g_thePlayerList->Get_Local_Player()->Get_Science_Purchase_Points() > 0) {
        m_sciencePurchasePoints = g_thePlayerList->Get_Local_Player()->Get_Science_Purchase_Points();
    } else {
        m_starHilight = false;
    }

    GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonGeneral"));

    if (window == nullptr) {
        return nullptr;
    }

    if (m_starHilight) {
        if (g_theGameLogic->Get_Frame() % 30 <= 15) {
            Gadget_Button_Set_Enabled_Image(window, m_generalButtonEnableImage);
        } else {
            Gadget_Button_Set_Enabled_Image(window, m_generalButtonHilitedImage);
        }

        return nullptr;
    } else {
        Gadget_Button_Set_Enabled_Image(window, m_generalButtonEnableImage);
        return nullptr;
    }
}

void ControlBar::Update_Radar_Attack_Glow()
{
    if (m_triggerRadarAttackGlow && m_radarAttackGlowWindow != nullptr) {
        m_radarAttackGlowCounter--;

        if (m_radarAttackGlowCounter > 0) {
            if ((m_radarAttackGlowCounter % 15) == 0) {
                m_radarAttackGlowWindow->Win_Enable((m_radarAttackGlowWindow->Win_Get_Status() & WIN_STATUS_ENABLED) == 0);
            }
        } else {
            m_triggerRadarAttackGlow = false;
            m_radarAttackGlowWindow->Win_Enable(true);
        }
    }
}

void ControlBar::Update_Context_Purchase_Science()
{
    Player *player = g_thePlayerList->Get_Local_Player();
    GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
        m_contextParent[1], g_theNameKeyGenerator->Name_To_Key("GeneralsExpPoints.wnd:ProgressBarExperience"));

    if (window != nullptr) {
        Gadget_Progress_Bar_Set_Progress(window,
            (100 * (player->Get_Current_Skill_Points() - player->Get_Rank_Progress()))
                / (player->Get_Skill_Points_Needed_For_Next_Rank() - player->Get_Rank_Progress()));
    }
}

void ControlBar::Set_Portrait_By_Object(Object *obj)
{
    if (obj != nullptr) {
        if (obj->Is_KindOf(KINDOF_SHOW_PORTRAIT_WHEN_CONTROLLED) && !obj->Is_Locally_Controlled()) {
            Set_Portrait_By_Object(nullptr);
            return;
        }

        const ThingTemplate *thing = obj->Get_Template();
        Player *player = obj->Get_Controlling_Player();
        Drawable *drawable = obj->Get_Drawable();

        if (drawable != nullptr && drawable->Get_Stealth_Look() == STEALTHLOOK_DISGUISED) {
            thing = drawable->Get_Template();

            if (thing->Is_KindOf(KINDOF_SHOW_PORTRAIT_WHEN_CONTROLLED)) {
                Set_Portrait_By_Object(nullptr);
                return;
            }

            StealthUpdate *update = obj->Get_Stealth_Update();

            if (update != nullptr && update->Has_Disguised_Template()) {
                player = g_thePlayerList->Get_Nth_Player(update->Get_Player_Index());
            }
        }

        m_unitSelectedWindow->Win_Hide(false);
        m_rightHUDCameoWindow->Win_Set_Enabled_Image(0, thing->Get_Selected_Portrait_Image());
        Gadget_Button_Draw_Overlay_Image(m_rightHUDCameoWindow, ControlBar::Calculate_Veterancy_Overlay_For_Object(obj));
        m_rightHUDWindow->Win_Clear_Status(WIN_STATUS_IMAGE);
        m_rightHUDCameoWindow->Win_Set_Status(WIN_STATUS_IMAGE);

        for (int i = 0; i < UNIT_UPGRADE_WINDOW_COUNT; i++) {
            Utf8String name = thing->Get_Upgrade_Cameo_Name(i);
            const UpgradeTemplate *upgrade;

            if (name.Is_Empty() || (upgrade = g_theUpgradeCenter->Find_Upgrade(name)) == 0) {
                m_unitUpgradeWindows[i]->Win_Hide(true);
            } else {
                m_unitUpgradeWindows[i]->Win_Hide(false);
                m_unitUpgradeWindows[i]->Win_Set_Enabled_Image(0, upgrade->Get_Button_Image());

                if (obj->Has_Upgrade(upgrade) || (player != nullptr && player->Has_Upgrade_Complete(upgrade))) {
                    m_unitUpgradeWindows[i]->Win_Enable(true);
                } else {
                    m_unitUpgradeWindows[i]->Win_Enable(false);
                }
            }
        }
    } else {
        m_unitSelectedWindow->Win_Hide(true);
        m_rightHUDWindow->Win_Set_Status(WIN_STATUS_IMAGE);
        m_rightHUDCameoWindow->Win_Clear_Status(WIN_STATUS_IMAGE);

        for (int i = 0; i < UNIT_UPGRADE_WINDOW_COUNT; i++) {
            m_unitUpgradeWindows[i]->Win_Hide(true);
        }

        Gadget_Button_Draw_Overlay_Image(m_rightHUDCameoWindow, nullptr);
    }
}

void ControlBar::Hide_Special_Power_Shortcut()
{
    if (m_specialPowerShortcutBarParent != nullptr) {
        m_specialPowerShortcutBarParent->Win_Hide(true);
    }
}

void ControlBar::Mark_UI_Dirty()
{
    m_UIDirty = true;
#ifdef GAME_DEBUG_STRUCTS
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (frame != m_uiDirtyFrame) {
        if (frame == m_uiDirtyFrame + 1) {
            m_uiDirtyCounter++;
        } else {
            m_uiDirtyCounter = 1;
        }
    }

    m_uiDirtyFrame = frame;
    captainslog_dbgassert(m_uiDirtyCounter > 20,
        "Serious flaw in interface system! Either new code or INI has caused the interface to be marked dirty every frame. "
        "This problem actually causes the interface to completely lockup not allowing you to click normal game buttons.");
#endif
}

void ControlBar::On_Player_Rank_Changed(const Player *player)
{
    if (player->Is_Local_Player()) {
        Player *local_player = g_thePlayerList->Get_Local_Player();

        if (m_sciencePurchasePoints <= local_player->Get_Science_Purchase_Points() && g_theTransitionHandler != nullptr
            && g_theInGameUI->Get_Input_Enabled()) {
            g_theTransitionHandler->Set_Group("ControlBarArrow", false);
        }

        m_starHilight = true;
        Mark_UI_Dirty();
    }
}

void ControlBar::On_Player_Science_Purchase_Points_Changed(const Player *player)
{
    if (player->Is_Local_Player()) {
        Player *local_player = g_thePlayerList->Get_Local_Player();

        if (m_sciencePurchasePoints <= local_player->Get_Science_Purchase_Points() && g_theTransitionHandler != nullptr
            && g_theInGameUI->Get_Input_Enabled()) {
            g_theTransitionHandler->Set_Group("ControlBarArrow", false);
        }

        m_starHilight = true;
        Mark_UI_Dirty();
    }
}

void ControlBar::Hide_Communicator(bool hide)
{
    if (m_popupCommunicator != nullptr) {
        m_popupCommunicator->Win_Hide(hide);
    }
}

void ControlBar::Parse_Command_Button_Definition(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    CommandButton *button = g_theControlBar->Find_Non_Const_Command_Button(name);

    if (button != nullptr) {
        captainslog_relassert(ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES,
            CODE_06,
            "[LINE: %d - FILE: '%s'] Duplicate commandbutton '%s' found!",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str(),
            name.Str());
        button = g_theControlBar->New_Command_Button_Override(button);
    } else {
        button = g_theControlBar->New_Command_Button(name);

        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            button->Set_Is_Allocated();
        }
    }

    ini->Init_From_INI(button, CommandButton::Get_Field_Parse());
    const SpecialPowerTemplate *power = button->Get_Special_Power();
    bool needs_power = (button->Get_Options() & COMMAND_OPTION_NEED_SPECIAL_POWER_SCIENCE) != 0;

    if (power == nullptr || needs_power) {
        captainslog_dbgassert(power != nullptr || !needs_power,
            "[LINE: %d in '%s'] CommandButton %s has Options = NEED_SPECIAL_POWER_SCIENCE but doesn't specify a "
            "SpecialPower = xxxx. Please evaluate INI.",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str(),
            name.Str());
    } else {
        captainslog_dbgassert(false,
            "[LINE: %d in '%s'] CommandButton %s has SpecialPower = %s but the button also requires Options = "
            "NEED_SPECIAL_POWER_SCIENCE. Failure to do so will cause bugs such as invisible side shortcut buttons",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str(),
            name.Str(),
            power->Get_Name().Str());
    }
}

void ControlBar::Parse_Command_Set_Definition(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    CommandSet *commandset = g_theControlBar->Find_Non_Const_Command_Set(name);

    if (commandset != nullptr) {
        captainslog_relassert(ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES,
            CODE_06,
            "[LINE: %d - FILE: '%s'] Duplicate commandset '%s' found!",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str(),
            name.Str());
        commandset = g_theControlBar->New_Command_Set_Override(commandset);
    } else {
        commandset = g_theControlBar->New_Command_Set(name);

        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            commandset->Set_Is_Allocated();
        }
    }

    captainslog_dbgassert(commandset != nullptr, "Parse_Command_Set_Definition: Unable to allocate set '%s'", name.Str());
    ini->Init_From_INI(commandset, CommandSet::Get_Field_Parse());
}

CommandButton *ControlBar::Find_Non_Const_Command_Button(const Utf8String &name)
{
    for (CommandButton *button = m_commandButtons; button != nullptr; button = button->Get_Next()) {
        if (button->Get_Name() == name) {
            return static_cast<CommandButton *>(button->Friend_Get_Final_Override());
        }
    }

    return nullptr;
}

CommandSet *ControlBar::Find_Non_Const_Command_Set(const Utf8String &name)
{
    for (CommandSet *set = m_commandSets; set != nullptr; set = set->Get_Next_Command_Set()) {
        if (set->Get_Name() == name) {
            return set;
        }
    }

    return nullptr;
}

const CommandSet *ControlBar::Find_Command_Set(const Utf8String &name)
{
    CommandSet *set = Find_Non_Const_Command_Set(name);

    if (set != nullptr) {
        return static_cast<const CommandSet *>(set->Friend_Get_Final_Override());
    }

    return nullptr;
}

const CommandButton *ControlBar::Find_Command_Button(const Utf8String &name)
{
    CommandButton *button = Find_Non_Const_Command_Button(name);

    if (button != nullptr) {
        return static_cast<const CommandButton *>(button->Friend_Get_Final_Override());
    }

    return nullptr;
}

CommandButton *ControlBar::New_Command_Button(const Utf8String &name)
{
    CommandButton *new_button = new CommandButton();
    new_button->Set_Name(name);
    new_button->Friend_Add_To_List(&m_commandButtons);
    return new_button;
}

CommandButton *ControlBar::New_Command_Button_Override(CommandButton *button)
{
    if (button == nullptr) {
        return nullptr;
    }

    CommandButton *new_button = new CommandButton();
    *new_button = *button;
    new_button->Set_Is_Allocated();
    button->Set_Next(new_button);
    return new_button;
}

CommandSet *ControlBar::New_Command_Set(const Utf8String &name)
{
    CommandSet *new_set = new CommandSet(name);
    new_set->Friend_Add_To_List(&m_commandSets);
    return new_set;
}

CommandSet *ControlBar::New_Command_Set_Override(CommandSet *set)
{
    if (set == nullptr) {
        return nullptr;
    }

    CommandSet *new_set = new CommandSet(set->Get_Name());
    *new_set = *set;
    new_set->Set_Is_Allocated();
    set->Set_Next(new_set);
    return new_set;
}

void ControlBar::Set_Control_Bar_Scheme_By_Player(Player *p)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00460340, 0x0072F421), this, p);
#endif
}

void ControlBar::Set_Control_Bar_Scheme_By_Player_Template(PlayerTemplate *tmplate)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, PlayerTemplate *>(PICK_ADDRESS(0x004606B0, 0x0072F642), this, tmplate);
#endif
}

void ControlBar::Init_Special_Power_Shortcut_Bar(Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00461680, 0x0073027A), this, player);
#endif
}

void ControlBar::Switch_To_Context(ControlBarContext context, Drawable *draw)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, ControlBarContext, Drawable *>(PICK_ADDRESS(0x0045F8C0, 0x0072E3D0), this, context, draw);
#endif
}

void ControlBar::Evaluate_Context_UI()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x0045EC00, 0x0072D9A7), this);
#endif
}

void ControlBar::Update_Special_Power_Shortcut()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x00462230, 0x00731139), this);
#endif
}

void ControlBar::Populate_Special_Power_Shortcut(Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00461B10, 0x0073067E), this, player);
#endif
}
