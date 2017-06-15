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

const char *g_timeOfDayNames[TIME_OF_DAY_COUNT] = {
    "NONE",
    "MORNING",
    "AFTERNOON",
    "EVENING",
    "NIGHT"
};

const char *g_weatherNames[WEATHER_COUNT]{ "NORMAL", "SNOWY" };
