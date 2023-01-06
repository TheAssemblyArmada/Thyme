/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Power Completion Die
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpowercompletiondie.h"

void SpecialPowerCompletionDie::Notify_Script_Engine()
{
#ifdef GAME_DLL
    Call_Method<void, SpecialPowerCompletionDie>(PICK_ADDRESS(0x0061B510, 0x008863A4), this);
#endif
}

void SpecialPowerCompletionDie::Set_Creator(ObjectID id)
{
#ifdef GAME_DLL
    Call_Method<void, SpecialPowerCompletionDie, ObjectID>(PICK_ADDRESS(0x0061B5B0, 0x00886441), this, id);
#endif
}
