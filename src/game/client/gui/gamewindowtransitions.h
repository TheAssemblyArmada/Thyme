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
#pragma once
#include "always.h"
#include "namekeygenerator.h"
#include "subsysteminterface.h"
#include <list>

enum WindowTransitionStyle
{
    TRANSITION_FLASH,
    TRANSITION_BUTTON_FLASH,
    TRANSITION_FADE,
    TRANSITION_SCALE_UP,
    TRANSITION_MAIN_MENU_SCALE_UP,
    TRANSITION_TYPE_TEXT,
    TRANSITION_SCREEN_FADE,
    TRANSITION_COUNT_UP,
    TRANSITION_FULL_FADE,
    TRANSITION_TEXT_ON_FRAME,
    TRANSITION_MAIN_MENU_MEDIUM_SCALE_UP,
    TRANSITION_MAIN_MENU_SMALL_SCALE_DOWN,
    TRANSITION_CONTROL_BAR_ARROW,
    TRANSITION_SCORE_SCALE_UP,
    TRANSITION_REVERSE_SOUND,
};

class TransitionGroup;

class GameWindowTransitionsHandler : public SubsystemInterface
{
public:
    GameWindowTransitionsHandler();
    ~GameWindowTransitionsHandler() override;

    void Init() override;
    void Reset() override;
    void Update() override;
    void Draw() override;

    void Set_Group(Utf8String str, bool b);
    bool Is_Finished();
    void Reverse(Utf8String str);

protected:
    std::list<TransitionGroup *> m_groupList;
    TransitionGroup *m_group1;
    TransitionGroup *m_group2;
    TransitionGroup *m_group3;
    TransitionGroup *m_group4;
};

#ifdef GAME_DLL
extern GameWindowTransitionsHandler *&g_theTransitionHandler;
#else
extern GameWindowTransitionsHandler *g_theTransitionHandler;
#endif
