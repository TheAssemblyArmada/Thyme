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
#include "image.h"
#include "ini.h"
#include "shell.h"

// clang-format off
const FieldParse ShellMenuSchemeManager::s_shellMenuSchemeFieldParseTable[] = 
{
    {"ImagePart", &ShellMenuSchemeManager::Parse_Image_Part, nullptr, 0},
    {"LinePart", &ShellMenuSchemeManager::Parse_Line_Part, nullptr, 0},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

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

void ShellMenuScheme::Add_Line(ShellMenuSchemeLine *line)
{
    if (line != nullptr) {
        m_lines.push_back(line);
    }
}

void ShellMenuScheme::Add_Image(ShellMenuSchemeImage *image)
{
    if (image != nullptr) {
        m_images.push_back(image);
    }
}

void ShellMenuSchemeManager::Parse_Image_Part(INI *ini, void *formal, void *store, const void *user_data)
{
    // clang-format off
    static const FieldParse myFieldParse[] = 
    {
        {"Position", &INI::Parse_ICoord2D, nullptr, offsetof(ShellMenuSchemeImage, m_position)},
        {"Size", &INI::Parse_ICoord2D, nullptr, offsetof(ShellMenuSchemeImage, m_size)},
        {"ImageName", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ShellMenuSchemeImage, m_image)},
        {nullptr, nullptr, nullptr, 0}
    };
    // clang-format on

    ShellMenuSchemeImage *image = new ShellMenuSchemeImage();
    ini->Init_From_INI(image, myFieldParse);
    static_cast<ShellMenuScheme *>(formal)->Add_Image(image);
}

void ShellMenuSchemeManager::Parse_Line_Part(INI *ini, void *formal, void *store, const void *user_data)
{
    // clang-format off
    static const FieldParse myFieldParse[] = 
    {
        {"StartPosition", &INI::Parse_ICoord2D, nullptr, offsetof(ShellMenuSchemeLine, m_startPosition)},
        {"EndPosition", &INI::Parse_ICoord2D, nullptr, offsetof(ShellMenuSchemeLine, m_endPosition)},
        {"Color", &INI::Parse_Color_Int, nullptr, offsetof(ShellMenuSchemeLine, m_color)},
        {"Width", &INI::Parse_Int, nullptr, offsetof(ShellMenuSchemeLine, m_width)},
        {nullptr, nullptr, nullptr, 0}
    };
    // clang-format on

    ShellMenuSchemeLine *line = new ShellMenuSchemeLine();
    ini->Init_From_INI(line, myFieldParse);
    static_cast<ShellMenuScheme *>(formal)->Add_Line(line);
}

ShellMenuScheme *ShellMenuSchemeManager::New_Shell_Menu_Scheme(Utf8String name)
{
    name.To_Lower();

    for (auto it = m_schemes.begin(); it != m_schemes.end(); it++) {
        ShellMenuScheme *scheme = *it;

        if (scheme->m_name.Compare(name) == 0) {
            m_schemes.erase(it);
            delete scheme;
            break;
        }
    }

    ShellMenuScheme *scheme = new ShellMenuScheme();
    scheme->m_name.Set(name);
    m_schemes.push_back(scheme);
    return scheme;
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

void ShellMenuSchemeManager::Parse(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    ShellMenuSchemeManager *manager = g_theShell->Get_Menu_Scheme_Manager();

    if (manager == nullptr) {
        captainslog_dbgassert(false, "ShellMenuSchemeManager::Parse: Unable to Get SMSchemeManager");
    }

    if (manager != nullptr) {
        ShellMenuScheme *scheme = manager->New_Shell_Menu_Scheme(name);
        captainslog_dbgassert(
            scheme != nullptr, "ShellMenuSchemeManager::Parse: Unable to allocate Scheme '%s'", name.Str());
        ini->Init_From_INI(scheme, ShellMenuSchemeManager::Get_Field_Parse());
    }
}
