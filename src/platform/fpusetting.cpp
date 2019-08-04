/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Map file handling utility functions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "fpusetting.h"

void Set_FP_Mode()
{
#ifdef GAME_DLL
    Call_Function<void>(0x004A1E40);
#else
    
#endif
}

