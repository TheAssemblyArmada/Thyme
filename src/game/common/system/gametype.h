/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Couple of datatypes and matching name arrays.
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

enum TimeOfDayType : int32_t
{
    TIME_OF_DAY_INVALID = 0,
    TIME_OF_DAY_FIRST = 1,
    TIME_OF_DAY_MORNING = 1,
    TIME_OF_DAY_AFTERNOON,
    TIME_OF_DAY_EVENING,
    TIME_OF_DAY_NIGHT,
    TIME_OF_DAY_COUNT,
};

enum WeatherType : int32_t
{
    WEATHER_NORMAL,
    WEATHER_SNOWY,
    WEATHER_COUNT,
};

enum BodyDamageType : int32_t
{
    BODY_PRISTINE,
    BODY_DAMAGED,
    BODY_REALLYDAMAGED,
    BODY_RUBBLE,
    BODY_COUNT,
};

enum SpeakerType : int32_t
{
    SPEAKERS_TWO,
    SPEAKERS_HEADPHONES,
    SPEAKERS_SUROUND,
    SPEAKERS_FOUR,
    SPEAKERS_FIVE_P_ONE,
    SPEAKERS_SEVEN_P_ONE,
    SPEAKERS_COUNT,
};

enum PriorityType : int32_t
{
    PRIORITY_LOWEST,
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
    PRIORITY_CRITICAL,
    PRIORITY_COUNT,
};

enum ScienceType : int32_t
{
    SCIENCE_INVALID = -1,
    SCIENCE_BOGUS = 0,
};

// Placeholder
enum ObjectID : uint32_t
{
    INVALID_OBJECT_ID,
};

DEFINE_ENUMERATION_OPERATORS_T(ObjectID, uint32_t);

enum DrawableID : uint32_t
{
    INVALID_DRAWABLE_ID,
};

enum DrawableStatus
{
    DRAWABLE_STATUS_UNK = 0,
    DRAWABLE_STATUS_DRAWS_IN_MIRROR = 1 << 0, // draws in mirror reflections
    DRAWABLE_STATUS_SHADOWS_ENABLED = 1 << 1, // shadows are enabled
    DRAWABLE_STATUS_AMBIENT_LIGHT_LOCKED = 1 << 2, // ambient light is being used, if not set a tint envelope is applied
    DRAWABLE_STATUS_8 = 1 << 3, // newDrawable called with this in handleBuildPlacements and placeBuildAvailable
    DRAWABLE_STATUS_16 = 1 << 4, // set in showRallyPoint
    DRAWABLE_STATUS_32 = 1 << 5, // may not exist
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(DrawableStatus);

enum PathfindLayerEnum
{
    LAYER_INVALID,
    LAYER_GROUND,
    LAYER_BRIDGE_1,
    LAYER_BRIDGE_2,
    LAYER_BRIDGE_3,
    LAYER_BRIDGE_4,
    LAYER_BRIDGE_5,
    LAYER_BRIDGE_6,
    LAYER_BRIDGE_7,
    LAYER_BRIDGE_8,
    LAYER_BRIDGE_9,
    LAYER_BRIDGE_10,
    LAYER_BRIDGE_11,
    LAYER_BRIDGE_12,
    LAYER_BRIDGE_13,
    LAYER_WALLS,
    LAYER_COUNT,
};

enum BuildableStatus
{
    BSTATUS_YES,
    BSTATUS_IGNORE_PREREQUISITES,
    BSTATUS_NO,
    BSTATUS_ONLY_AI,
    BSTATUS_COUNT,
};

enum Surfaces
{
    SURFACE_GROUND,
    SURFACE_AIR,
    SURFACE_GROUND_AIR,
    SURFACE_COUNT,
};

enum ShakeIntensities
{
    SHAKE_SUBTLE,
    SHAKE_NORMAL,
    SHAKE_STRONG,
    SHAKE_SEVERE,
    SHAKE_EXTREME,
    SHAKE_INSANE,
    SHAKE_COUNT,
};

enum WeaponSlotType
{
    WEAPONSLOT_UNK = -1,
    WEAPONSLOT_PRIMARY = 0,
    WEAPONSLOT_SECONDARY,
    WEAPONSLOT_TERTIARY,
    WEAPONSLOT_COUNT,
};

enum CommandSourceType
{
    COMMANDSOURCE_PLAYER,
    COMMANDSOURCE_SCRIPT,
    COMMANDSOURCE_AI,
    COMMANDSOURCE_DOZER,
    COMMANDSOURCE_DEFAULT_SWITCH_WEAPON,
    COMMANDSOURCE_COUNT,
};

enum VeterancyLevel
{
    VETERANCY_REGULAR,
    VETERANCY_VETERAN,
    VETERANCY_ELITE,
    VETERANCY_HEROIC,
    VETERANCY_COUNT,
    VETERANCY_INVALID,
};
DEFINE_ENUMERATION_OPERATORS(VeterancyLevel);

enum ParticlePriorityType : int32_t
{
    PARTICLE_PRIORITY_NONE,
    PARTICLE_PRIORITY_LOWEST,
    PARTICLE_PRIORITY_WEAPON_EXPLOSION = 1,
    PARTICLE_PRIORITY_SCORCHMARK,
    PARTICLE_PRIORITY_DUST_TRAIL,
    PARTICLE_PRIORITY_BUILDUP,
    PARTICLE_PRIORITY_DEBRIS_TRAIL,
    PARTICLE_PRIORITY_UNIT_DAMAGE_FX,
    PARTICLE_PRIORITY_DEATH_EXPLOSION,
    PARTICLE_PRIORITY_SEMI_CONSTANT,
    PARTICLE_PRIORITY_CONSTANT,
    PARTICLE_PRIORITY_WEAPON_TRAIL,
    PARTICLE_PRIORITY_AREA_EFFECT,
    PARTICLE_PRIORITY_CRITICAL,
    PARTICLE_PRIORITY_ALWAYS_RENDER,
    PARTICLE_PRIORITY_COUNT,
};
DEFINE_ENUMERATION_OPERATORS(ParticlePriorityType);

enum CellShroudStatus
{
    SHROUD_STATUS_CLEAR,
    SHROUD_STATUS_FOG,
    SHROUD_STATUS_SHROUD,
};

enum TerrainDecalType // names not 100% confirmed yet
{
    TERRAIN_DECAL_DEMORALIZED_OBSOLETE,
    TERRAIN_DECAL_HORDE_INFANTRY, // references EXHorde texture
    TERRAIN_DECAL_NATIONALISM, // references EXHorde_UP texture
    TERRAIN_DECAL_HORDE_VEHICLE, // references EXHordeB texture
    TERRAIN_DECAL_4, // unknown, references EXHordeB_UP texture
    TERRAIN_DECAL_JUNK_CRATE, // references EXJunkCrate texture
    TERRAIN_DECAL_FANATICISM, // references EXHordeC_UP texture
    TERRAIN_DECAL_CHEMICAL_SUITS, // references EXChemSuit texture
    TERRAIN_DECAL_8, // unknown, references blank texture
    TERRAIN_DECAL_9, // unknown, references blank texture
    TERRAIN_DECAL_COUNT,
};

enum StealthLookType
{
    STEALTHLOOK_NONE = 0,
    STEALTHLOOK_VISIBLE_FRIENDLY,
    STEALTHLOOK_DISGUISED,
    STEALTHLOOK_VISIBLE_DETECTED,
    STEALTHLOOK_VISIBLE_DETECTED_FRIENDLY,
    STEALTHLOOK_INVISIBLE
};

enum GameMode
{
    GAME_SINGLE_PLAYER,
    GAME_LAN,
    GAME_SKIRMISH,
    GAME_REPLAY,
    GAME_SHELL,
    GAME_INTERNET,
    GAME_NONE,
};

enum GameDifficulty
{
    DIFFICULTY_EASY,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD,
    DIFFICULTY_COUNT,
};

enum Relationship
{
    ENEMIES,
    NEUTRAL,
    ALLIES,
};

extern const char *g_timeOfDayNames[TIME_OF_DAY_COUNT + 1];
extern const char *g_weatherNames[WEATHER_COUNT + 1];
extern const char *g_bodyDamageNames[BODY_COUNT + 1];
extern const char *g_speakerTypes[SPEAKERS_COUNT + 1];
extern const char *g_audioPriorityNames[PRIORITY_COUNT + 1];
extern const char *g_buildableStatusNames[BSTATUS_COUNT + 1];
extern const char *g_surfaceNames[SURFACE_COUNT + 1];
extern const char *g_shakeIntensityNames[SHAKE_COUNT + 1];
extern const char *g_weaponSlotNames[WEAPONSLOT_COUNT + 1];
extern const char *g_commandSourceMaskNames[COMMANDSOURCE_COUNT + 1];
extern const char *g_veterancyNames[VETERANCY_COUNT + 1];

constexpr int32_t MAX_PLAYER_COUNT = 16;
