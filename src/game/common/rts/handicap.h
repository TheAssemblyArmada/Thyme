////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: HANDICAP.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: TODO.
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

class Dict;
class ThingTemplate;

class Handicap
{
public:
    enum HandicapType
    {
        BUILDCOST = 0,
        BUILDTIME,
        HANDICAP_TYPE_COUNT,
    };

    enum ThingType
    {
        NONE = 0,
        GENERIC,
        POLYGON,
    };

    Handicap();

    void Init();
    void Read_From_Dict(Dict const *dict);
    ThingType Get_Best_Thing_Type(ThingTemplate const *thing);
    float Get_Handicap(HandicapType ht, ThingTemplate const *thing);

private:
    float m_handicaps[HANDICAP_TYPE_COUNT][POLYGON];
};