/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief KindOf enum and string listings.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "kindof.h"
#include "bitflags.h"

// Mapping of bits to names... probably not a great idea having it based
// on the template param incase of collision if it needs extending later,
// but this is what the original looks like it did.
// Maps what each bit flags.
template<>
const char *BitFlags<KINDOF_COUNT>::s_bitNamesList[]{ "OBSTACLE",
    "SELECTABLE",
    "IMMOBILE",
    "CAN_ATTACK",
    "STICK_TO_TERRAIN_SLOPE",
    "CAN_CAST_REFLECTIONS",
    "SHRUBBERY",
    "STRUCTURE",
    "INFANTRY",
    "VEHICLE",
    "AIRCRAFT",
    "HUGE_VEHICLE",
    "DOZER",
    "HARVESTER",
    "COMMANDCENTER",
    "LINEBUILD",
    "SALVAGER",
    "WEAPON_SALVAGER",
    "TRANSPORT",
    "BRIDGE",
    "LANDMARK_BRIDGE",
    "BRIDGE_TOWER",
    "PROJECTILE",
    "PRELOAD",
    "NO_GARRISON",
    "WAVEGUIDE",
    "WAVE_EFFECT",
    "NO_COLLIDE",
    "REPAIR_PAD",
    "HEAL_PAD",
    "STEALTH_GARRISON",
    "CASH_GENERATOR",
    "DRAWABLE_ONLY",
    "MP_COUNT_FOR_VICTORY",
    "REBUILD_HOLE",
    "SCORE",
    "SCORE_CREATE",
    "SCORE_DESTROY",
    "NO_HEAL_ICON",
    "CAN_RAPPEL",
    "PARACHUTABLE",
    "CAN_BE_REPULSED",
    "MOB_NEXUS",
    "IGNORED_IN_GUI",
    "CRATE",
    "CAPTURABLE",
    "CLEARED_BY_BUILD",
    "SMALL_MISSILE",
    "ALWAYS_VISIBLE",
    "UNATTACKABLE",
    "MINE",
    "CLEANUP_HAZARD",
    "PORTABLE_STRUCTURE",
    "ALWAYS_SELECTABLE",
    "ATTACK_NEEDS_LINE_OF_SIGHT",
    "WALK_ON_TOP_OF_WALL",
    "DEFENSIVE_WALL",
    "FS_POWER",
    "FS_FACTORY",
    "FS_BASE_DEFENSE",
    "FS_TECHNOLOGY",
    "AIRCRAFT_PATH_AROUND",
    "LOW_OVERLAPPABLE",
    "FORCEATTACKABLE",
    "AUTO_RALLYPOINT",
    "TECH_BUILDING",
    "POWERED",
    "PRODUCED_AT_HELIPAD",
    "DRONE",
    "CAN_SEE_THROUGH_STRUCTURE",
    "BALLISTIC_MISSILE",
    "CLICK_THROUGH",
    "SUPPLY_SOURCE_ON_PREVIEW",
    "PARACHUTE",
    "GARRISONABLE_UNTIL_DESTROYED",
    "BOAT",
    "IMMUNE_TO_CAPTURE",
    "HULK",
    "SHOW_PORTRAIT_WHEN_CONTROLLED",
    "SPAWNS_ARE_THE_WEAPONS",
    "CANNOT_BUILD_NEAR_SUPPLIES",
    "SUPPLY_SOURCE",
    "REVEAL_TO_ALL",
    "DISGUISER",
    "INERT",
    "HERO",
    "IGNORES_SELECT_ALL",
    "DONT_AUTO_CRUSH_INFANTRY",
    "CLIFF_JUMPER",
    "FS_SUPPLY_DROPZONE",
    "FS_SUPERWEAPON",
    "FS_BLACK_MARKET",
    "FS_SUPPLY_CENTER",
    "FS_STRATEGY_CENTER",
    "MONEY_HACKER",
    "ARMOR_SALVAGER",
    "REVEALS_ENEMY_PATHS",
    "BOOBY_TRAP",
    "FS_FAKE",
    "FS_INTERNET_CENTER",
    "BLAST_CRATER",
    "PROP",
    "OPTIMIZED_TREE",
    "FS_ADVANCED_TECH",
    "FS_BARRACKS",
    "FS_WARFACTORY",
    "FS_AIRFIELD",
    "AIRCRAFT_CARRIER",
    "NO_SELECT",
    "REJECT_UNMANNED",
    "CANNOT_RETALIATE",
    "TECH_BASE_DEFENSE",
    "EMP_HARDENED",
    "DEMOTRAP",
    "CONSERVATIVE_BUILDING",
    "IGNORE_DOCKING_BONES",
    nullptr };

BitFlags<KINDOF_COUNT> KINDOFMASK_NONE;

#ifndef GAME_DLL
BitFlags<KINDOF_COUNT> KINDOFMASK_FS;
#endif

void Init_KindOf_Masks()
{
    KINDOFMASK_FS.Set(KINDOF_FS_FACTORY, true);
    KINDOFMASK_FS.Set(KINDOF_FS_BASE_DEFENSE, true);
    KINDOFMASK_FS.Set(KINDOF_FS_TECHNOLOGY, true);
    KINDOFMASK_FS.Set(KINDOF_FS_SUPPLY_DROPZONE, true);
    KINDOFMASK_FS.Set(KINDOF_FS_SUPERWEAPON, true);
    KINDOFMASK_FS.Set(KINDOF_FS_BLACK_MARKET, true);
    KINDOFMASK_FS.Set(KINDOF_FS_SUPPLY_CENTER, true);
    KINDOFMASK_FS.Set(KINDOF_FS_STRATEGY_CENTER, true);
    KINDOFMASK_FS.Set(KINDOF_FS_FAKE, true);
    KINDOFMASK_FS.Set(KINDOF_FS_INTERNET_CENTER, true);
    KINDOFMASK_FS.Set(KINDOF_FS_ADVANCED_TECH, true);
    KINDOFMASK_FS.Set(KINDOF_FS_BARRACKS, true);
    KINDOFMASK_FS.Set(KINDOF_FS_WARFACTORY, true);
    KINDOFMASK_FS.Set(KINDOF_FS_AIRFIELD, true);
}
