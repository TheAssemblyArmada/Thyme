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
const char *BitFlags<MODELCONDITION_COUNT>::s_bitNamesList[] = {
    "TOPPLED",
    "FRONTCRUSHED",
    "BACKCRUSHED",
    "DAMAGED",
    "REALLYDAMAGED",
    "RUBBLE",
    "SPECIAL_DAMAGED",
    "NIGHT",
    "SNOW",
    "PARACHUTING",
    "GARRISONED",
    "ENEMYNEAR",
    "WEAPONSET_VETERAN",
    "WEAPONSET_ELITE",
    "WEAPONSET_HERO",
    "WEAPONSET_CRATEUPGRADE_ONE",
    "WEAPONSET_CRATEUPGRADE_TWO",
    "WEAPONSET_PLAYER_UPGRADE",
    "DOOR_1_OPENING",
    "DOOR_1_CLOSING",
    "DOOR_1_WAITING_OPEN",
    "DOOR_1_WAITING_TO_CLOSE",
    "DOOR_2_OPENING",
    "DOOR_2_CLOSING",
    "DOOR_2_WAITING_OPEN",
    "DOOR_2_WAITING_TO_CLOSE",
    "DOOR_3_OPENING",
    "DOOR_3_CLOSING",
    "DOOR_3_WAITING_OPEN",
    "DOOR_3_WAITING_TO_CLOSE",
    "DOOR_4_OPENING",
    "DOOR_4_CLOSING",
    "DOOR_4_WAITING_OPEN",
    "DOOR_4_WAITING_TO_CLOSE",
    "ATTACKING",
    "PREATTACK_A",
    "FIRING_A",
    "BETWEEN_FIRING_SHOTS_A",
    "RELOADING_A",
    "PREATTACK_B",
    "FIRING_B",
    "BETWEEN_FIRING_SHOTS_B",
    "RELOADING_B",
    "PREATTACK_C",
    "FIRING_C",
    "BETWEEN_FIRING_SHOTS_C",
    "RELOADING_C",
    "TURRET_ROTATE",
    "POST_COLLAPSE",
    "MOVING",
    "DYING",
    "AWAITING_CONSTRUCTION",
    "PARTIALLY_CONSTRUCTED",
    "ACTIVELY_BEING_CONSTRUCTED",
    "PRONE",
    "FREEFALL",
    "ACTIVELY_CONSTRUCTING",
    "CONSTRUCTION_COMPLETE",
    "RADAR_EXTENDING",
    "RADAR_UPGRADED",
    "PANICKING",
    "AFLAME",
    "SMOLDERING",
    "BURNED",
    "DOCKING",
    "DOCKING_BEGINNING",
    "DOCKING_ACTIVE",
    "DOCKING_ENDING",
    "CARRYING",
    "FLOODED",
    "LOADED",
    "JETAFTERBURNER",
    "JETEXHAUST",
    "PACKING",
    "UNPACKING",
    "DEPLOYED",
    "OVER_WATER",
    "POWER_PLANT_UPGRADED",
    "CLIMBING",
    "SOLD",
    "RAPPELLING",
    "ARMED",
    "POWER_PLANT_UPGRADING",
    "SPECIAL_CHEERING",
    "CONTINUOUS_FIRE_SLOW",
    "CONTINUOUS_FIRE_MEAN",
    "CONTINUOUS_FIRE_FAST",
    "RAISING_FLAG",
    "CAPTURED",
    "EXPLODED_FLAILING",
    "EXPLODED_BOUNCING",
    "SPLATTED",
    "USING_WEAPON_A",
    "USING_WEAPON_B",
    "USING_WEAPON_C",
    "PREORDER",
    "CENTER_TO_LEFT",
    "LEFT_TO_CENTER",
    "CENTER_TO_RIGHT",
    "RIGHT_TO_CENTER",
    "RIDER1",
    "RIDER2",
    "RIDER3",
    "RIDER4",
    "RIDER5",
    "RIDER6",
    "RIDER7",
    "RIDER8",
    "STUNNED_FLAILING",
    "STUNNED",
    "SECOND_LIFE",
    "JAMMED",
    "ARMORSET_CRATEUPGRADE_ONE",
    "ARMORSET_CRATEUPGRADE_TWO",
    "USER_1",
    "USER_2",
    "DISGUISED",
    nullptr,
};

// TODO Temp to force instantiation. Fixes issue with Parameter::Read_Parameter.
// template class BitFlags<KIND_OF_COUNT>;
