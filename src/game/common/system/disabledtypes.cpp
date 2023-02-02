/**
 * @file
 *
 * @author xezon
 *
 * @brief DisabledType
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "disabledtypes.h"

#include "bitflags.h"

template<>
const char *DisabledBitFlags::s_bitNamesList[DISABLED_TYPE_COUNT + 1] = {
    "DEFAULT",
    "DISABLED_HACKED",
    "DISABLED_EMP",
    "DISABLED_HELD",
    "DISABLED_PARALYZED",
    "DISABLED_UNMANNED",
    "DISABLED_UNDERPOWERED",
    "DISABLED_FREEFALL",
    "DISABLED_AWESTRUCK",
    "DISABLED_BRAINWASHED",
    "DISABLED_SUBDUED",
    "DISABLED_SCRIPT_DISABLED",
    "DISABLED_SCRIPT_UNDERPOWERED",
    nullptr,
};

#ifndef GAME_DLL
DisabledBitFlags DISABLEDMASK_ALL;
#endif

DisabledBitFlags DISABLEDMASK_NONE;

void Init_Disabled_Masks()
{
    DISABLEDMASK_ALL.Set_All();
}
