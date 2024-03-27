/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Skirmish Battle Honors
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
#include "gamewindow.h"
#include "userpreferences.h"

class SkirmishBattleHonors : public UserPreferences
{
public:
    SkirmishBattleHonors();
    virtual ~SkirmishBattleHonors() override;
    int Get_Endurance_Medal(Utf8String name, int level) const;
    int Get_Wins() const;
    int Get_Win_Streak() const;
    int Get_Best_Win_Streak() const;
    int Get_Losses() const;
    int Get_China_Campaign_Complete(int level) const;
    int Get_USA_Campaign_Complete(int level) const;
    int Get_GLA_Campaign_Complete(int level) const;
    int Get_Challenge_Campaign_Complete(int general, int level) const;
    int Get_Honors() const;
};

void Reset_Battle_Honor_Insertion();
void Insert_Battle_Honor(
    GameWindow *window, Image const *image, bool earned, int tooltip, int *row, int *column, Utf16String str, int score);
void Battle_Honor_Tooltip(GameWindow *window, WinInstanceData *instance, unsigned int mouse);
void Populate_Skirmish_Battle_Honors();

#ifdef GAME_DLL
extern int &g_rowsToSkip;
#else
extern int g_rowsToSkip;
#endif
