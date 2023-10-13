/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Command Translate
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
#include "drawable.h"
#include "messagestream.h"

struct PickAndPlayInfo
{
    bool is_airborne;
    Drawable *drawable;
    WeaponSlotType *weapon_slot_type;
    SpecialPowerType special_power_type;
    PickAndPlayInfo() : is_airborne(false), drawable(nullptr), weapon_slot_type(nullptr), special_power_type(SPECIAL_INVALID)
    {
    }
};

void Pick_And_Play_Unit_Voice_Response(
    const std::list<Drawable *> *list, GameMessage::MessageType type, PickAndPlayInfo *info);
