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
#ifdef PLATFORM_WINDOWS
#include <imm.h>
#endif

class GameWindow;
class Utf16String;

class IMEManager : public IMEManagerInterface
{
public:
    IMEManager();

    virtual ~IMEManager() override;

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void Attach(GameWindow *window) override;
    virtual void Detach() override;

    virtual void Enable() override;
    virtual void Disable() override;
    virtual bool Is_Enabled() const override;

    virtual bool Is_Attached_To(GameWindow *window) const override;
    virtual GameWindow *Get_Window() const override;

    virtual bool Is_Composing() const override;
    virtual void Get_Composition_String(Utf16String &string) override;
    virtual int Get_Composition_Cursor_Position() const override;

    virtual int Get_Index_Base() const override;

    virtual int Get_Candidate_Count() const override;
    virtual Utf16String *Get_Candidate(int index) override;
    virtual int Get_Selected_Candidate_Index() const override;

    virtual int Get_Candidate_Page_Size() const override;
    virtual int Get_Candidate_Page_Start() const override;

    virtual bool Service_IME_Message(void *window_handle, unsigned int message, int w_param, int l_param) override;

    virtual int Result() const override;

    unichar_t Convert_Char_To_Wide(unsigned int wparam);
    void Convert_To_Unicode(char *mbcs, Utf16String &unicode);
    void Update_Composition_String();
    void Get_Results_String();
    void Open_Candidate_List(int candidate_flags);
    void Close_Candidate_List(int candidate_flags);
    void Update_Candidate_List(int candidate_flags);
    void Update_Properties();
    void Resize_Candidate_Window(int page_size);
    void Open_Status_Window();
    void Close_Status_Window();
    void Update_Status_Window();

private:
    int m_result;
    GameWindow *m_window;
#ifdef PLATFORM_WINDOWS
    HIMC m_context;
    HIMC m_oldContext;
#else
    unsigned int m_context;
    unsigned int m_oldContext;
#endif
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

IMEManagerInterface *Create_IME_Manager_Interface();
