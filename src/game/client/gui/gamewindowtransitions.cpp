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
#include "gamewindowmanager.h"
#include "ini.h"

#ifndef GAME_DLL
GameWindowTransitionsHandler *g_theTransitionHandler;
#endif

Transition *Get_Transition_For_Style(int style)
{
#ifdef GAME_DLL
    return Call_Function<Transition *, int>(PICK_ADDRESS(0x00514720, 0x008F08C3), style);
#else
    return nullptr;
#endif
}

TransitionWindow::TransitionWindow() :
    m_frameDelay(0),
    m_style(TRANSITION_FLASH),
    m_winNameKey(NAMEKEY_INVALID),
    m_window(nullptr),
    m_transition(nullptr),
    m_startFrame(0)
{
}

TransitionWindow::~TransitionWindow()
{
    m_window = nullptr;

    if (m_transition != nullptr) {
        delete m_transition;
    }

    m_transition = nullptr;
}

bool TransitionWindow::Init()
{
    m_winNameKey = g_theNameKeyGenerator->Name_To_Key(m_winName.Str());
    m_window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, m_winNameKey);
    m_startFrame = m_frameDelay;

    if (m_transition != nullptr) {
        delete m_transition;
    }

    m_transition = Get_Transition_For_Style(m_style);
    m_transition->Init(m_window);

    return true;
}

void TransitionWindow::Update(int frame)
{
    if (frame >= m_startFrame && frame <= m_transition->Get_Max_Frames() + m_startFrame) {
        if (m_transition != nullptr) {
            m_transition->Update(frame - m_startFrame);
        }
    }
}

bool TransitionWindow::Is_Finished()
{
    if (m_transition != nullptr) {
        return m_transition->Is_Finished();
    } else {
        return true;
    }
}

void TransitionWindow::Reverse()
{
    if (m_transition != nullptr) {
        m_transition->Reverse();
    }
}

void TransitionWindow::Skip()
{
    if (m_transition != nullptr) {
        m_transition->Skip();
    }
}

void TransitionWindow::Draw()
{
    if (m_transition != nullptr) {
        m_transition->Draw();
    }
}

int TransitionWindow::Get_Total_Frames()
{
    if (m_transition != nullptr) {
        return m_frameDelay + m_transition->Get_Max_Frames();
    } else {
        return m_frameDelay;
    }
}

TransitionGroup::TransitionGroup() : m_fireOnce(false), m_currentFrame(0) {}

TransitionGroup::~TransitionGroup()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        TransitionWindow *window = *it;

        if (window != nullptr) {
            delete window;
        }

        it = m_windows.erase(it);
    }
}

void TransitionGroup::Init()
{
    m_currentFrame = 0;
    m_frameAdjust = 1;

    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        (*it)->Init();
    }
}

void TransitionGroup::Update()
{
    m_currentFrame += m_frameAdjust;

    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        (*it)->Update(m_currentFrame);
    }
}

bool TransitionGroup::Is_Finished()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        if (!(*it)->Is_Finished()) {
            return false;
        }
    }

    return true;
}

void TransitionGroup::Reverse()
{
    int max_frames = 0;
    m_frameAdjust = -1;

    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        int total_frames = (*it)->Get_Total_Frames();

        if (total_frames > max_frames) {
            max_frames = total_frames;
        }
    }

    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        (*it)->Reverse();
    }

    m_currentFrame = max_frames;
}

bool TransitionGroup::Is_Reversed()
{
    return m_frameAdjust < 0;
}

void TransitionGroup::Skip()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        (*it)->Skip();
    }
}

void TransitionGroup::Draw()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); it++) {
        (*it)->Draw();
    }
}

void TransitionGroup::Add_Window(TransitionWindow *window)
{
    if (window != nullptr) {
        m_windows.push_back(window);
    }
}

// clang-format off
FieldParse GameWindowTransitionsHandler::s_gameWindowTransitionsFieldParseTable[] = { 
    { "Window", &GameWindowTransitionsHandler::Parse_Window, nullptr, 0 },
    { "FireOnce", &INI::Parse_Bool, nullptr, offsetof(TransitionGroup, m_fireOnce) },
    { nullptr, nullptr, nullptr, 0 }
};
// clang-format on

GameWindowTransitionsHandler::GameWindowTransitionsHandler() :
    m_group1(nullptr), m_group2(nullptr), m_group3(nullptr), m_group4(nullptr)
{
}

GameWindowTransitionsHandler::~GameWindowTransitionsHandler()
{
    m_group1 = nullptr;
    m_group2 = nullptr;
    m_group3 = nullptr;
    m_group4 = nullptr;

    for (auto it = m_groupList.begin(); it != m_groupList.end(); it++) {
        TransitionGroup *group = *it;

        if (group != nullptr) {
            delete group;
        }

        it = m_groupList.erase(it);
    }
}

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
    if (m_group3 == m_group1) {
        m_group4 = nullptr;
    } else {
        m_group4 = m_group3;
    }

    m_group3 = m_group1;

    if (m_group1 != nullptr && !m_group1->Is_Finished()) {
        m_group1->Update();
    }

    if (m_group1 != nullptr && m_group1->Is_Finished() && m_group1->Fire_Once()) {
        m_group1 = nullptr;
    }

    if (m_group1 != nullptr && m_group2 != nullptr && m_group1->Is_Finished()) {
        m_group1 = m_group2;
        m_group2 = nullptr;
    }

    if (m_group1 == nullptr && m_group2 != nullptr) {
        m_group1 = m_group2;
        m_group2 = nullptr;
    }

    if (m_group1 != nullptr && m_group1->Is_Finished() && m_group1->Is_Reversed()) {
        m_group1 = nullptr;
    }
}

void GameWindowTransitionsHandler::Draw()
{
    if (m_group3 != nullptr) {
        m_group3->Draw();
    }

    if (m_group4 != nullptr) {
        m_group4->Draw();
    }
}

void GameWindowTransitionsHandler::Set_Group(Utf8String name, bool skip)
{
    if (name.Is_Empty() && skip) {
        m_group1 = nullptr;
    }

    if (skip && m_group1 != nullptr) {
        m_group1->Skip();
        m_group1 = Find_Group(name);

        if (m_group1 != nullptr) {
            m_group1->Init();
        }
    } else if (m_group1 != nullptr) {
        if (!m_group1->Fire_Once() && !m_group1->Is_Reversed()) {
            m_group1->Reverse();
        }

        m_group2 = Find_Group(name);

        if (m_group2 != nullptr) {
            m_group2->Init();
        }
    } else {
        m_group1 = Find_Group(name);

        if (m_group1 != nullptr) {
            m_group1->Init();
        }
    }
}

bool GameWindowTransitionsHandler::Is_Finished()
{
    if (m_group1 != nullptr) {
        return m_group1->Is_Finished();
    } else {
        return true;
    }
}

void GameWindowTransitionsHandler::Reverse(Utf8String name)
{
    TransitionGroup *group = Find_Group(name);

    if (m_group1 == group) {
        m_group1->Reverse();
    } else if (m_group2 == group) {
        m_group2 = nullptr;
    } else {
        if (m_group1 != nullptr) {
            m_group1->Skip();
        }

        if (m_group2 != nullptr) {
            m_group2->Skip();
        }

        m_group1 = group;
        m_group1->Init();
        m_group1->Skip();
        m_group1->Reverse();
        m_group2 = nullptr;
    }
}

void GameWindowTransitionsHandler::Remove(Utf8String name, bool skip)
{
    TransitionGroup *group = Find_Group(name);

    if (m_group2 == group) {
        if (skip) {
            m_group2->Skip();
        }

        m_group2 = nullptr;
    }

    if (m_group1 == group) {
        if (skip) {
            m_group1->Skip();
        }

        m_group1 = m_group2;
    }
}

TransitionGroup *GameWindowTransitionsHandler::Get_New_Group(Utf8String name)
{
    if (name.Is_Empty()) {
        return nullptr;
    } else {
        if (Find_Group(name) != nullptr) {
            captainslog_dbgassert(
                false, "GameWindowTransitionsHandler::Get_New_Group - We already have a group %s", name.Str());
            return nullptr;
        } else {
            TransitionGroup *group = new TransitionGroup();
            group->Set_Name(name);
            m_groupList.push_back(group);
            return group;
        }
    }
}

TransitionGroup *GameWindowTransitionsHandler::Find_Group(Utf8String name)
{
    if (name.Is_Empty()) {
        return nullptr;
    } else {
        for (auto it = m_groupList.begin(); it != m_groupList.end(); it++) {
            TransitionGroup *group = *it;

            if (name.Compare_No_Case(group->Get_Name()) == 0) {
                return group;
            }
        }

        return nullptr;
    }
}

void GameWindowTransitionsHandler::Load()
{
    INI ini;
    ini.Load("Data\\INI\\WindowTransitions.ini", INI_LOAD_OVERWRITE, nullptr);
}

static LookupListRec s_transitionStyleNames[] = { "FLASH",
    TRANSITION_FLASH,
    "BUTTONFLASH",
    TRANSITION_BUTTON_FLASH,
    "WINFADE",
    TRANSITION_FADE,
    "WINSCALEUP",
    TRANSITION_SCALE_UP,
    "MAINMENUSCALEUP",
    TRANSITION_MAIN_MENU_SCALE_UP,
    "TYPETEXT",
    TRANSITION_TYPE_TEXT,
    "SCREENFADE",
    TRANSITION_SCREEN_FADE,
    "COUNTUP",
    TRANSITION_COUNT_UP,
    "FULLFADE",
    TRANSITION_FULL_FADE,
    "TEXTONFRAME",
    TRANSITION_TEXT_ON_FRAME,
    "MAINMENUMEDIUMSCALEUP",
    TRANSITION_MAIN_MENU_MEDIUM_SCALE_UP,
    "MAINMENUSMALLSCALEDOWN",
    TRANSITION_MAIN_MENU_SMALL_SCALE_DOWN,
    "CONTROLBARARROW",
    TRANSITION_CONTROL_BAR_ARROW,
    "SCORESCALEUP",
    TRANSITION_SCORE_SCALE_UP,
    "REVERSESOUND",
    TRANSITION_REVERSE_SOUND,
    nullptr,
    0 };

void GameWindowTransitionsHandler::Parse_Window(INI *ini, void *formal, void *store, const void *user_data)
{
    // clang-format off
    static FieldParse s_fieldParse[] = { 
        { "WinName", &INI::Parse_AsciiString, nullptr, offsetof(TransitionWindow, m_winName) },
        { "Style", &INI::Parse_Lookup_List, s_transitionStyleNames, offsetof(TransitionWindow, m_style) },
        { "FrameDelay", &INI::Parse_Int, nullptr, offsetof(TransitionWindow, m_frameDelay) },
        { nullptr, nullptr, nullptr, 0 }
    };
    // clang-format on

    TransitionGroup *group = static_cast<TransitionGroup *>(formal);
    TransitionWindow *window = new TransitionWindow();
    ini->Init_From_INI(window, s_fieldParse);
    group->Add_Window(window);
}

void GameWindowTransitionsHandler::Parse_Window_Transitions(INI *ini)
{
    Utf8String str;
    str.Set(ini->Get_Next_Token());
    captainslog_dbgassert(
        g_theTransitionHandler != nullptr, "Parse_Window_Transitions: TheTransitionHandler doesn't exist yet");

    if (g_theTransitionHandler != nullptr) {
        TransitionGroup *group = g_theTransitionHandler->Get_New_Group(str);
        captainslog_dbgassert(group != nullptr, "Parse_Window_Transitions: Unable to allocate group '%s'", str.Str());
        ini->Init_From_INI(group, Get_Field_Parse());
    }
}
