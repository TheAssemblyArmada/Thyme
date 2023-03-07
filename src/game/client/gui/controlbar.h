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

class Player;
class PlayerTemplate;

class ControlBar
{
public:
    void Mark_UI_Dirty();
    void Set_Control_Bar_Scheme_By_Player(Player *player);
    void Set_Control_Bar_Scheme_By_Player_Template(PlayerTemplate *tmplate);
    void On_Player_Rank_Changed(const Player *player);
    void On_Player_Science_Purchase_Points_Changed(const Player *player);
};

#ifdef GAME_DLL
extern ControlBar *&g_theControlBar;
#else
extern ControlBar *g_theControlBar;
#endif
