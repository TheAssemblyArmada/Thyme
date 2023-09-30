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

#ifndef GAME_DLL
HeaderTemplateManager *g_theHeaderTemplateManager;
#endif

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
