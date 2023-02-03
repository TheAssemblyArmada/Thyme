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

#include "asciistring.h"
#include "coord.h"
#include "list"
#include <map>
#include <set>

class WaypointMap : public std::map<Utf8String, Coord3D>
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
    Utf8String m_displayName;
    Utf8String m_lookupTag;
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

class MapCache : public std::map<Utf8String, MapMetaData>
{
public:
    MapCache() {}
    ~MapCache() {}

    Utf8String Get_Map_Dir() { return s_mapDirName; }
    Utf8String Get_Map_Extension() { return s_mapExtension; }
    Utf8String Get_Cache_Name() { return s_mapCacheName; }
    Utf8String Get_User_Map_Dir();

    void Write_Cache_INI(bool custom_cache);
    void Update_Cache();
    bool Clear_Unseen_Maps(Utf8String prefix);
    void Load_Standard_Maps();
    bool Load_User_Maps();

private:
    static const char *const s_mapDirName;
    static const char *const s_mapExtension;
    static const char *const s_mapCacheName;

    std::map<Utf8String, bool> m_seen;
    std::set<Utf8String> m_unk;
};

#ifdef GAME_DLL
extern WaypointMap *&g_waypoints;
extern MapCache *&g_theMapCache;
#else
extern WaypointMap *g_waypoints;
extern MapCache *g_theMapCache;
#endif
