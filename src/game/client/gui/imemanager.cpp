/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Input method editor manager class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "imemanager.h"

#include "unicodestring.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

IMEManager::IMEManager() :
    m_result(0),
    m_window(nullptr),
    m_context(0),
    m_disabled(0),
    m_composing(false),
    m_compositionCursorPos(0),
    m_compositionStringLength(0),
    m_indexBase(1),
    m_pageStart(0),
    m_pageSize(0),
    m_selectedIndex(0),
    m_candidateCount(0),
    m_candidateString(nullptr),
    m_unicodeIME(false),
    m_unknownCount(0),
    m_candidateWindow(nullptr),
    m_statusWindow(nullptr),
    m_candidateTextArea(nullptr),
    m_candidateUpArrow(nullptr),
    m_candidateDownArrow(nullptr),
    // BUGFIX init all members
    m_oldContext(0),
    m_compositionString{},
    m_resultsString{}

{
}

IMEManager::~IMEManager()
{
    // TODO

    Detach();

    // TODO
}

void IMEManager::Init()
{
    // TODO
}

void IMEManager::Attach(GameWindow *window)
{
    // TODO
}

void IMEManager::Detach()
{
    m_window = nullptr;
}

void IMEManager::Enable()
{
    // TOOD
}

void IMEManager::Disable()
{
    // TODO
}

bool IMEManager::Is_Enabled() const
{
    return m_context != 0 && m_disabled == 0;
}

bool IMEManager::Is_Attached_To(GameWindow *window) const
{
    return m_window == window;
}

GameWindow *IMEManager::Get_Window() const
{
    return m_window;
}

bool IMEManager::Is_Composing() const
{
    return m_composing;
}

void IMEManager::Get_Composition_String(Utf16String &string)
{
    string.Set(m_compositionString);
}

int IMEManager::Get_Composition_Cursor_Position() const
{
    return 0;
}

int IMEManager::Get_Index_Base() const
{
    return m_indexBase;
}

int IMEManager::Get_Candidate_Count() const
{
    return m_candidateCount;
}

Utf16String *IMEManager::Get_Candidate(int index)
{
    if (m_candidateString && index >= 0 && index < m_candidateCount) {
        return &m_candidateString[index];
    }

    // TODO this may be dangerous, need to find out if candidate is modified...
    return (Utf16String *)&Utf16String::s_emptyString;
}

int IMEManager::Get_Selected_Candidate_Index() const
{
    return m_selectedIndex;
}

int IMEManager::Get_Candidate_Page_Size() const
{
    return m_pageSize;
}

int IMEManager::Get_Candidate_Page_Start() const
{
    return m_pageStart;
}

bool IMEManager::Service_IME_Message(void *window_handle, unsigned int message, int w_param, int l_param)
{
    // TODO
    return 0;
}

int IMEManager::Result() const
{
    return m_result;
}
