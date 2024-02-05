/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Credits
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
#include "ini.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <list>

class DisplayString;

enum CreditStyle
{
    STYLE_TITLE,
    STYLE_MINORTITLE,
    STYLE_NORMAL,
    STYLE_COLUMN,
    STYLE_BLANK,
};

class CreditsLine
{
public:
    CreditsLine();
    ~CreditsLine();

private:
    CreditStyle m_style;
    Utf16String m_string;
    Utf16String m_secondColumnString;
    bool m_hasFirstColumn;
    bool m_hasSecondColumn;
    DisplayString *m_displayString;
    DisplayString *m_secondColumnDisplayString;
    int m_xPos;
    int m_yPos;
    int m_scroll;
    int m_color;
    friend class CreditsManager;
};

class CreditsManager : public SubsystemInterface
{
public:
    CreditsManager();
    virtual ~CreditsManager() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override;

    void Load();
    void Add_Blank();
    void Add_Text(Utf8String text);
    Utf16String Get_Unicode_String(Utf8String string);

    static void Parse_Blank(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Text(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse(INI *ini);
    const FieldParse *Get_Field_Parse() { return s_creditsFieldParseTable; }
    static const FieldParse s_creditsFieldParseTable[];

private:
    std::list<CreditsLine *> m_creditLines;
    int m_iterator;
    std::list<CreditsLine *> m_displayedCreditLines;
    int m_scrollRate;
    int m_scrollRateEveryFrames;
    bool m_scrollDown;
    int m_titleColor;
    int m_minorTitleColor;
    int m_normalColor;
    CreditStyle m_currentStyle;
    char m_atEndOfList;
    int m_currentScrollFrame;
    int m_fontHeight;
};

#ifdef GAME_DLL
extern CreditsManager *&g_theCredits;
#else
extern CreditsManager *g_theCredits;
#endif
