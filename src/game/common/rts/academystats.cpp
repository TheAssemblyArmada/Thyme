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

void AcademyStats::Init(const Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, AcademyStats, const Player *>(PICK_ADDRESS(0x00575300, 0x008DCE35), this, player);
#endif
}

void AcademyStats::Update()
{
#ifdef GAME_DLL
    Call_Method<void, AcademyStats>(PICK_ADDRESS(0x005755A0, 0x008DD21B), this);
#endif
}

void AcademyStats::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00576BF0, 0x008DF4B4), this, xfer);
#endif
}
