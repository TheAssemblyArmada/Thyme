/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Sticky Bomb Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "stickybombupdate.h"

void StickyBombUpdate::Detonate()
{
#ifdef GAME_DLL
    Call_Method<void, StickyBombUpdate>(PICK_ADDRESS(0x005DA160, 0x0098E574), this);
#endif
}
