/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Diplomacy
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "diplomacy.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

void Toggle_Diplomacy(bool immediate)
{
#ifdef GAME_DLL
    Call_Function<void, bool>(PICK_ADDRESS(0x005EEF60, 0x00A11E14), immediate);
#endif
}
