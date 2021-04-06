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
#include "dbgwinframe.h"
#include <cstdio>

wxWindow *g_mainWindow;

DbgWinFrame::DbgWinFrame(wxWindow *parent, const wxString &label) :
    DbgWinBaseFrame(parent),
    m_variablesCache(),
    m_nextVarIndex(0),
    m_currentFrame(0),
    m_pauseFrame(-1),
    m_blockVariableUpdate(false)
{
    g_mainWindow = parent;
    SetPosition({ 0, 0 });
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Step, this, XRCID("m_stepBtn"));
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Step_Ten, this, XRCID("m_stepTenBtn"));
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Clear, this, XRCID("m_clearBtn"));
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Clear, this, XRCID("m_clearBtn"));
    Bind(wxEVT_SCROLLWIN_THUMBTRACK, &DbgWinFrame::On_Thumb, this, XRCID("m_m_variableList"));
    Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &DbgWinFrame::On_Thumb_Release, this, XRCID("m_m_variableList"));
    Bind(wxEVT_CLOSE_WINDOW, &DbgWinFrame::On_Exit, this);
}

void DbgWinFrame::On_Step(wxCommandEvent &event)
{
    m_pauseFrame = m_currentFrame + 1;
    Set_Paused(false);
}

void DbgWinFrame::On_Step_Ten(wxCommandEvent &event)
{
    m_pauseFrame = m_currentFrame + 10;
    Set_Paused(false);
}

void DbgWinFrame::On_Clear(wxCommandEvent &event)
{
    m_variablesCache.clear();
    m_variableList->Clear();
    m_nextVarIndex = 0;
    m_msgTxt->Clear();
}

void DbgWinFrame::On_Exit(wxCloseEvent &event)
{
    Iconize();
}

void DbgWinFrame::On_Thumb(wxScrollWinEvent &event)
{
    printf("Handling scroll thumb event.\n");
    if (!m_blockVariableUpdate) {
        m_blockVariableUpdate = true;
    }
}

void DbgWinFrame::On_Thumb_Release(wxScrollWinEvent &event)
{
    printf("Handling scroll thumb release event.\n");
    m_blockVariableUpdate = false;
}

void DbgWinFrame::Set_Frame_Number(int frame)
{
    char buf[32];
    int ret = std::snprintf(buf, sizeof(buf), "Frame %9d", frame);
    m_frameTxt->SetLabel(buf);
    m_frameTxt->Refresh();
    m_currentFrame = frame;

    if (m_currentFrame == m_pauseFrame) {
        Set_Paused(true);
    }
}

void DbgWinFrame::Append_Variable(const char *var, const char *val)
{
    bool refresh = false;
    wxString str = var;
    str += " = ";
    str += val;

    while (m_blockVariableUpdate) {
        wxMilliSleep(1);
    }

    auto it = m_variablesCache.find(var);
    if (it == m_variablesCache.end()) {
        unsigned index = m_nextVarIndex++;
        m_variablesCache[var] = index;
        m_variableList->Insert(str, index);
    } else {
        m_variableList->SetString(it->second, str);
    }
}

void DbgWinFrame::Append_Message(const char *msg)
{
    wxString str = msg;
    str += "\n";

    m_msgTxt->AppendText(str);
}

bool DbgWinFrame::Paused()
{
    return m_pauseBtn->GetValue();
}

bool DbgWinFrame::Run_Fast()
{
    return m_runFastXTenBtn->GetValue();
}

void DbgWinFrame::Set_Paused(bool paused)
{
    m_pauseBtn->SetValue(paused);
}
