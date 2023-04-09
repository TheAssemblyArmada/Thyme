/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Ability Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialabilityupdate.h"

SpecialPowerType SpecialAbilityUpdate::Get_Special_Power_Type() const
{
#ifdef GAME_DLL
    return Call_Method<SpecialPowerType, SpecialAbilityUpdate const>(PICK_ADDRESS(0x00605B00, 0x00986CED), this);
#else
    return SPECIAL_INVALID;
#endif
}
