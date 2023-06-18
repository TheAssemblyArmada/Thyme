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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "audioeventrts.h"
#include "ingameui.h"

class Player;
class PlayerTemplate;

class CommandSet : public Overridable
{
    IMPLEMENT_POOL(CommandSet)

public:
    enum
    {
        MAX_COMMAND_BUTTONS = 18,
    };

    virtual ~CommandSet() override;
    const CommandButton *Get_Command_Button(int button) const;

private:
    Utf8String m_name;
    const CommandButton *m_command[MAX_COMMAND_BUTTONS];
    CommandSet *m_nextCommandSet;
};

enum GUICommand
{
    GUI_COMMAND_NONE,
    GUI_COMMAND_DOZER_CONSTRUCT,
    GUI_COMMAND_DOZER_CONSTRUCT_CANCEL,
    GUI_COMMAND_UNIT_BUILD,
    GUI_COMMAND_CANCEL_UNIT_BUILD,
    GUI_COMMAND_PLAYER_UPGRADE,
    GUI_COMMAND_OBJECT_UPGRADE,
    GUI_COMMAND_CANCEL_UPGRADE,
    GUI_COMMAND_ATTACK_MOVE,
    GUI_COMMAND_GUARD,
    GUI_COMMAND_GUARD_WITHOUT_PURSUIT,
    GUI_COMMAND_GUARD_FLYING_UNITS_ONLY,
    GUI_COMMAND_STOP,
    GUI_COMMAND_WAYPOINTS,
    GUI_COMMAND_EXIT_CONTAINER,
    GUI_COMMAND_EVACUATE,
    GUI_COMMAND_EXECUTE_RAILED_TRANSPORT,
    GUI_COMMAND_BEACON_DELETE,
    GUI_COMMAND_SET_RALLY_POINT,
    GUI_COMMAND_SELL,
    GUI_COMMAND_FIRE_WEAPON,
    GUI_COMMAND_SPECIAL_POWER,
    GUI_COMMAND_PURCHASE_SCIENCE,
    GUI_COMMAND_HACK_INTERNET,
    GUI_COMMAND_TOGGLE_OVERCHARGE,
    GUI_COMMAND_COMBATDROP,
    GUI_COMMAND_SWITCH_WEAPON,
    GUI_COMMAND_HIJACK_VEHICLE,
    GUI_COMMAND_CONVERT_TO_CARBOMB,
    GUI_COMMAND_SABOTAGE_BUILDING,
    GUI_COMMAND_PLACE_BEACON,
    GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT,
    GUI_COMMAND_SPECIAL_POWER_CONSTRUCT,
    GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT,
    GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE,
};

enum CommandButtonMappedBorderType
{
    COMMAND_BUTTON_BORDER_NONE,
    COMMAND_BUTTON_BORDER_BUILD,
    COMMAND_BUTTON_BORDER_UPGRADE,
    COMMAND_BUTTON_BORDER_ACTION,
    COMMAND_BUTTON_BORDER_SYSTEM,
    COMMAND_BUTTON_BORDER_COUNT,
};

class CommandButton : public Overridable
{
    IMPLEMENT_POOL(CommandButton)

public:
    virtual ~CommandButton() override;

    GUICommand Get_Command() const { return m_command; }
    CommandButton *Get_Next() const { return m_nextCommandButton; }
    const SpecialPowerTemplate *Get_Special_Power() const { return m_specialPower; }
    unsigned int Get_Options() const { return m_options; }
    WeaponSlotType Get_Weapon_Slot() const { return m_weaponSlot; }
    int Get_Max_Shots_To_Fire() const { return m_maxShotsToFire; }
    const Utf8String &Get_Name() const { return m_name; }
    const UpgradeTemplate *Get_Upgrade_Template() const { return m_upgradeTemplate; }
    Override<ThingTemplate> Get_Template() const { return m_thingTemplate; }

    bool Is_Valid_Object_Target(const Object *obj1, const Object *obj2) const;
    bool Is_Valid_Relationship_Target(Relationship r) const;

private:
    Utf8String m_name;
    GUICommand m_command;
    CommandButton *m_nextCommandButton;
    unsigned int m_options;
    ThingTemplate *m_thingTemplate;
    UpgradeTemplate *m_upgradeTemplate;
    SpecialPowerTemplate *m_specialPower;
    RadiusCursorType m_radiusCursorType;
    Utf8String m_cursorName;
    Utf8String m_invalidCursorName;
    Utf8String m_textLabel;
    Utf8String m_descriptLabel;
    Utf8String m_purchasedLabel;
    Utf8String m_conflictingLabel;
    WeaponSlotType m_weaponSlot;
    int m_maxShotsToFire;
    std::vector<ScienceType> m_sciences;
    CommandButtonMappedBorderType m_commandButtonBorder;
    Utf8String m_buttonImageString;
    int m_unk;
    AudioEventRTS m_unitSpecificSound;
    Image *m_buttonImage;
    int m_cameoFlashTime;
};

class ControlBar
{
public:
    void Mark_UI_Dirty();
    void Set_Control_Bar_Scheme_By_Player(Player *player);
    void Set_Control_Bar_Scheme_By_Player_Template(PlayerTemplate *tmplate);
    void On_Player_Rank_Changed(const Player *player);
    void On_Player_Science_Purchase_Points_Changed(const Player *player);
    const CommandSet *Find_Command_Set(const Utf8String &name);
    const CommandButton *Find_Command_Button(const Utf8String &name);
    void Hide_Communicator(bool hide);
    void Init_Special_Power_Shortcut_Bar(Player *player);
};

#ifdef GAME_DLL
extern ControlBar *&g_theControlBar;
#else
extern ControlBar *g_theControlBar;
#endif
