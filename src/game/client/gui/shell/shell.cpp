/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shell
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shell.h"
#include "imemanager.h"
#include "shellmenuscheme.h"
#include "windowlayout.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Shell *g_theShell;
#endif

Shell::Shell() :
    m_screenCount(0),
    m_background(nullptr),
    m_clearBackground(false),
    m_pendingPush(false),
    m_pendingPop(false),
    m_pendingPushName(""),
    m_isShellActive(true),
    m_shellMapOn(false),
    m_saveLoad(nullptr),
    m_popupReplay(nullptr),
    m_optionsLayout(nullptr)
{
    for (int i = 0; i < 16; i++) {
        m_screenStack[i] = nullptr;
    }

    m_animateWindowManager = new AnimateWindowManager();
    m_shellMenuSchemeManager = new ShellMenuSchemeManager();
}

Shell::~Shell()
{
    for (WindowLayout *layout = Top(); layout != nullptr; layout = Top()) {
        Pop_Immediate();
    }

    if (m_background != nullptr) {
        m_background->Destroy_Windows();
        m_background->Delete_Instance();
        m_background = nullptr;
    }

    if (m_animateWindowManager != nullptr) {
        delete m_animateWindowManager;
        m_animateWindowManager = nullptr;
    }

    if (m_shellMenuSchemeManager != nullptr) {
        delete m_shellMenuSchemeManager;
        m_shellMenuSchemeManager = nullptr;
    }

    if (m_saveLoad != nullptr) {
        m_saveLoad->Destroy_Windows();
        m_saveLoad->Delete_Instance();
        m_saveLoad = nullptr;
    }

    if (m_popupReplay != nullptr) {
        m_popupReplay->Destroy_Windows();
        m_popupReplay->Delete_Instance();
        m_popupReplay = nullptr;
    }

    if (m_optionsLayout != nullptr) {
        m_optionsLayout->Destroy_Windows();
        m_optionsLayout->Delete_Instance();
        m_optionsLayout = nullptr;
    }
}

void Shell::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Default\\ShellMenuScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\ShellMenuScheme.ini", INI_LOAD_OVERWRITE, nullptr);

    if (m_shellMenuSchemeManager != nullptr) {
        m_shellMenuSchemeManager->Init();
    }
}

void Shell::Reset()
{
    g_theIMEManager->Detach();

    while (m_screenCount != 0) {
        Pop();
    }

    m_animateWindowManager->Reset();
}

void Shell::Update()
{
#ifdef GAME_DLL
    Call_Method<void, Shell>(PICK_ADDRESS(0x005C4FE0, 0x009DFF2B), this);
#endif
}

void Shell::Push(Utf8String filename, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, Utf8String, bool>(PICK_ADDRESS(0x005C50C0, 0x009E01DB), this, filename, b);
#endif
}

WindowLayout *Shell::Top()
{
    if (m_screenCount != 0) {
        return m_screenStack[m_screenCount - 1];
    } else {
        return nullptr;
    }
}

void Shell::Show_Shell(int i)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, int>(PICK_ADDRESS(0x005C5300, 0x009E0558), this, i);
#endif
}

void Shell::Hide_Shell()
{
#ifdef GAME_DLL
    Call_Method<void, Shell>(PICK_ADDRESS(0x005C54E0, 0x009E08B7), this);
#endif
}

void Shell::Show_Shell_Map(bool use_shell_map)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, bool>(PICK_ADDRESS(0x005C5380, 0x009E06A8), this, use_shell_map);
#endif
}

void Shell::Pop_Immediate()
{
    WindowLayout *layout = Top();

    if (layout != nullptr) {
        captainslog_debug("Shell:Pop_Immediate() - stack was");

        for (int i = 0; i < m_screenCount; i++) {
            captainslog_debug("\t\t%s", m_screenStack[i]->Get_Filename().Str());
        }

        m_pendingPop = false;
        bool b = true;
        layout->Run_Shutdown(&b);
        Do_Pop(false);

        if (g_theIMEManager != nullptr) {
            g_theIMEManager->Detach();
        }
    }
}

void Shell::Do_Pop(bool impending_push)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, bool>(PICK_ADDRESS(0x005C5620, 0x009E0BD1), this, impending_push);
#endif
}

void Shell::Pop()
{
#ifdef GAME_DLL
    Call_Method<void, Shell>(PICK_ADDRESS(0x005C51E0, 0x009E035A), this);
#endif
}
