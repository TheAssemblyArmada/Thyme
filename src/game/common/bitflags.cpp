/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around std::bitset.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bitflags.h"

// TODO move this somewhere more appropriate?
template<>
const char *BitFlags<OBJECT_STATUS_COUNT>::s_bitNamesList[] = {
    "NONE",
    "DESTROYED",
    "CAN_ATTACK",
    "UNDER_CONSTRUCTION",
    "UNSELECTABLE",
    "NO_COLLISIONS",
    "NO_ATTACK",
    "AIRBORNE_TARGET",
    "PARACHUTING",
    "REPULSOR",
    "HIJACKED",
    "AFLAME",
    "BURNED",
    "WET",
    "IS_FIRING_WEAPON",
    "IS_BRAKING",
    "STEALTHED",
    "DETECTED",
    "CAN_STEALTH",
    "SOLD",
    "UNDERGOING_REPAIR",
    "RECONSTRUCTING",
    "MASKED",
    "IS_ATTACKING",
    "USING_ABILITY",
    "IS_AIMING_WEAPON",
    "NO_ATTACK_FROM_AI",
    "IGNORING_STEALTH",
    "IS_CARBOMB",
    "DECK_HEIGHT_OFFSET",
    "STATUS_RIDER1",
    "STATUS_RIDER2",
    "STATUS_RIDER3",
    "STATUS_RIDER4",
    "STATUS_RIDER5",
    "STATUS_RIDER6",
    "STATUS_RIDER7",
    "STATUS_RIDER8",
    "FAERIE_FIRE",
    "KILLING_SELF",
    "REASSIGN_PARKING",
    "BOOBY_TRAPPED",
    "IMMOBILE",
    "DISGUISED",
    "DEPLOYED",
    nullptr,
};

template<>
const char *BitFlags<DISABLED_TYPE_COUNT>::s_bitNamesList[] = {
    "DEFAULT",
    "DISABLED_HACKED",
    "DISABLED_EMP",
    "DISABLED_HELD",
    "DISABLED_PARALYZED",
    "DISABLED_UNMANNED",
    "DISABLED_UNDERPOWERED",
    "DISABLED_FREEFALL",
    "DISABLED_AWESTRUCK",
    "DISABLED_BRAINWASHED",
    "DISABLED_SUBDUED",
    "DISABLED_SCRIPT_DISABLED",
    "DISABLED_SCRIPT_UNDERPOWERED",
    nullptr,
};

// TODO Temp to force instantiation. Fixes issue with Parameter::Read_Parameter.
// template class BitFlags<KIND_OF_COUNT>;
