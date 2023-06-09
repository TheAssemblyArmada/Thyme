/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Victory Conditions
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
#include "subsysteminterface.h"

class Player;

class VictoryConditionsInterface : public SubsystemInterface
{
public:
    virtual ~VictoryConditionsInterface() override;
    virtual bool Has_Achieved_Victory(Player *player) = 0;
    virtual bool Has_Been_Defeated(Player *player) = 0;
    virtual bool Has_Single_Player_Been_Defeated(Player *player) = 0;
    virtual void Cache_Player_Ptrs() = 0;
    virtual bool Is_Local_Allied_Victory() = 0;
    virtual bool Is_Local_Allied_Defeat() = 0;
    virtual bool Is_Local_Defeat() = 0;
    virtual bool Am_I_Observer() = 0;
    virtual unsigned int Get_End_Frame() = 0;

    void Set_Victory_Conditions(int conditions) { m_victoryConditions = conditions; }

private:
    int m_victoryConditions;
};

VictoryConditionsInterface *Create_Victory_Conditions();

#ifdef GAME_DLL
extern VictoryConditionsInterface *&g_theVictoryConditions;
#else
extern VictoryConditionsInterface *g_theVictoryConditions;
#endif
