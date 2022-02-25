/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Provides an interface for the Input method editor manager.
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

class GameWindow;
class Utf16String;

class IMEManagerInterface : public SubsystemInterface
{
public:
    virtual ~IMEManagerInterface() {}

    virtual void Attach(GameWindow *window) = 0;
    virtual void Detach() = 0;

    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual bool Is_Enabled() const = 0;

    virtual bool Is_Attached_To(GameWindow *window) const = 0;
    virtual GameWindow *Get_Window() const = 0;

    virtual bool Is_Composing() const = 0;
    virtual void Get_Composition_String(Utf16String &string) = 0;
    virtual int Get_Composition_Cursor_Position() const = 0;

    virtual int Get_Index_Base() const = 0;

    virtual int Get_Candidate_Count() const = 0;
    virtual Utf16String *Get_Candidate(int index) = 0;
    virtual int Get_Selected_Candidate_Index() const = 0;

    virtual int Get_Candidate_Page_Size() const = 0;
    virtual int Get_Candidate_Page_Start() const = 0;

    virtual bool Service_IME_Message(void *window_handle, unsigned int message, int w_param, int l_param) = 0;

    virtual int Result() const = 0;
};

#ifdef GAME_DLL
extern IMEManagerInterface *&g_theIMEManager;
#else
extern IMEManagerInterface *g_theIMEManager;
#endif
