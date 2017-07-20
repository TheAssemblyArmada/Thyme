////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMETYPE.CPP
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
#include "gametype.h"

const char *g_timeOfDayNames[TIME_OF_DAY_COUNT + 1] = {
    "NONE",
    "MORNING",
    "AFTERNOON",
    "EVENING",
    "NIGHT",
    nullptr
};

const char *g_weatherNames[WEATHER_COUNT + 1]{ "NORMAL", "SNOWY", nullptr };
const char *g_bodyDamageNames[BODY_COUNT + 1] = { "PRISTINE", "DAMAGED", "REALLYDAMAGED", "RUBBLE", nullptr };

const char *g_speakerTypes[SPEAKERS_COUNT + 1] = {
    "2 Speakers",
    "Headphones",
    "Surround Sound",
    "4 Speaker",
    "5.1 Surround",
    "7.1 Surround",
    nullptr
};
