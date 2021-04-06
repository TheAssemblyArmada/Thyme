/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief C Interface for DebugWindow developer plugin.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "debugwindow.h"
#include "dbgwinapp.h"
#include "dbgwinframe.h"
#include <cstdio>

void AdjustVariable(const char *var, const char *val)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Variable(var, val);
    }
}

void AdjustVariableAndPause(const char *var, const char *val)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Variable(var, val);
        wxframe->Set_Paused(true);
    }
}

void AppendMessage(const char *msg)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Message(msg);
    }
}

void AppendMessageAndPause(const char *msg)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Message(msg);
        wxframe->Set_Paused(true);
    }
}

bool CanAppContinue()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        return !wxframe->Paused();
    }

    return true;
}

void ForceAppContinue()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Set_Paused(false);
    }
}

bool RunAppFast()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        return wxframe->Run_Fast();
    }

    return false;
}

void SetFrameNumber(int frame)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Set_Frame_Number(frame);
    }
}
