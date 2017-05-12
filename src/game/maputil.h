////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MAPUTIL.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Map file handling utility functions.
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

#ifndef MAPUTIL_H
#define MAPUTIL_H

#include "asciistring.h"
#include "coord.h"
#include "hooker.h"
#include "list"
#include <map>

// Part of some static class?
#define g_waypoints (Make_Global<WaypointMap*>(0x00945AD4))
#define g_keyInitialCameraPosition (Make_Global<StaticNameKey>(0x00A3A880))
#define TheMapCache (Make_Global<MapCache*>(0x00A2B974))

class WaypointMap : public std::map<AsciiString, Coord3D>
{
public:
    void Update();
public:
    int m_numStartSpots;
};

struct WinTimeStamp
{
    unsigned int m_lowTimeStamp;
    unsigned int m_highTimeStamp;
};

struct MapMetaData
{
    AsciiString m_displayName;
    AsciiString m_lookupTag;
    Region3D m_extent;
    int m_numPlayers;
    bool m_isMultiplayer;
    bool m_isOfficial;
    unsigned int m_filesize;
    unsigned int m_CRC;
    WinTimeStamp m_timestamp;
    WaypointMap m_waypoints;
    std::list<Coord3D> m_supplyPositions;
    std::list<Coord3D> m_techPositions;
};

class MapCache : std::map<AsciiString, MapMetaData>
{
public:
    AsciiString Get_Map_Dir() { return s_mapDirName; }
    AsciiString Get_Map_Extension() { return s_mapExtension; }
    AsciiString Get_Cache_Name() { return s_mapCacheName; }
    AsciiString Get_User_Map_Dir();

    void Write_Cache_INI(bool custom_cache);
    void Update_Cache();
    bool Clear_Unseen_Maps(AsciiString prefix);
    void Load_Standard_Maps();
    bool Load_User_Maps();

private:
    static char const *const s_mapDirName;
    static char const *const s_mapExtension;
    static char const *const s_mapCacheName;

    std::map<AsciiString, int> m_seen;
};

#endif // MAPUTIL_H
