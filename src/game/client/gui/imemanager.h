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
#pragma once

#include "always.h"
#include "imemanagerinterface.h"

class GameWindow;
class Utf16String;

class IMEManager : public IMEManagerInterface
{
public:
    IMEManager();

    ~IMEManager() override;

    void Init() override;
    void Reset() override {}
    void Update() override {}

    void Attach(GameWindow *window) override;
    void Detach() override;

    void Enable() override;
    void Disable() override;
    bool Is_Enabled() const override;

    bool Is_Attached_To(GameWindow *window) const override;
    GameWindow *Get_Window() const override;

    bool Is_Composing() const override;
    void Get_Composition_String(Utf16String &string) override;
    int Get_Composition_Cursor_Position() const override;

    int Get_Index_Base() const override;

    int Get_Candidate_Count() const override;
    Utf16String *Get_Candidate(int index) override;
    int Get_Selected_Candidate_Index() const override;

    int Get_Candidate_Page_Size() const override;
    int Get_Candidate_Page_Start() const override;

    bool Service_IME_Message(void *window_handle, unsigned int message, int w_param, int l_param) override;

    int Result() const override;

private:
    int m_result;
    GameWindow *m_window;
    unsigned int m_context;
    unsigned int m_oldContext;
    int m_disabled;
    bool m_composing;
    unichar_t m_compositionString[2048 + 1];
    unichar_t m_resultsString[2048 + 1];
    int m_compositionCursorPos;
    int m_compositionStringLength;
    int m_indexBase;
    int m_pageStart;
    int m_pageSize;
    int m_selectedIndex;
    int m_candidateCount;
    Utf16String *m_candidateString;
    bool m_unicodeIME;
    int m_unknownCount;
    GameWindow *m_candidateWindow;
    GameWindow *m_statusWindow;
    GameWindow *m_candidateTextArea;
    GameWindow *m_candidateUpArrow;
    GameWindow *m_candidateDownArrow;
};
