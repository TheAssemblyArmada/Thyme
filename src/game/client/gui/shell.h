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
#pragma once
#include "always.h"
#include "subsysteminterface.h"

class WindowLayout;
class AnimateWindowManager;
class ShellMenuSchemeManager;

class Shell : public SubsystemInterface
{
public:
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    void Push(Utf8String filename, bool b);
    WindowLayout *Top();
    void Show_Shell(int i);
    void Hide_Shell();

    int Get_Screen_Count() const { return m_screenCount; }

private:
    WindowLayout *m_screenStack[16];
    int m_screenCount;
    WindowLayout *m_background;
    bool m_clearBackground;
    bool m_pendingPush;
    bool m_pendingPop;
    Utf8String m_pendingPushName;
    bool m_isShellActive;
    bool m_shellMapOn;
    AnimateWindowManager *m_animateWindowManager;
    ShellMenuSchemeManager *m_shellMenuSchemeManager;
    WindowLayout *m_saveLoad;
    WindowLayout *m_popupReplay;
    WindowLayout *m_optionsLayout;
};

#ifdef GAME_DLL
extern Shell *&g_theShell;
#else
extern Shell *g_theShell;
#endif
