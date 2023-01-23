/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Assisted Targeting Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "assistedtargetingupdate.h"

bool AssistedTargetingUpdate::Is_Free_To_Assist() const
{
#ifdef GAME_DLL
    return Call_Method<bool, const AssistedTargetingUpdate>(PICK_ADDRESS(0x0061D380, 0x008A30A5), this);
#else
    return false;
#endif
}

void AssistedTargetingUpdate::Assist_Attack(const Object *source_obj, Object *target_obj)
{
#ifdef GAME_DLL
    Call_Method<void, AssistedTargetingUpdate, const Object *, Object *>(
        PICK_ADDRESS(0x0061D3C0, 0x008A310E), this, source_obj, target_obj);
#endif
}
