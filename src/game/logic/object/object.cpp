/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "object.h"

ObjectShroudStatus Object::Get_Shrouded_Status(int index) const
{
#ifdef GAME_DLL
    return Call_Function<ObjectShroudStatus, int>(PICK_ADDRESS(0x00547D60, 0x007D12FB), index);
#endif
    return ObjectShroudStatus(0);
}
