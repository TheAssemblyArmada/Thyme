/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Point Group
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pointgr.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void PointGroupClass::Init()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00837890, 0x00566750));
#endif
}

void PointGroupClass::Shutdown()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00838010, 0x00566F00));
#endif
}
