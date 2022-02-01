/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Language Filter
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
#include "unicodestring.h"

class LanguageFilter
{
public:
    void Filter_Line(Utf16String &line);
};

#ifdef GAME_DLL
extern LanguageFilter *&g_theLanguageFilter;
#else
extern LanguageFilter *g_theLanguageFilter;
#endif
