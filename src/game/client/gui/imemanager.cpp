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
#include "gamewindowmanager.h"
#include "main.h"
#include "namekeygenerator.h"
#include "unicodestring.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

IMEManager::IMEManager() :
    // BUGFIX init all members
    m_result(0),
    m_window(nullptr),
    m_context(0),
    m_oldContext(0),
    m_disabled(0),
    m_composing(false),
    m_compositionString{},
    m_resultsString{},
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
    m_candidateDownArrow(nullptr)
{
}

IMEManager::~IMEManager()
{
    if (m_candidateWindow != nullptr) {
        g_theWindowManager->Win_Destroy(m_candidateWindow);
    }

    if (m_statusWindow != nullptr) {
        g_theWindowManager->Win_Destroy(m_statusWindow);
    }

    if (m_candidateString != nullptr) {
        delete m_candidateString;
    }

    Detach();
#ifdef PLATFORM_WINDOWS
    ImmAssociateContext(g_applicationHWnd, m_oldContext);
    ImmReleaseContext(g_applicationHWnd, m_oldContext);

    if (m_context != nullptr) {
        ImmDestroyContext(m_context);
    }
#endif
}

void IMEManager::Init()
{
#ifdef PLATFORM_WINDOWS
    m_context = ImmCreateContext();
    m_oldContext = ImmGetContext(g_applicationHWnd);
#endif
    m_disabled = false;
    m_candidateWindow = g_theWindowManager->Win_Create_From_Script("IMECandidateWindow.wnd", nullptr);

    if (m_candidateWindow != nullptr) {
        m_candidateWindow->Win_Set_Status(WIN_STATUS_ABOVE);
        m_candidateWindow->Win_Hide(true);
        m_candidateTextArea = g_theWindowManager->Win_Get_Window_From_Id(
            m_candidateWindow, g_theNameKeyGenerator->Name_To_Key("IMECandidateWindow.wnd:TextArea"));
        m_candidateUpArrow = g_theWindowManager->Win_Get_Window_From_Id(
            m_candidateWindow, g_theNameKeyGenerator->Name_To_Key("IMECandidateWindow.wnd:UpArrow"));
        m_candidateDownArrow = g_theWindowManager->Win_Get_Window_From_Id(
            m_candidateWindow, g_theNameKeyGenerator->Name_To_Key("IMECandidateWindow.wnd:DownArrow"));

        if (m_candidateTextArea == nullptr) {
            g_theWindowManager->Win_Destroy(m_candidateWindow);
            m_candidateWindow = nullptr;
        }
    }

    m_statusWindow = g_theWindowManager->Win_Create_From_Script("IMEStatusWindow.wnd", nullptr);

    if (m_statusWindow != nullptr) {
        m_statusWindow->Win_Hide(true);
    }

    if (m_candidateWindow != nullptr) {
        m_candidateWindow->Win_Set_User_Data(g_theIMEManager);
        m_candidateTextArea->Win_Set_User_Data(g_theIMEManager);
    }

    Detach();
    Enable();
}

void IMEManager::Attach(GameWindow *window)
{
    if (m_window != window) {
        Detach();

        if (m_disabled == 0) {
#ifdef PLATFORM_WINDOWS
            ImmAssociateContext(g_applicationHWnd, m_context);
#endif
            Update_Status_Window();
        }

        m_window = window;
    }
}

void IMEManager::Detach()
{
    m_window = nullptr;
}

void IMEManager::Enable()
{
    if (--m_disabled <= 0) {
        m_disabled = 0;
#ifdef PLATFORM_WINDOWS
        ImmAssociateContext(g_applicationHWnd, m_context);
#endif
    }
}

void IMEManager::Disable()
{
    m_disabled++;
#ifdef PLATFORM_WINDOWS
    ImmAssociateContext(g_applicationHWnd, nullptr);
#endif
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
#ifdef GAME_DLL
    return Call_Method<bool, IMEManager, void *, unsigned int, int, int>(
        PICK_ADDRESS(0x00412A30, 0x00A228A1), this, window_handle, message, w_param, l_param);
#else
    return false;
#endif
}

int IMEManager::Result() const
{
    return m_result;
}

void IMEManager::Update_Status_Window() {}

IMEManagerInterface *Create_IME_Manager_Interface()
{
    return new IMEManager();
}
