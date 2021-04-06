/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief WX application implementation for the DebugWindow plugin.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <wx/app.h>

extern const int CMD_SHOW_WINDOW;
extern const int CMD_TERMINATE;
class DbgWinFrame;

class DbgWinApp : public wxApp
{
public:
    DbgWinApp();

    virtual bool OnInit() override;

    DbgWinFrame *Frame() { return m_frame; }

private:
    void OnShowWindow(wxThreadEvent &event);
    void OnTerminate(wxThreadEvent &event);

private:
    DbgWinFrame *m_frame;
};

wxDECLARE_APP(DbgWinApp);
