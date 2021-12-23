/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aiupdate.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void AIUpdateModuleData::Parse_Locomotor_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    // todo needs AIUpdateModuleData
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, const void *>(
        PICK_ADDRESS(0x005CFEF0, 0x007F35AD), ini, formal, store, user_data);
#endif
}
