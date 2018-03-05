/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Map file handling utility functions.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef MAPUTIL_H
#define MAPUTIL_H

#include "asciistring.h"
#include "coord.h"
#include "list"
#include <map>

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
    static const char *const s_mapDirName;
    static const char *const s_mapExtension;
    static const char *const s_mapCacheName;

    std::map<AsciiString, int> m_seen;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

extern WaypointMap *&g_waypoints;
extern MapCache *&g_theMapCache;
#else
extern WaypointMap *g_waypoints;
extern MapCache *g_theMapCache;
#endif

#endif // MAPUTIL_H
