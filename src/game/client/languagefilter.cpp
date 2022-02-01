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
#include "languagefilter.h"

#ifndef GAME_DLL
LanguageFilter *g_theLanguageFilter;
#else
#include "hooker.h"
#endif

void LanguageFilter::Filter_Line(Utf16String &line)
{
#ifdef GAME_DLL
    Call_Method<void, LanguageFilter>(PICK_ADDRESS(0x00500E30, 0x008DAE87), this);
#endif
}
