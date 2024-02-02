/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shell Menu Scheme
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shellmenuscheme.h"
#include "ini.h"

ShellMenuSchemeLine::ShellMenuSchemeLine() : m_width(1), m_color(0xFFFFFF)
{
    m_startPosition.x = 0;
    m_startPosition.y = 0;
    m_endPosition.x = 0;
    m_endPosition.y = 0;
}

ShellMenuSchemeImage::ShellMenuSchemeImage() : m_image(nullptr)
{
    m_position.y = 0;
    m_position.x = 0;
    m_size.x = 0;
    m_size.y = 0;
}

ShellMenuScheme::~ShellMenuScheme()
{
    for (auto it = m_images.begin(); it != m_images.end(); it = m_images.erase(it)) {
        delete *it;
    }

    for (auto it = m_lines.begin(); it != m_lines.end(); it = m_lines.erase(it)) {
        delete *it;
    }
}

ShellMenuSchemeManager::~ShellMenuSchemeManager()
{
    m_currentScheme = nullptr;

    for (auto it = m_schemes.begin(); it != m_schemes.end(); it = m_schemes.erase(it)) {
        delete *it;
    }
}

void ShellMenuSchemeManager::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Default\\ShellMenuScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\ShellMenuScheme.ini", INI_LOAD_OVERWRITE, nullptr);
}
