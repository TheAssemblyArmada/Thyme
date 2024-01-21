/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Selection Info
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "selectioninfo.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

PickType Get_Pick_Types_For_Context(bool force_to_attack)
{
#ifdef GAME_DLL
    return Call_Function<PickType, bool>(PICK_ADDRESS(0x00723550, 0x0086D346), force_to_attack);
#else
    return PICK_TYPE_UNK1;
#endif
}
