/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Selection Xlat
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "selectionxlat.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
SelectionTranslator *g_theSelectionTranslator;
#endif

bool Can_Select_Drawable(const Drawable *drawable, bool drag_selecting)
{
#ifdef GAME_DLL
    return Call_Function<bool, const Drawable *, bool>(PICK_ADDRESS(0x00614AD0, 0x00A1CCBC), drawable, drag_selecting);
#else
    return false;
#endif
}
