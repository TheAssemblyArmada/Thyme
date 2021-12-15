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
#include "gametype.h"

const char *g_timeOfDayNames[TIME_OF_DAY_COUNT + 1] = {
    "NONE",
    "MORNING",
    "AFTERNOON",
    "EVENING",
    "NIGHT",
    nullptr,
};

const char *g_weatherNames[WEATHER_COUNT + 1]{
    "NORMAL",
    "SNOWY",
    nullptr,
};
const char *g_bodyDamageNames[BODY_COUNT + 1] = {
    "PRISTINE",
    "DAMAGED",
    "REALLYDAMAGED",
    "RUBBLE",
    nullptr,
};

const char *g_speakerTypes[SPEAKERS_COUNT + 1] = {
    "2 Speakers",
    "Headphones",
    "Surround Sound",
    "4 Speaker",
    "5.1 Surround",
    "7.1 Surround",
    nullptr,
};

const char *g_audioPriorityNames[PRIORITY_COUNT + 1] = {
    "LOWEST",
    "LOW",
    "NORMAL",
    "HIGH",
    "CRITICAL",
    nullptr,
};

const char *g_buildableStatusNames[BSTATUS_COUNT + 1] = {
    "Yes",
    "Ignore_Prerequisites",
    "No",
    "Only_By_AI",
    nullptr,
};

const char *g_surfaceNames[SURFACE_COUNT + 1] = {
    "Ground",
    "Air",
    "Ground or Air",
    nullptr,
};

const char *g_shakeIntensityNames[SHAKE_COUNT + 1] = {
    "Subtle",
    "Normal",
    "Strong",
    "Severe",
    "Cine_Extreme",
    "Cine_Insane",
    nullptr,
};

const char *g_weaponSlotNames[WEAPONSLOT_COUNT + 1] = {
    "PRIMARY",
    "SECONDARY",
    "TERTIARY",
    nullptr,
};

const char *g_commandSourceMaskNames[COMMANDSOURCE_COUNT + 1] = {
    "FROM_PLAYER",
    "FROM_SCRIPT",
    "FROM_AI",
    "FROM_DOZER",
    "DEFAULT_SWITCH_WEAPON",
    nullptr,
};

const char *g_veterancyNames[VETERANCY_COUNT + 1] = {
    "REGULAR",
    "VETERAN",
    "ELITE",
    "HEROIC",
    nullptr,
};
