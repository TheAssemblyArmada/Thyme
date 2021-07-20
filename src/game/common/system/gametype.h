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

// Placeholder
enum ObjectID : int32_t
{
    OBJECT_UNK,
};

enum DrawableID : uint32_t
{
    DRAWABLE_UNK,
};

enum PathfindLayerEnum
{
    LAYER_UNK,
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
    WEAPONSLOT_PRIMARY,
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
};

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
extern const char *g_particlePriorityNames[PARTICLE_PRIORITY_COUNT + 1];
