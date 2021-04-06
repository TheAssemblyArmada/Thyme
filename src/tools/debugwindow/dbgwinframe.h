/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief WX window implementation for the DebugWindow plugin.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/xrc/xmlres.h>

#include "wxdbgwinui.h"

#include <atomic>
#include <map>

extern wxWindow *g_mainWindow;

/**
 * The base class here is defined in an XRC resource file while is complied into the binary by the build system.
 */
class DbgWinFrame : public DbgWinBaseFrame
{
public:
    DbgWinFrame(wxWindow *parent, const wxString &label);

    // wx Event handlers
    void On_Step(wxCommandEvent &event);
    void On_Step_Ten(wxCommandEvent &event);
    void On_Clear(wxCommandEvent &event);
    void On_Exit(wxCloseEvent &event);
    void On_Thumb(wxScrollWinEvent &event);
    void On_Thumb_Release(wxScrollWinEvent &event);

    // C API Handlers
    void Set_Frame_Number(int frame);
    void Append_Variable(const char *var, const char *val);
    void Append_Message(const char *msg);
    bool Paused();
    bool Run_Fast();
    void Set_Paused(bool paused);

private:
    std::map<wxString, int> m_variablesCache;
    std::atomic<unsigned> m_nextVarIndex;
    std::atomic<int> m_currentFrame;
    std::atomic<int> m_pauseFrame;
    std::atomic<bool> m_blockVariableUpdate;
};
