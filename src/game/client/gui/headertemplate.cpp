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
#include "headertemplate.h"
#include "gamefont.h"
#include "globallanguage.h"
#include "registryget.h"

#ifndef GAME_DLL
HeaderTemplateManager *g_theHeaderTemplateManager;
#endif

// clang-format off
const FieldParse HeaderTemplateManager::s_headerFieldParseTable[] = {
    {"Font", &INI::Parse_Quoted_AsciiString, nullptr, offsetof(HeaderTemplate, m_fontName)},
    {"Point", &INI::Parse_Int, nullptr, offsetof(HeaderTemplate, m_point)},
    {"Bold", &INI::Parse_Bool, nullptr, offsetof(HeaderTemplate, m_bold)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

HeaderTemplateManager::HeaderTemplateManager() {}

HeaderTemplateManager::~HeaderTemplateManager()
{
    for (auto it = m_templates.begin(); it != m_templates.end(); it = m_templates.erase(it)) {
        if (*it != nullptr) {
            delete *it;
        }
    }
}

void HeaderTemplateManager::Init()
{
    INI ini;
    Utf8String ini_name;
    ini_name.Format("Data\\%s\\HeaderTemplate.ini", Get_Registry_Language().Str());
    ini.Load(ini_name, INI_LOAD_OVERWRITE, nullptr);
    Populate_Game_Fonts();
}

HeaderTemplate *HeaderTemplateManager::Find_Header_Template(Utf8String name)
{
    for (auto it = m_templates.begin(); it != m_templates.end(); it++) {
        HeaderTemplate *header_template = *it;

        if (header_template->m_name.Compare(name) == 0) {
            return header_template;
        }
    }

    return nullptr;
}

GameFont *HeaderTemplateManager::Get_Font_From_Template(Utf8String name)
{
    HeaderTemplate *header_template = Find_Header_Template(name);

    if (header_template != nullptr) {
        return header_template->m_font;
    } else {
        return nullptr;
    }
}

void HeaderTemplateManager::Populate_Game_Fonts()
{
    for (auto it = m_templates.begin(); it != m_templates.end(); it++) {
        HeaderTemplate *header = *it;
        GameFont *font = g_theFontLibrary->Get_Font(
            header->m_fontName, g_theGlobalLanguage->Adjust_Font_Size(header->m_point), header->m_bold);
        captainslog_dbgassert(font != nullptr,
            "HeaderTemplateManager::Populate_Game_Fonts - Could not find font %s %d",
            header->m_fontName.Str(),
            header->m_point);
        header->m_font = font;
    }
}

HeaderTemplate *HeaderTemplateManager::New_Header_Template(Utf8String name)
{
    HeaderTemplate *header = new HeaderTemplate();
    captainslog_dbgassert(
        header != nullptr, "Unable to create a new Header Template in HeaderTemplateManager::New_Header_Template");

    if (header != nullptr) {
        header->m_name = name;
        m_templates.push_back(header);
        return header;
    } else {
        return nullptr;
    }
}

void HeaderTemplateManager::Parse(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    HeaderTemplate *header = g_theHeaderTemplateManager->Find_Header_Template(name);

    if (header != nullptr) {
        captainslog_dbgassert(false,
            "[LINE: %d in '%s'] Duplicate header Template %s found!",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str(),
            name.Str());
    } else {
        header = g_theHeaderTemplateManager->New_Header_Template(name);
    }

    ini->Init_From_INI(header, Get_Field_Parse());
}

void HeaderTemplateManager::Header_Notify_Resolution_Change()
{
    Populate_Game_Fonts();
}
