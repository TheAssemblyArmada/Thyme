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

class GameWindow;

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

class Transition
{
public:
    Transition();
    virtual ~Transition();
    virtual void Init(GameWindow *window) = 0;
    virtual void Update(int frame) = 0;
    virtual void Reverse() = 0;
    virtual void Draw() = 0;
    virtual void Skip() = 0;

    int Get_Max_Frames() const { return m_maxFrames; }
    bool Is_Finished() const { return m_isFinished; }

private:
    int m_maxFrames;
    bool m_isFinished;
    bool m_unk1;
};

class TransitionWindow
{
public:
    TransitionWindow();
    ~TransitionWindow();
    bool Init();
    void Update(int frame);
    bool Is_Finished();
    void Reverse();
    void Skip();
    void Draw();
    int Get_Total_Frames();

private:
    Utf8String m_winName;
    int m_frameDelay;
    WindowTransitionStyle m_style;
    NameKeyType m_winNameKey;
    GameWindow *m_window;
    Transition *m_transition;
    int m_startFrame;
    friend class GameWindowTransitionsHandler;
};

class TransitionGroup
{
public:
    TransitionGroup();
    ~TransitionGroup();
    void Init();
    void Update();
    bool Is_Finished();
    void Reverse();
    bool Is_Reversed();
    void Skip();
    void Draw();
    void Add_Window(TransitionWindow *window);

    bool Fire_Once() const { return m_fireOnce; }
    void Set_Name(Utf8String name) { m_name = name; }
    Utf8String Get_Name() const { return m_name; }

private:
    bool m_fireOnce;
    std::list<TransitionWindow *> m_windows;
    int m_frameAdjust;
    int m_currentFrame;
    Utf8String m_name;
    friend class GameWindowTransitionsHandler;
};

class GameWindowTransitionsHandler : public SubsystemInterface
{
public:
    GameWindowTransitionsHandler();
    virtual ~GameWindowTransitionsHandler();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override;

    void Load();
    void Set_Group(Utf8String name, bool skip);
    bool Is_Finished();
    void Reverse(Utf8String name);
    void Remove(Utf8String name, bool skip);
    TransitionGroup *Get_New_Group(Utf8String name);
    TransitionGroup *Find_Group(Utf8String name);

    static void Parse_Window(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Window_Transitions(INI *ini);

    static FieldParse *Get_Field_Parse() { return s_gameWindowTransitionsFieldParseTable; }

protected:
    std::list<TransitionGroup *> m_groupList;
    TransitionGroup *m_group1;
    TransitionGroup *m_group2;
    TransitionGroup *m_group3;
    TransitionGroup *m_group4;

    static FieldParse s_gameWindowTransitionsFieldParseTable[];
};

#ifdef GAME_DLL
extern GameWindowTransitionsHandler *&g_theTransitionHandler;
#else
extern GameWindowTransitionsHandler *g_theTransitionHandler;
#endif
