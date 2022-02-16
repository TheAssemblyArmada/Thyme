/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for handling WND UI system transitions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamewindowtransitions.h"

void GameWindowTransitionsHandler::Init()
{
    // TODO investigate should this clear m_groupList, original doesn't
    m_group1 = nullptr;
    m_group2 = nullptr;
    m_group3 = nullptr;
    m_group4 = nullptr;
}

void GameWindowTransitionsHandler::Reset()
{
    // TODO investigate should this clear m_groupList, original doesn't
    m_group1 = nullptr;
    m_group2 = nullptr;
    m_group3 = nullptr;
    m_group4 = nullptr;
}

void GameWindowTransitionsHandler::Update()
{
#ifdef GAME_DLL
    Call_Method<void, GameWindowTransitionsHandler>(PICK_ADDRESS(0x00514EA0, 0), this);
#endif
}

void GameWindowTransitionsHandler::Draw()
{
#ifdef GAME_DLL
    Call_Method<void, GameWindowTransitionsHandler>(PICK_ADDRESS(0x00515010, 0), this);
#endif
}