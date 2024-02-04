/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Header Template
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
#include "gamefont.h"
#include "ini.h"
#include <list>

struct HeaderTemplate
{
    HeaderTemplate() : m_font(nullptr), m_point(0), m_bold(false) {}

    GameFont *m_font;
    Utf8String m_name;
    Utf8String m_fontName;
    int m_point;
    bool m_bold;
};

class HeaderTemplateManager
{
public:
    HeaderTemplateManager();
    ~HeaderTemplateManager();
    void Init();
    HeaderTemplate *Find_Header_Template(Utf8String name);
    HeaderTemplate *New_Header_Template(Utf8String name);
    GameFont *Get_Font_From_Template(Utf8String name);
    HeaderTemplate *Get_First_Header();
    HeaderTemplate *Get_Next_Header(HeaderTemplate *tmplate);
    void Header_Notify_Resolution_Change();
    void Populate_Game_Fonts();

    static void Parse(INI *ini);
    static const FieldParse *Get_Field_Parse() { return s_headerFieldParseTable; }
    static const FieldParse s_headerFieldParseTable[];

private:
    std::list<HeaderTemplate *> m_templates;
};

#ifdef GAME_DLL
extern HeaderTemplateManager *&g_theHeaderTemplateManager;
#else
extern HeaderTemplateManager *g_theHeaderTemplateManager;
#endif
