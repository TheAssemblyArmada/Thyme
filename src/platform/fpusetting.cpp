/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Function for ensuring FPU mode is consistent.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "fpusetting.h"
#include <float.h>

void Set_FP_Mode()
{
#ifdef PLATFORM_WINDOWS
    _fpreset();
    _controlfp(_statusfp() & 0xFFFCFCFF | 0x00020000, 0x00030300);
#endif
}
