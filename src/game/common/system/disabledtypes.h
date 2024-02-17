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
#pragma once

#include "always.h"

template<int> class BitFlags;

enum DisabledType
{
    DISABLED_TYPE_DEFAULT,
    DISABLED_TYPE_DISABLED_HACKED,
    DISABLED_TYPE_DISABLED_EMP,
    DISABLED_TYPE_DISABLED_HELD,
    DISABLED_TYPE_DISABLED_PARALYZED,
    DISABLED_TYPE_DISABLED_UNMANNED,
    DISABLED_TYPE_DISABLED_UNDERPOWERED,
    DISABLED_TYPE_DISABLED_FREEFALL,
    DISABLED_TYPE_DISABLED_AWESTRUCK,
    DISABLED_TYPE_DISABLED_BRAINWASHED,
    DISABLED_TYPE_DISABLED_SUBDUED,
    DISABLED_TYPE_DISABLED_SCRIPT_DISABLED,
    DISABLED_TYPE_DISABLED_SCRIPT_UNDERPOWERED,

    DISABLED_TYPE_COUNT
};

using DisabledBitFlags = BitFlags<DISABLED_TYPE_COUNT>;

#ifdef GAME_DLL
extern DisabledBitFlags &DISABLEDMASK_ALL;
#else
extern DisabledBitFlags DISABLEDMASK_ALL;
#endif
extern DisabledBitFlags DISABLEDMASK_NONE;
void Init_Disabled_Masks();
