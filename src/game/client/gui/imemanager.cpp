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
#include "gamefont.h"
#include "gamewindowmanager.h"
#include "main.h"
#include "namekeygenerator.h"
#include "unicodestring.h"
#include "w3ddisplay.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifdef PLATFORM_WINDOWS
#include <mbstring.h>
#include <windows.h>
#endif

static const int IMECandidateWindowLineSpacing = 2;

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
#ifdef PLATFORM_WINDOWS
    bool result;
    captainslog_dbgassert(window_handle == g_applicationHWnd, "Unexpected window handle for IMEManager");

    switch (message) {
        case WM_IME_NOTIFY:
            switch (w_param) {
                case IMN_CHANGECANDIDATE:
                    Update_Candidate_List(l_param);
                    m_result = 1;
                    result = true;
                    break;
                case IMN_CLOSECANDIDATE:
                    Close_Candidate_List(l_param);
                    m_result = 1;
                    result = true;
                    break;
                case IMN_OPENCANDIDATE:
                    Open_Candidate_List(l_param);
                    m_result = 1;
                    result = true;
                    break;
                case IMN_SETCONVERSIONMODE:
                    result = false;
                    break;
                case IMN_SETSENTENCEMODE:
                    result = false;
                    break;
                case IMN_GUIDELINE:
                    m_result = 1;
                    result = true;
                    break;
                default:
                    m_result = 1;
                    result = true;
                    break;
            }
            break;
        case WM_IME_COMPOSITIONFULL:
            m_result = 1;
            result = true;
            break;
        case WM_IME_SELECT:
            captainslog_debug("IMM: WM_IME_SELECT");
            result = false;
            break;
        case WM_IME_CHAR: {
            unichar_t c = Convert_Char_To_Wide(w_param);

            if (m_window != nullptr && (c > U_CHAR(' ') || c == U_CHAR('\r'))) {
                g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_IME_CHAR, w_param, l_param);
                m_result = 0;
                result = true;
            } else {
                result = false;
            }

            break;
        }
        case WM_IME_SETCONTEXT:
            Update_Properties();
            m_result = 0;
            result = false;
        case WM_CHAR:
            if (m_window != nullptr && (w_param > U_CHAR(' ') || w_param == U_CHAR('\r'))) {
                g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_IME_CHAR, w_param, l_param);
                m_result = 0;
                result = true;
            } else {
                result = false;
            }

            break;
        case WM_IME_STARTCOMPOSITION:
            m_composing = true;
            m_unknownCount = 0;
            Update_Composition_String();
            m_result = 1;
            result = true;
            break;
        case WM_IME_ENDCOMPOSITION:
            m_composing = false;

            while (m_unknownCount > 0) {
                g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_CHAR, 14, 2);
                --m_unknownCount;
            }

            Close_Candidate_List(0);
            result = true;
            break;
        case WM_IME_COMPOSITION:
            if ((l_param & GCS_RESULTSTR) != 0) {
                if (m_window != nullptr) {
                    m_composing = false;

                    while (m_unknownCount > 0) {
                        g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_CHAR, 14, 2);
                        --m_unknownCount;
                    }

                    unichar_t *results_string = m_resultsString;
                    Get_Results_String();

                    while (*results_string != U_CHAR('\0')) {
                        g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_IME_CHAR, *results_string++, 0);
                    }
                }

                m_unknownCount = 0;
            } else if ((l_param & CS_INSERTCHAR) != 0 && (l_param & CS_NOMOVECARET) != 0) {
                if (m_window != nullptr) {
                    m_composing = false;

                    while (m_unknownCount > 0) {
                        g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_CHAR, 14, 2);
                        --m_unknownCount;
                    }

                    unichar_t *composition_string = m_compositionString;
                    Update_Composition_String();

                    while (*composition_string != U_CHAR('\0')) {
                        g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_IME_CHAR, *composition_string++, 0);
                        m_unknownCount++;
                    }

                    m_composing = true;
                }
            } else if ((l_param & GCS_COMPSTR) != 0 && m_window != nullptr) {
                m_composing = false;

                while (m_unknownCount > 0) {
                    g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_CHAR, 14, 2);
                    --m_unknownCount;
                }

                unichar_t *composition_string = m_compositionString;
                Update_Composition_String();

                while (*composition_string != U_CHAR('\0')) {
                    g_theWindowManager->Win_Send_Input_Msg(m_window, GWM_IME_CHAR, *composition_string++, 0);
                    m_unknownCount++;
                }
            }

            m_result = 1;
            result = true;
            break;
        default:
            result = false;
            break;
    }

    return result;
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

unichar_t IMEManager::Convert_Char_To_Wide(unsigned int wparam)
{
    char buf[3];
    unichar_t c[2];

    if ((wparam & 0xFF00) != 0) {
        buf[0] = (wparam << 8) & 0xFF;
        buf[1] = wparam & 0xFF;
        buf[2] = 0;
    } else {
        buf[0] = wparam & 0xFF;
        buf[1] = 0;
    }

#ifdef PLATFORM_WINDOWS
    if (MultiByteToWideChar(0, 0, buf, strlen(buf), c, 1) == 1) {
        return c[0];
    } else {
        return U_CHAR('\0');
    }
#else
    return U_CHAR('\0');
#endif
}

void IMEManager::Update_Composition_String()
{
#ifdef PLATFORM_WINDOWS
    m_compositionCursorPos = 0;
    m_compositionString[0] = U_CHAR('\0');
    m_compositionStringLength = 0;

    if (m_context != nullptr) {
        LONG ret1 = ImmGetCompositionStringW(m_context, 8, m_compositionString, 2048);

        if (ret1 < 0) {
            char buf[4098];
            LONG ret2 = ImmGetCompositionStringA(m_context, 8, buf, 4096);

            if (ret2 > 0) {
                buf[ret2] = '\0';
                int unicode_count = MultiByteToWideChar(0, 0, buf, -1, m_compositionString, 2048);
                m_compositionString[2048] = U_CHAR('\0');
                int len;

                if (unicode_count >= 0) {
                    m_compositionCursorPos = ImmGetCompositionStringA(m_context, 128, nullptr, 0);
                    len = u_strlen(m_compositionString);
                } else {
                    len = 0;
                }

                m_compositionCursorPos = _mbsnccnt(reinterpret_cast<const unsigned char *>(buf), m_compositionCursorPos);
                m_compositionString[len] = U_CHAR('\0');
                m_compositionStringLength = len;

                if (m_compositionCursorPos <= len) {
                    if (m_compositionCursorPos < 0) {
                        m_compositionCursorPos = 0;
                    }
                } else {
                    m_compositionCursorPos = len;
                }
            }
        } else {
            m_compositionStringLength = ret1 / 2;
            m_compositionCursorPos = ImmGetCompositionStringW(m_context, 128, 0, 0);
        }
    }

    captainslog_dbgassert(m_compositionStringLength < 2048, "composition string too large");
    m_compositionString[m_compositionStringLength] = U_CHAR('\0');
    m_compositionString[2048] = U_CHAR('\0');
#endif
}

void IMEManager::Get_Results_String()
{
#ifdef PLATFORM_WINDOWS
    int len = 0;
    m_resultsString[0] = U_CHAR('\0');

    if (m_context != nullptr) {
        LONG ret1 = ImmGetCompositionStringW(m_context, 2048, m_resultsString, 2048);

        if (ret1 < 0) {
            char buf[4098];
            ret1 = ImmGetCompositionStringA(m_context, 2048, buf, 4096);

            if (ret1 > 0) {
                buf[ret1] = '\0';
                int unicode_count = MultiByteToWideChar(0, 0, buf, strlen(buf), m_resultsString, 2048);

                if (unicode_count < 0) {
                    unicode_count = 0;
                }

                len = unicode_count;
            } else {
                len = ret1 / 2;
            }
        }
    }

    captainslog_dbgassert(len < 2048, "composition string too large");
    m_resultsString[len] = U_CHAR('\0');
    m_resultsString[2048] = U_CHAR('\0');
#endif
}

void IMEManager::Open_Candidate_List(int candidate_flags)
{
    if (m_candidateWindow != nullptr) {
        Update_Candidate_List(candidate_flags);
        Resize_Candidate_Window(m_pageSize);
        m_candidateWindow->Win_Hide(false);
        m_candidateWindow->Win_Bring_To_Top();
        g_theWindowManager->Win_Set_Modal(m_candidateWindow);
        Update_Properties();
        m_candidateWindow->Win_Set_Position(g_theDisplay->Get_Width(), 0);
    }
}

void IMEManager::Close_Candidate_List(int candidate_flags)
{
    if (m_candidateWindow != nullptr) {
        m_candidateWindow->Win_Hide(true);
        g_theWindowManager->Win_Unset_Modal(m_candidateWindow);
    }

    if (m_candidateString != nullptr) {
        delete[] m_candidateString;
        m_candidateString = nullptr;
    }

    m_candidateCount = 0;
}

void IMEManager::Update_Candidate_List(int candidate_flags)
{
#ifdef PLATFORM_WINDOWS
    if (m_candidateString != nullptr) {
        delete[] m_candidateString;
        m_candidateString = nullptr;
    }

    m_pageSize = 10;
    m_candidateCount = 0;
    m_pageStart = 0;
    m_selectedIndex = 0;

    if (m_candidateWindow != nullptr && m_context != nullptr && candidate_flags) {
        DWORD index = 0;
        int flags = 1;

        while (index < 32) {
            if ((flags & candidate_flags) != 0) {
                bool is_unicode = true;
                DWORD count = 0;
                DWORD size = ImmGetCandidateListCountW(m_context, &count);

                if (size > 0 || (is_unicode = false, size = ImmGetCandidateListCountA(m_context, &count), size > 0)) {
                    CANDIDATELIST *list = new CANDIDATELIST[size];

                    if (list != nullptr) {
                        memset(list, 0, size);
                        bool b2 = true;
                        DWORD size2;

                        if (is_unicode) {
                            size2 = ImmGetCandidateListW(m_context, index, list, size);
                        } else {
                            size2 = ImmGetCandidateListA(m_context, index, list, size);
                        }

                        if (size2 == 0 || size2 > size) {
                            captainslog_dbgassert(size2 < size, "IME candidate buffer overrun");
                            b2 = false;
                        }

                        if (b2 && list->dwStyle && list->dwStyle != IME_CAND_CODE) {
                            if (list->dwPageStart > list->dwSelection
                                || list->dwSelection >= list->dwPageSize + list->dwPageStart) {
                                list->dwPageStart = list->dwPageSize * (list->dwSelection / list->dwPageSize);
                            }

                            m_pageSize = list->dwPageSize;
                            m_candidateCount = list->dwCount;
                            m_pageStart = list->dwPageStart;
                            m_selectedIndex = list->dwSelection;

                            if (m_candidateUpArrow != nullptr) {
                                m_candidateUpArrow->Win_Hide(m_pageStart == 0);
                            }

                            if (m_candidateDownArrow != nullptr) {
                                m_candidateDownArrow->Win_Hide(m_candidateCount - m_pageStart <= m_pageSize);
                            }

                            if (m_candidateCount > 0) {
                                m_candidateString = new Utf16String[m_candidateCount];

                                if (m_candidateString != nullptr) {
                                    for (int i = 0; i < m_candidateCount; i++) {
                                        char *candidate = reinterpret_cast<char *>(list) + list->dwOffset[i];

                                        if (is_unicode) {
                                            m_candidateString->Set(reinterpret_cast<unichar_t *>(candidate));
                                        } else {
                                            Convert_To_Unicode(candidate, m_candidateString[i]);
                                        }
                                    }
                                }
                            }
                        }

                        delete[] list;
                    }
                }

                return;
            }

            index++;
            flags *= 2;
        }
    }
#endif
}

void IMEManager::Convert_To_Unicode(char *mbcs, Utf16String &unicode)
{
#ifdef PLATFORM_WINDOWS
    int required_count = MultiByteToWideChar(0, 0, mbcs, strlen(mbcs), 0, 0);
    unicode.Clear();

    if (required_count > 0) {
        unichar_t *buf = new unichar_t[required_count + 1];

        if (buf != nullptr) {
            int written_count = MultiByteToWideChar(0, 0, mbcs, strlen(mbcs), buf, required_count);

            if (written_count > 0) {
                buf[written_count] = U_CHAR('\0');
                unicode = buf;
            } else {
                unicode.Clear();
            }

            delete[] buf;
        }
    }
#endif
}

void IMEManager::Update_Properties()
{
#ifdef PLATFORM_WINDOWS
    HKL layout = GetKeyboardLayout(0);
    DWORD property = ImmGetProperty(layout, IGP_PROPERTY);
    m_indexBase = (property & IME_PROP_CANDLIST_START_FROM_1) != 0;
    m_unicodeIME = (property & IME_PROP_UNICODE) != 0;
#endif
}

void IMEManager::Resize_Candidate_Window(int page_size)
{
    if (m_candidateWindow != nullptr) {
        GameFont *font = m_candidateTextArea->Win_Get_Font();

        if (font != nullptr) {
            int text_area_height = (IMECandidateWindowLineSpacing + font->m_height) * page_size;

            int width;
            int height;
            m_candidateTextArea->Win_Get_Size(&width, &height);
            int window_add_height = text_area_height - height;
            m_candidateTextArea->Win_Set_Size(width, text_area_height);
            m_candidateWindow->Win_Get_Size(&width, &height);
            height += window_add_height;
            m_candidateWindow->Win_Set_Size(width, height);

            if (m_candidateDownArrow != nullptr) {
                int x;
                int y;
                m_candidateDownArrow->Win_Get_Position(&x, &y);
                m_candidateDownArrow->Win_Set_Position(x, window_add_height + y);
            }
        }
    }
}
