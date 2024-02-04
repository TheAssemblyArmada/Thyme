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
#include "animatewindowmanager.h"
#include "gamewindow.h"
#include "subsysteminterface.h"

class WindowLayout;
class ShellMenuSchemeManager;

class Shell : public SubsystemInterface
{
public:
    Shell();
    virtual ~Shell() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    WindowLayout *Find_Screen_By_Filename(Utf8String filename);
    void Hide(bool hide);
    void Push(Utf8String filename, bool b);
    void Pop();
    void Pop_Immediate();
    void Show_Shell(int i);
    void Show_Shell_Map(bool use_shell_map);
    void Hide_Shell();
    WindowLayout *Top();
    void Link_Screen(WindowLayout *screen);
    void Unlink_Screen(WindowLayout *screen);
    void Do_Push(Utf8String layout_file);
    void Do_Pop(bool impending_push);
    void Shutdown_Complete(WindowLayout *screen, bool impending_push);
    void Register_With_Animate_Manager(GameWindow *win, AnimTypes anim_type, bool needs_to_finish, unsigned int delay_ms);
    bool Is_Anim_Finished();
    void Reverse_Animate_Window();
    bool Is_Anim_Reversed();
    void Load_Scheme(Utf8String scheme);
    WindowLayout *Get_Save_Load_Menu_Layout();
    WindowLayout *Get_Popup_Replay_Layout();
    WindowLayout *Get_Options_Layout();
    void Destroy_Options_Layout();

    int Get_Screen_Count() const { return m_screenCount; }
    bool Is_Shell_Active() const { return m_isShellActive; }
    ShellMenuSchemeManager *Get_Menu_Scheme_Manager() const { return m_shellMenuSchemeManager; }

#ifdef GAME_DLL
    Shell *Hook_Ctor() { return new (this) Shell; }
#endif

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
