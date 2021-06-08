/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Base class for game objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "object.h"

void Object::Set_Receiving_Difficulty_Bonus(bool bonus)
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00548560, 0x007D19D0), this, bonus);
#endif
}
