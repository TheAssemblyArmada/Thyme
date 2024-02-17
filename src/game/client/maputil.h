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

#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "ini.h"
#include "unicodestring.h"
#include <list>
#include <map>
#include <set>

struct FileInfo;
class GameWindow;
class Image;

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

class MapMetaData
{
public:
    Utf16String m_displayName;
    Utf8String m_lookupTag;
    Region3D m_extent;
    int m_numPlayers;
    bool m_isMultiplayer;
    bool m_isOfficial;
    unsigned int m_fileSize;
    unsigned int m_CRC;
    WinTimeStamp m_timestamp;
    WaypointMap m_waypoints;
    std::list<Coord3D> m_supplyPositions;
    std::list<Coord3D> m_techPositions;
    Utf8String m_mapFilename;
};

class MapMetaDataReader
{
public:
    static const FieldParse *Get_Field_Parse();
    static const FieldParse s_mapFieldParseTable[];
    Region3D m_extent;
    int m_numPlayers;
    bool m_isMultiplayer;
    Utf8String m_displayName;
    Utf8String m_nameLookupTag;
    bool m_isOfficial;
    WinTimeStamp m_timestamp;
    unsigned int m_filesize;
    unsigned int m_CRC;
    Coord3D m_waypoints[8];
    Coord3D m_initialCameraPosition;
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
    MapMetaData *Find_Map(Utf8String map_name);
    bool Add_Map(Utf8String dir, Utf8String name, FileInfo *info, bool build_cache);
    static void Parse_Map_Cache_Definition(INI *ini);

private:
    static const char *const s_mapDirName;
    static const char *const s_mapExtension;
    static const char *const s_mapCacheName;

    std::map<Utf8String, bool> m_seen;
    std::set<Utf8String> m_unk;
};

bool Would_Map_Transfer(const Utf8String &path);
int Populate_Map_List_Box_No_Reset(GameWindow *listbox, bool is_official, bool is_multiplayer, Utf8String map_name);
int Populate_Map_List_Box(GameWindow *listbox, bool is_official, bool is_multiplayer, Utf8String map_name);
bool Is_Valid_Map(Utf8String map_name, bool is_multiplayer);
Utf8String Get_Default_Map(bool is_multiplayer);
Utf8String Get_Official_Multiplayer_Map();
bool Is_Map_Official(Utf8String map_name);
Image *Get_Map_Preview_Image(Utf8String map_name);
void Find_Draw_Positions(int start_x, int start_y, int width, int height, Region3D extent, ICoord2D *ul, ICoord2D *lr);

#ifdef GAME_DLL
extern WaypointMap *&g_waypoints;
extern MapCache *&g_theMapCache;
#else
extern WaypointMap *g_waypoints;
extern MapCache *g_theMapCache;
#endif
