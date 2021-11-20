/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "polygontrigger.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
PolygonTrigger *PolygonTrigger::s_thePolygonTriggerListPtr;
#endif

int PolygonTrigger::Parse_Polygon_Triggers_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *userdata)
{
#ifdef GAME_DLL
    return Call_Function<int, DataChunkInput &, DataChunkInfo *, void *>(
        PICK_ADDRESS(0x00570D60, 0x006CE6F6), file, info, userdata);
#else
    return 0;
#endif
}

void PolygonTrigger::Delete_Triggers()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x005714C0, 0x006CF080));
#endif
}

bool PolygonTrigger::Point_In_Trigger(ICoord3D &point) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const PolygonTrigger, ICoord3D &>(PICK_ADDRESS(0x00571660, 0x006CF4D4), this, point);
#else
    return 0;
#endif
}
