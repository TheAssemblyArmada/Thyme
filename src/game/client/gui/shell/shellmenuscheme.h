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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "ini.h"
#include <list>

class Image;

class ShellMenuSchemeLine
{
public:
    ShellMenuSchemeLine();
    ~ShellMenuSchemeLine() {}

private:
    ICoord2D m_startPosition;
    ICoord2D m_endPosition;
    int m_width;
    int m_color;
    friend class ShellMenuSchemeManager;
};

class ShellMenuSchemeImage
{
public:
    ShellMenuSchemeImage();
    ~ShellMenuSchemeImage() { m_image = nullptr; }

private:
    Utf8String m_unk;
    ICoord2D m_position;
    ICoord2D m_size;
    Image *m_image;
    friend class ShellMenuSchemeManager;
};

class ShellMenuScheme
{
public:
    ShellMenuScheme() {}
    ~ShellMenuScheme();
    void Add_Line(ShellMenuSchemeLine *line);
    void Add_Image(ShellMenuSchemeImage *image);
    void Draw();

private:
    Utf8String m_name;
    std::list<ShellMenuSchemeImage *> m_images;
    std::list<ShellMenuSchemeLine *> m_lines;
    friend class ShellMenuSchemeManager;
};

class ShellMenuSchemeManager
{
public:
    ShellMenuSchemeManager() : m_currentScheme(nullptr) {}
    ~ShellMenuSchemeManager();
    void Init();
    void Update() {}

    ShellMenuScheme *New_Shell_Menu_Scheme(Utf8String name);
    void Set_Shell_Menu_Scheme(Utf8String name);
    void Draw();

    static void Parse_Image_Part(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Line_Part(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse(INI *ini);
    static const FieldParse *Get_Field_Parse() { return s_shellMenuSchemeFieldParseTable; }
    static const FieldParse s_shellMenuSchemeFieldParseTable[];

private:
    std::list<ShellMenuScheme *> m_schemes;
    ShellMenuScheme *m_currentScheme;
};
