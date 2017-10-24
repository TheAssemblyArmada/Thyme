////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MAPUTIL.CPP
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
#include "maputil.h"
#include "namekeygenerator.h"
#include "minmax.h"
#include "globaldata.h"

const char *const MapCache::s_mapDirName = "Maps";
const char *const MapCache::s_mapExtension = "map";
const char *const MapCache::s_mapCacheName = "MapCache.ini";

// Updates a waypoint map from the global waypoints map with initial cam position
// and player start waypoints.
void WaypointMap::Update()
{
    if ( g_waypoints != nullptr ) {
        clear();
        AsciiString key_name = g_theNameKeyGenerator->Key_To_Name(g_keyInitialCameraPosition);
        auto it = g_waypoints->find(key_name);

        if ( it != g_waypoints->end() ) {
            (*this)[key_name] = it->second;
        }

        m_numStartSpots = 0;

        for ( int i = 1; i <= 8; ++i ) {
            key_name.Format("Player_%d_Start", i);
            it = g_waypoints->find(key_name);

            if ( it != g_waypoints->end() ) {
                break;
            }

            (*this)[key_name] = it->second;
            ++m_numStartSpots;
        }
        
        m_numStartSpots = Max(m_numStartSpots, 1);
    } else {
        m_numStartSpots = 1;
    }
}

AsciiString MapCache::Get_User_Map_Dir()
{
    AsciiString dir = g_theWriteableGlobalData->m_userDataDirectory;
    dir += Get_Map_Dir();

    return dir;
}
