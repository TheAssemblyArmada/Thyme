/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief EVA
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "eva.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Eva *g_theEva;
#endif

void Eva::Set_Should_Play(EvaMessage message)
{
#ifdef GAME_DLL
    Call_Method<void, Eva, EvaMessage>(PICK_ADDRESS(0x00513750, 0x009BBA4C), this, message);
#endif
}
