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
#include "gamelogic.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ControlBar *g_theControlBar;
#endif

void ControlBar::Mark_UI_Dirty()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x0045B3F0, 0x00729C50), this);
#endif
}

void ControlBar::Set_Control_Bar_Scheme_By_Player(Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00460340, 0x0072F421), this, player);
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

    if (r) {
        if (r == ALLIES) {
            options = 4;
        } else if (r == NEUTRAL) {
            options = 2;
        }
    } else {
        options = 1;
    }

    return (options & m_options) != 0;
}

const CommandButton *CommandSet::Get_Command_Button(int button) const
{
    const CommandButton *c;
    if (g_theGameLogic != nullptr && g_theGameLogic->Find_Control_Bar_Override(m_name, button, c)) {
        return c;
    }

    return m_command[button];
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
