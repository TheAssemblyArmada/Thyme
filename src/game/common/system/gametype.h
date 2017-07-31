////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMETYPE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Couple of datatypes and matching name arrays.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef GAMETYPE_H
#define GAMETYPE_H

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

extern const char *g_timeOfDayNames[TIME_OF_DAY_COUNT + 1];
extern const char *g_weatherNames[WEATHER_COUNT + 1];
extern const char *g_bodyDamageNames[BODY_COUNT + 1];
extern const char *g_speakerTypes[SPEAKERS_COUNT + 1];
extern const char *g_audioPriorityNames[PRIORITY_COUNT + 1];

#endif
