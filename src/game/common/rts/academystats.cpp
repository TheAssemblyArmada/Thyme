/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Academy Stats
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "academystats.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void AcademyStats::Record_Income()
{
#ifdef GAME_DLL
    Call_Method<void, AcademyStats>(PICK_ADDRESS(0x00575810, 0x008DD650), this);
#endif
}
