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
#include "behaviormodule.h"
#include "captainslog.h"
#include "gamelogic.h"
#include "image.h"
#include "player.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ControlBar *g_theControlBar;
#endif

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
    captainslog_dbgassert(index >= MAX_COMMAND_BUTTONS, "Parse_Command_Button: button index '%d' out of range", index);
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
    { "Science", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(CommandButton, m_maxShotsToFire) },
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
    bool is_dirty = true;

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

void CommandButton::Set_Next(CommandButton **next)
{
    m_nextCommandButton = *next;
    *next = this;
}

void ControlBar::Mark_UI_Dirty()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x0045B3F0, 0x00729C50), this);
#endif
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

// On_Player_Rank_Changed and On_Player_Science_Purchase_Points_Changed have identical addresses in game exe (because the
// code is identical)
void ControlBar::On_Player_Rank_Changed(const Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, const Player *>(PICK_ADDRESS(0x0045EB90, 0x001055D6), this, player);
#endif
}

void ControlBar::On_Player_Science_Purchase_Points_Changed(const Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, const Player *>(PICK_ADDRESS(0x0045EB90, 0x001056F2), this, player);
#endif
}

const CommandSet *ControlBar::Find_Command_Set(const Utf8String &name)
{
#ifdef GAME_DLL
    return Call_Method<const CommandSet *, ControlBar, const Utf8String &>(PICK_ADDRESS(0x0045F770, 0x0072E236), this, name);
#else
    return nullptr;
#endif
}
const CommandButton *ControlBar::Find_Command_Button(const Utf8String &name)
{
#ifdef GAME_DLL
    return Call_Method<const CommandButton *, ControlBar, const Utf8String &>(
        PICK_ADDRESS(0x0045F6D0, 0x0072E204), this, name);
#else
    return nullptr;
#endif
}

void ControlBar::Hide_Communicator(bool hide)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, bool>(PICK_ADDRESS(0x00460AB0, 0x0072FA8D), this, hide);
#endif
}

void ControlBar::Init_Special_Power_Shortcut_Bar(Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00461680, 0x0073027A), this, player);
#endif
}
