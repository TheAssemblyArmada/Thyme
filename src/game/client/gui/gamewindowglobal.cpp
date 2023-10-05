/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Window Manager Global
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorspace.h"
#include "display.h"
#include "gamefont.h"
#include "gamewindowmanager.h"
#include "image.h"

void GameWindowManager::Win_Draw_Image(const Image *image, int start_x, int start_y, int end_x, int end_y, int color)
{
    g_theDisplay->Draw_Image(image, start_x, start_y, end_x, end_y, color, Display::DRAWIMAGE_ADDITIVE);
}

void GameWindowManager::Win_Fill_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Fill_Rect(start_x, start_y, end_x - start_x, end_y - start_y, color);
}

void GameWindowManager::Win_Open_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Open_Rect(start_x, start_y, end_x - start_x, end_y - start_y, width, color);
}

void GameWindowManager::Win_Draw_Line(int color, float width, int start_x, int start_y, int end_x, int end_y)
{
    g_theDisplay->Draw_Line(start_x, start_y, end_x, end_y, width, color);
}

const Image *GameWindowManager::Win_Find_Image(const char *name)
{
    if (g_theMappedImageCollection == nullptr) {
        return nullptr;
    }

    return g_theMappedImageCollection->Find_Image_By_Name(name);
}

int GameWindowManager::Win_Make_Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    return Make_Color(red, green, blue, alpha);
}

void GameWindowManager::Win_Format_Text(GameFont *font, Utf16String text, int color, int x, int y, int width, int height) {}

void GameWindowManager::Win_Get_Text_Size(GameFont *font, Utf16String text, int *width, int *height, int max_width)
{
    if (width != nullptr) {
        *width = 0;
    }

    if (height != nullptr) {
        *height = 0;
    }
}

int GameWindowManager::Win_Font_Height(GameFont *font)
{
    return font->m_height;
}

int GameWindowManager::Win_Is_Digit(int c)
{
    // Can't figure out how to make this work on Linux, making it windows only for now
#ifdef PLATFORM_WINDOWS
    return iswdigit(c);
#else
    return false;
#endif
}

int GameWindowManager::Win_Is_Ascii(int c)
{
    // Can't figure out how to make this work on Linux, making it windows only for now
#ifdef PLATFORM_WINDOWS
    return iswascii(c);
#else
    return false;
#endif
}

int GameWindowManager::Win_Is_Al_Num(int c)
{
    // Can't figure out how to make this work on Linux, making it windows only for now
#ifdef PLATFORM_WINDOWS
    return iswalnum(c);
#else
    return false;
#endif
}

GameFont *GameWindowManager::Win_Find_Font(Utf8String font_name, int point_size, bool bold)
{
    if (g_theFontLibrary != nullptr) {
        return g_theFontLibrary->Get_Font(font_name, point_size, bold);
    } else {
        return nullptr;
    }
}
