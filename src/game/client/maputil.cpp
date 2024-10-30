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
#include "maputil.h"
#include "cachedfileinputstream.h"
#include "colorspace.h"
#include "crc.h"
#include "datachunk.h"
#include "dict.h"
#include "filesystem.h"
#include "fpusetting.h"
#include "gadgetlistbox.h"
#include "gamestate.h"
#include "gametext.h"
#include "globaldata.h"
#include "image.h"
#include "localfilesystem.h"
#include "mapobject.h"
#include "quotedprintable.h"
#include "skirmishbattlehonors.h"
#include "staticnamekey.h"
#include "thingfactory.h"
#include <algorithm>

#ifndef GAME_DLL
WaypointMap *g_waypoints = nullptr;
MapCache *g_theMapCache = nullptr;
#endif

static std::vector<ICoord2D> s_boundaries;
static Dict s_worldDict;
static std::list<Coord3D> s_supplyPositions;
static std::list<Coord3D> s_techPositions;
static unsigned int s_width = 0;
static unsigned int s_height = 0;
static int s_borderSize = 0;
static int s_mapDX = 0;
static int s_mapDY = 0;

const char *const MapCache::s_mapDirName = "Maps";
const char *const MapCache::s_mapExtension = "map";
const char *const MapCache::s_mapCacheName = "MapCache.ini";

void Parse_Supply_Position_Coord3D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord3D c;
    INI::Parse_Coord3D(ini, nullptr, &c, nullptr);
    static_cast<MapMetaDataReader *>(formal)->m_supplyPositions.push_back(c);
}

void Parse_Tech_Position_Coord3D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord3D c;
    INI::Parse_Coord3D(ini, nullptr, &c, nullptr);
    static_cast<MapMetaDataReader *>(formal)->m_techPositions.push_back(c);
}

// clang-format off
const FieldParse MapMetaDataReader::s_mapFieldParseTable[] = {
    {"isOfficial", &INI::Parse_Bool, nullptr, offsetof(MapMetaDataReader, m_isOfficial)},
    {"isMultiplayer", &INI::Parse_Bool, nullptr, offsetof(MapMetaDataReader, m_isMultiplayer)},
    {"extentMin", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_extent.lo)},
    {"extentMax", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_extent.hi)},
    {"numPlayers", &INI::Parse_Int, nullptr, offsetof(MapMetaDataReader, m_numPlayers)},
    {"fileSize", &INI::Parse_Unsigned_Int, nullptr, offsetof(MapMetaDataReader, m_filesize)},
    {"fileCRC", &INI::Parse_Unsigned_Int, nullptr, offsetof(MapMetaDataReader, m_CRC)},
    {"timestampLo", &INI::Parse_Int, nullptr, offsetof(MapMetaDataReader, m_timestamp.m_lowTimeStamp)},
    {"timestampHi", &INI::Parse_Int, nullptr, offsetof(MapMetaDataReader, m_timestamp.m_highTimeStamp)},
    {"displayName", &INI::Parse_AsciiString, nullptr, offsetof(MapMetaDataReader, m_displayName)},
    {"nameLookupTag", &INI::Parse_AsciiString, nullptr, offsetof(MapMetaDataReader, m_nameLookupTag)},
    {"supplyPosition", &Parse_Supply_Position_Coord3D, nullptr, 0},
    {"techPosition", &Parse_Tech_Position_Coord3D, nullptr, 0},
    {"Player_1_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[0])},
    {"Player_2_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[1])},
    {"Player_3_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[2])},
    {"Player_4_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[3])},
    {"Player_5_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[4])},
    {"Player_6_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[5])},
    {"Player_7_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[6])},
    {"Player_8_Start", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_waypoints[7])},
    {"InitialCameraPosition", &INI::Parse_Coord3D, nullptr, offsetof(MapMetaDataReader, m_initialCameraPosition)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

// Updates a waypoint map from the global waypoints map with initial cam position
// and player start waypoints.
void WaypointMap::Update()
{
    if (g_waypoints != nullptr) {
        clear();
        Utf8String key_name = g_theNameKeyGenerator->Key_To_Name(g_theInitialCameraPositionKey);
        auto it = g_waypoints->find(key_name);

        if (it != g_waypoints->end()) {
            (*this)[key_name] = it->second;
        }

        m_numStartSpots = 0;

        for (int i = 1; i <= 8; ++i) {
            key_name.Format("Player_%d_Start", i);
            it = g_waypoints->find(key_name);

            if (it == g_waypoints->end()) {
                break;
            }

            (*this)[key_name] = it->second;
            ++m_numStartSpots;
        }

        m_numStartSpots = std::max(m_numStartSpots, 1);
    } else {
        m_numStartSpots = 1;
    }
}

Utf8String MapCache::Get_User_Map_Dir()
{
    Utf8String dir = g_theWriteableGlobalData->m_userDataDirectory;
    dir += Get_Map_Dir();

    return dir;
}

void MapCache::Write_Cache_INI(bool custom_cache)
{
    Utf8String user_dir;

    if (custom_cache && !g_theWriteableGlobalData->m_buildMapCache) {
        user_dir = Get_User_Map_Dir();
    } else {
        user_dir = Get_Map_Dir();
    }

    Utf8String name(user_dir);
    name.Concat('\\');

    g_theFileSystem->Create_Directory(user_dir);
    name.Concat("MapCache.ini");
    FILE *file = fopen(name.Str(), "w");
    captainslog_dbgassert(file != nullptr, "Failed to create %s", name.Str());

    if (file != nullptr) {
        fprintf(file, "; FILE: %s /////////////////////////////////////////////////////////////\n", name.Str());
        fprintf(file, "; This INI file is auto-generated - do not modify\n");
        fprintf(file, "; /////////////////////////////////////////////////////////////////////////////\n");
        user_dir.To_Lower();

        MapMetaData data;
        for (auto it = begin(); it != end(); it++) {
            if (it->first.Starts_With_No_Case(user_dir.Str())) {
                data = it->second;
                fprintf(file, "\nMapCache %s\n", Ascii_String_To_Quoted_Printable(it->first.Str()).Str());
                fprintf(file, "  fileSize = %u\n", data.m_fileSize);
                fprintf(file, "  fileCRC = %u\n", data.m_CRC);
                fprintf(file, "  timestampLo = %d\n", data.m_timestamp.m_lowTimeStamp);
                fprintf(file, "  timestampHi = %d\n", data.m_timestamp.m_highTimeStamp);
                fprintf(file, "  isOfficial = %s\n", data.m_isOfficial ? "yes" : "no");
                fprintf(file, "  isMultiplayer = %s\n", data.m_isMultiplayer ? "yes" : "no");
                fprintf(file, "  numPlayers = %d\n", data.m_numPlayers);
                fprintf(file,
                    "  extentMin = X:%2.2f Y:%2.2f Z:%2.2f\n",
                    data.m_extent.lo.x,
                    data.m_extent.lo.y,
                    data.m_extent.lo.z);
                fprintf(file,
                    "  extentMax = X:%2.2f Y:%2.2f Z:%2.2f\n",
                    data.m_extent.hi.x,
                    data.m_extent.hi.y,
                    data.m_extent.hi.z);
                fprintf(file, "  nameLookupTag = %s\n", data.m_lookupTag.Str());

                for (auto waypoint = data.m_waypoints.begin(); waypoint != data.m_waypoints.end(); waypoint++) {
                    fprintf(file,
                        "  %s = X:%2.2f Y:%2.2f Z:%2.2f\n",
                        waypoint->first.Str(),
                        waypoint->second.x,
                        waypoint->second.y,
                        waypoint->second.z);
                }

                for (auto tech_position = data.m_techPositions.begin(); tech_position != data.m_techPositions.end();
                     tech_position++) {
                    fprintf(file,
                        "  techPosition = X:%2.2f Y:%2.2f Z:%2.2f\n",
                        (*tech_position).x,
                        (*tech_position).y,
                        (*tech_position).z);
                }

                for (auto supply_position = data.m_supplyPositions.begin(); supply_position != data.m_supplyPositions.end();
                     supply_position++) {
                    fprintf(file,
                        "  supplyPosition = X:%2.2f Y:%2.2f Z:%2.2f\n",
                        (*supply_position).x,
                        (*supply_position).y,
                        (*supply_position).z);
                }

                fprintf(file, "END\n\n");
            }
        }

        fclose(file);
    }
}

void MapCache::Update_Cache()
{
    Set_FP_Mode();
    g_theFileSystem->Create_Directory(g_theMapCache->Get_User_Map_Dir());

    if (Load_User_Maps()) {
        Write_Cache_INI(true);
    }

    Load_Standard_Maps();

    if (g_theLocalFileSystem->Does_File_Exist(Get_Map_Dir().Str())) {
        bool build = g_theWriteableGlobalData->m_buildMapCache;
        g_theWriteableGlobalData->m_buildMapCache = true;
        Load_User_Maps();
        g_theWriteableGlobalData->m_buildMapCache = build;
        Write_Cache_INI(false);
    }
}

bool MapCache::Clear_Unseen_Maps(Utf8String prefix)
{
    prefix.To_Lower();
    bool found = false;

    for (auto it = m_seen.begin(); it != m_seen.end(); it++) {
        Utf8String name = it->first;

        if (!it->second) {
            if (name.Starts_With_No_Case(prefix.Str())) {
                erase(name);
                found = true;
            }
        }
    }

    return found;
}

void MapCache::Load_Standard_Maps()
{
    INI ini;
    Utf8String name;
    name.Format("%s\\%s", Get_Map_Dir().Str(), "MapCache.ini");
    File *file = g_theFileSystem->Open_File(name.Str(), File::READ);

    if (file != nullptr) {
        file->Close();
        file = nullptr;
        ini.Load(name, INI_LOAD_OVERWRITE, nullptr);
    }
}

bool MapCache::Load_User_Maps()
{
    Utf8String map_root;

    if (g_theWriteableGlobalData->m_buildMapCache) {
        map_root = Get_Map_Dir();
    } else {
        map_root = Get_User_Map_Dir();
        INI ini;
        Utf8String name;
        name.Format("%s\\%s", map_root.Str(), "MapCache.ini");
        File *file = g_theFileSystem->Open_File(name.Str(), File::READ);

        if (file != nullptr) {
            file->Close();
            file = nullptr;
            ini.Load(name, INI_LOAD_OVERWRITE, nullptr);
        }
    }

    m_seen.clear();

    for (auto it = begin(); it != end(); it++) {
        m_seen[it->first] = false;
    }

    std::set<Utf8String, rts::less_than_nocase<Utf8String>> map_paths;
    Utf8String dir;
    dir.Format("%s\\", map_root.Str());
    bool map_added = false;

    Utf8String filter;
    filter.Format("*.%s", Get_Map_Extension().Str());
    g_theFileSystem->Get_File_List_In_Directory(dir, filter, map_paths, true);

    for (auto it = map_paths.begin(); it != map_paths.end(); it++) {
        Utf8String map_path;
        map_path = *it;
        map_path.To_Lower();
        const char *map_file_name = map_path.Reverse_Find('\\');

        if (map_file_name != nullptr) {
            Utf8String expected_map_file;
            Utf8String map_name(map_file_name + 1);

            for (unsigned int i = 0; i < strlen(".map"); i++) {
                map_name.Remove_Last_Char();
            }

            expected_map_file.Format("%s\\%s%s", map_name.Str(), map_name.Str(), ".map");
            bool not_in_unk = false;

            if (g_theWriteableGlobalData->m_buildMapCache) {
                auto it2 = m_unk.find(map_name);

                if (m_unk.size() != 0) {
                    if (it2 == m_unk.end()) {
                        not_in_unk = true;
                    }
                }
            }

            if (!not_in_unk) {
                if (map_path.Ends_With_No_Case(expected_map_file.Str())) {
                    FileInfo info;

                    if (g_theFileSystem->Get_File_Info(map_path, &info)) {
                        char dest[PATH_MAX];
                        strcpy(dest, map_path.Str());

                        for (const char *c = dest; *c != '\0'; c++) {
                            if (*c == '\\' || *c == '/') {
                                c++;
                            }
                        }

                        m_seen[map_path] = true;
                        map_added |= Add_Map(map_root, *it, &info, g_theWriteableGlobalData->m_buildMapCache);
                    } else {
                        captainslog_dbgassert(false, "Could not get file info for map %s", (*it).Str());
                    }
                } else {
                    captainslog_dbgassert(false, "Found map '%s' in wrong spot (%s)", map_name.Str(), map_path.Str());
                }
            }
        } else {
            captainslog_dbgassert(false, "Couldn't find \\ in map name!");
        }
    }

    if (Clear_Unseen_Maps(map_root)) {
        return true;
    } else {
        return map_added;
    }
}

bool Parse_Size_Only(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    s_width = input.Read_Int32();
    s_height = input.Read_Int32();

    if (info->version < 3) {
        s_borderSize = 0;
    } else {
        s_borderSize = input.Read_Int32();
    }

    if (info->version >= 4) {
        int count = input.Read_Int32();
        s_boundaries.resize(count);

        for (int i = 0; i < count; i++) {
            s_boundaries[i].x = input.Read_Int32();
            s_boundaries[i].y = input.Read_Int32();
        }
    }

    return true;
}

bool Parse_Size_Only_In_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    return Parse_Size_Only(input, info, data);
}

bool Parse_World_Dict_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    s_worldDict = input.Read_Dict();
    return true;
}

bool Parse_Object_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    bool use_props = info->version >= 2;
    Coord3D loc;
    loc.x = input.Read_Real32();
    loc.y = input.Read_Real32();
    loc.z = input.Read_Real32();

    if (info->version <= 2) {
        loc.z = 0.0f;
    }

    float angle = input.Read_Real32();
    int flags = input.Read_Int32();
    Utf8String name = input.Read_AsciiString();
    Dict props;

    if (use_props) {
        props = input.Read_Dict();
    }

    ThingTemplate *tmplate = nullptr;

    MapObject *object = new MapObject(loc, name, angle, flags, &props, g_theThingFactory->Find_Template(name, false));

    if (object->Get_Properties()->Get_Type(g_waypointIDKey) == Dict::DICT_INT) {
        object->Set_Is_Waypoint();
        (*g_waypoints)[object->Get_Waypoint_Name()] = loc;
    } else if (object->Get_Thing_Template() != nullptr && object->Get_Thing_Template()->Is_KindOf(KINDOF_TECH_BUILDING)) {
        s_techPositions.push_back(loc);
    } else if (object->Get_Thing_Template() != nullptr) {
        if (object->Get_Thing_Template()->Is_KindOf(KINDOF_SUPPLY_SOURCE_ON_PREVIEW)) {
            s_supplyPositions.push_back(loc);
        }
    }

    object->Delete_Instance();
    return true;
}

bool Parse_Objects_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    input.m_currentObject = nullptr;
    input.Register_Parser("Object", info->label, Parse_Object_Data_Chunk, nullptr);
    return input.Parse(data);
}

bool Load_Map(Utf8String name)
{
    Utf8String name_copy2;
    int len = 0;
    char name_copy[PATH_MAX];
    strcpy(name_copy, name.Str());
    len = strlen(name_copy);
    char name_without_ext[PATH_MAX];

    if (len >= 4) {
        memset(name_without_ext, 0, sizeof(name_without_ext));
        strncpy(name_without_ext, name_copy, len - 4);
    }

    CachedFileInputStream stream;
    name_copy2 = name;

    if (stream.Open(name_copy2)) {
        DataChunkInput input(&stream);
        g_waypoints = new WaypointMap();
        input.Register_Parser("HeightMapData", Utf8String::s_emptyString, Parse_Size_Only_In_Chunk, nullptr);
        input.Register_Parser("WorldInfo", Utf8String::s_emptyString, Parse_World_Dict_Data_Chunk, nullptr);
        input.Register_Parser("ObjectsList", Utf8String::s_emptyString, Parse_Objects_Data_Chunk, nullptr);

        if (!input.Parse(nullptr)) {
            throw CODE_05;
        }

        s_mapDX = s_width - 2 * s_borderSize;
        s_mapDY = s_height - 2 * s_borderSize;
        return true;
    } else {
        return false;
    }
}

unsigned int Calc_CRC(Utf8String dir, Utf8String name)
{
    CRC crc;
    crc.Clear();

    Utf8String str;
    char name_copy[PATH_MAX];
    strcpy(name_copy, name.Str());
    int len = strlen(name_copy);
    char name_without_ext[PATH_MAX];

    if (len >= 4) {
        memset(name_without_ext, 0, sizeof(name_without_ext));
        strncpy(name_without_ext, name_copy, len - 4);
    }

    str = name;
    File *file = g_theFileSystem->Open_File(str.Str(), File::READ);

    if (file != nullptr) {
        for (;;) {
            unsigned char buf3[4096];
            int val = file->Read(buf3, 4096);

            if (val <= 0) {
                break;
            }

            crc.Compute_CRC(buf3, val);
        }

        file->Close();
        return crc.Get_CRC();
    } else {
        captainslog_dbgassert(false, "Couldn't open '%s'", name.Str());
        return 0;
    }
}

void Reset_Map()
{
    if (g_waypoints != nullptr) {
        delete g_waypoints;
        g_waypoints = nullptr;
    }

    s_techPositions.clear();
    s_supplyPositions.clear();
}

void Get_Extent(Region3D *region)
{
    region->lo.x = 0.0f;
    region->lo.y = 0.0f;
    region->hi.x = s_mapDX * 10.0f;
    region->hi.y = s_mapDY * 10.0f;
    region->lo.z = 0.0f;
    region->hi.z = 0.0f;
}

bool Would_Map_Transfer(const Utf8String &path)
{
    Utf8String str = g_theMapCache->Get_User_Map_Dir();
    return path.Starts_With_No_Case(str.Str());
}

int Populate_Map_List_Box_No_Reset(GameWindow *listbox, bool is_official, bool is_multiplayer, Utf8String map_name)
{
    if (g_theMapCache != nullptr && listbox != nullptr) {
        int columns = Gadget_List_Box_Get_Num_Columns(listbox);
        const Image *star_bronze = nullptr;
        const Image *star_silver = nullptr;
        const Image *star_gold = nullptr;
        const Image *red_yel_star = nullptr;
        SkirmishBattleHonors *honors = nullptr;
        int width = 10;
        int height = 10;

        if (columns > 1) {
            star_bronze = g_theMappedImageCollection->Find_Image_By_Name("Star-Bronze");
            star_silver = g_theMappedImageCollection->Find_Image_By_Name("Star-Silver");
            star_gold = g_theMappedImageCollection->Find_Image_By_Name("Star-Gold");
            red_yel_star = g_theMappedImageCollection->Find_Image_By_Name("RedYell_Star");
            honors = new SkirmishBattleHonors();
            int image_width;

            if (star_gold != nullptr) {
                image_width = star_gold->Get_Image_Width();
            } else {
                image_width = 10;
            }

            width = image_width;
            int column_width = Gadget_List_Box_Get_Column_Width(listbox, 0);
            width = GameMath::Min(column_width, image_width);
            height = width;
        }

        int color = Make_Color(0xFF, 0xFF, 0xFF, 0xFF);
        Utf16String map_display_name;
        int selected = 0;
        Utf8String map_root;

        if (is_official) {
            map_root = g_theMapCache->Get_Map_Dir();
        } else {
            map_root = g_theWriteableGlobalData->Get_Path_User_Data();
            map_root += g_theMapCache->Get_Map_Dir();
        }

        map_root.To_Lower();
        std::set<Utf16String> set;
        std::map<Utf16String, Utf8String> map;
        unsigned int count = 0;

        for (int i = 0; count < g_theMapCache->size(); i++) {
            captainslog_debug("Adding maps with %d players", i);

            for (auto it = g_theMapCache->begin(); it != g_theMapCache->end(); it++) {
                if (it->second.m_numPlayers == i) {
                    set.insert(it->second.m_displayName);
                    map[it->second.m_displayName] = it->first;
                    captainslog_debug("Adding map %s to temp cache.", it->first.Str());
                    count++;
                }
            }

            for (auto it = set.begin(); it != set.end(); it++) {
                Utf8String name;
                name = map[*it];
                auto it2 = g_theMapCache->find(name);
                captainslog_dbgassert(it2 != g_theMapCache->end(), "Map %ls not found in map cache.", (*it).Str());

                if (it2->first.Starts_With_No_Case(map_root.Str()) && is_multiplayer == it2->second.m_isMultiplayer) {
                    if (!it2->second.m_displayName.Is_Empty()) {
                        map_display_name = it2->second.m_displayName;
                        int i1 = -1;
                        int i2 = -1;

                        if (columns > 1 && it2->second.m_isMultiplayer) {
                            int medal1 = honors->Get_Endurance_Medal(it2->first, 2);
                            int medal2 = honors->Get_Endurance_Medal(it2->first, 3);
                            int medal3 = honors->Get_Endurance_Medal(it2->first, 4);

                            if (medal3 != 0) {
                                if (medal3 == it2->second.m_numPlayers - 1) {
                                    i1 = Gadget_List_Box_Add_Entry_Image(
                                        listbox, red_yel_star, i1, 0, width, height, true, -1);
                                    i2 = 4;
                                } else {
                                    i1 = Gadget_List_Box_Add_Entry_Image(listbox, star_gold, i1, 0, width, height, true, -1);
                                    i2 = 3;
                                }
                            } else if (medal2 != 0) {
                                i2 = 2;
                                i1 = Gadget_List_Box_Add_Entry_Image(listbox, star_silver, i1, 0, width, height, true, -1);
                            } else if (medal1 != 0) {
                                i2 = 1;
                                i1 = Gadget_List_Box_Add_Entry_Image(listbox, star_bronze, i1, 0, width, height, true, -1);
                            } else {
                                i2 = 0;
                                i1 = Gadget_List_Box_Add_Entry_Image(listbox, nullptr, i1, 0, width, height, true, -1);
                            }
                        }

                        i1 = Gadget_List_Box_Add_Entry_Text(listbox, map_display_name, color, i1, columns - 1, 1);

                        if (it2->first == map_name) {
                            selected = i1;
                        }

                        Gadget_List_Box_Set_Item_Data(listbox, const_cast<char *>(it2->first.Str()), i1, 0);

                        if (columns > 1) {
                            Gadget_List_Box_Set_Item_Data(listbox, reinterpret_cast<void *>(i2), i1, 1);
                        }
                    }
                }
            }

            set.clear();
            map.clear();
        }

        if (honors != nullptr) {
            delete honors;
            honors = nullptr;
        }

        Gadget_List_Box_Set_Selected(listbox, selected);

        if (selected >= 0) {
            int top_entry = Gadget_List_Box_Get_Top_Visible_Entry(listbox);
            int bottom_entry = Gadget_List_Box_Get_Bottom_Visible_Entry(listbox);
            int i3 = bottom_entry - top_entry;

            if (selected >= bottom_entry) {
                Gadget_List_Box_Set_Top_Visible_Entry(listbox, GameMath::Max(0, selected - GameMath::Max(1, i3 / 2)));
            }
        }

        return selected;
    } else {
        return -1;
    }
}

int Populate_Map_List_Box(GameWindow *listbox, bool is_official, bool is_multiplayer, Utf8String map_name)
{
    if (g_theMapCache != nullptr && listbox != nullptr) {
        Gadget_List_Box_Reset(listbox);
        return Populate_Map_List_Box_No_Reset(listbox, is_official, is_multiplayer, map_name);
    } else {
        return -1;
    }
}

bool Is_Valid_Map(Utf8String map_name, bool is_multiplayer)
{
    if (g_theMapCache != nullptr && !map_name.Is_Empty()) {
        g_theMapCache->Update_Cache();
        map_name.To_Lower();
        auto it = g_theMapCache->find(map_name);
        if (it != g_theMapCache->end() && is_multiplayer == it->second.m_isMultiplayer) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

Utf8String Get_Default_Map(bool is_multiplayer)
{
    if (g_theMapCache != nullptr) {
        g_theMapCache->Update_Cache();

        for (auto it = g_theMapCache->begin(); it != g_theMapCache->end(); it++) {
            if (is_multiplayer == it->second.m_isMultiplayer) {
                return it->first;
            }
        }

        return Utf8String::s_emptyString;
    }

    return Utf8String::s_emptyString;
}

Utf8String Get_Official_Multiplayer_Map()
{
    if (g_theMapCache != nullptr) {
        g_theMapCache->Update_Cache();

        for (auto it = g_theMapCache->begin(); it != g_theMapCache->end(); it++) {
            if (it->second.m_isMultiplayer && it->second.m_isOfficial) {
                return it->first;
            }
        }

        return Utf8String::s_emptyString;
    }

    return Utf8String::s_emptyString;
}

bool Is_Map_Official(Utf8String map_name)
{
    if (g_theMapCache != nullptr && !map_name.Is_Empty()) {
        g_theMapCache->Update_Cache();
        map_name.To_Lower();
        auto it = g_theMapCache->find(map_name);
        if (it != g_theMapCache->end()) {
            return it->second.m_isOfficial;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void Copy_From_Big_To_Dir(Utf8String &source, Utf8String &dest)
{
    File *file = g_theFileSystem->Open_File(source.Str(), File::READ | File::BINARY);
    captainslog_relassert(file != nullptr, 6, "Copy_From_Big_To_Dir - Error opening source file '%s'", source.Str());
    int size = file->Seek(0, File::END);
    file->Seek(0, File::START);
    char *buf = new char[size];
    captainslog_relassert(buf != nullptr, 6, "Copy_From_Big_To_Dir - Unable to allocate buffer for file '%s'", source.Str());
    int read_size = file->Read(buf, size);
    captainslog_relassert(read_size >= size, 6, "Copy_From_Big_To_Dir - Error reading from file '%s'", source.Str());
    file->Close();

    File *out_file = g_theFileSystem->Open_File(dest.Str(), File::WRITE | File::CREATE | File::BINARY);
    captainslog_relassert(out_file != nullptr, 6, "Copy_From_Big_To_Dir - Error writing to file '%s'", source.Str());
    int write_size = out_file->Write(buf, size);
    captainslog_relassert(write_size >= size, 6, "Copy_From_Big_To_Dir - Error writing to file '%s'", source.Str());
    out_file->Close();
    delete[] buf;
}

Image *Get_Map_Preview_Image(Utf8String map_name)
{
    if (g_theWriteableGlobalData == nullptr) {
        return nullptr;
    }

    captainslog_debug("%s Map Name ", map_name.Str());
    Utf8String map_image(map_name);
    Utf8String image_file;
    Utf8String image_name;
    Utf8String str4;
    map_image.Remove_Last_Char();
    map_image.Remove_Last_Char();
    map_image.Remove_Last_Char();
    map_image.Remove_Last_Char();
    image_file = map_image;
    str4 = map_image.Reverse_Find('\\') + 1;
    str4.Concat(".tga");
    map_image.Concat(".tga");
    Utf8String portable_map_path = g_theGameState->Real_To_Portable_Map_Path(image_file);
    image_name.Set(Utf8String::s_emptyString);

    for (int i = 0; i < portable_map_path.Get_Length(); i++) {
        char c = portable_map_path.Get_Char(i);

        if (c == '\\' || c == ':') {
            image_name.Concat('_');
        } else {
            image_name.Concat(c);
        }
    }

    image_file = image_name;
    image_file.Concat(".tga");
    Image *image = g_theMappedImageCollection->Find_Image_By_Name(image_name);

    if (image == nullptr) {
        if (!g_theFileSystem->Does_File_Exist(map_image.Str())) {
            return nullptr;
        }

        Utf8String preview_image;
        preview_image.Format("%sMapPreviews/", g_theWriteableGlobalData->Get_Path_User_Data().Str());
        g_theFileSystem->Create_Directory(preview_image);
        preview_image += image_file;
        Copy_From_Big_To_Dir(map_image, preview_image);

        image = new Image();
        image->Set_Name(image_name);
        image->Set_Filename(image_file);
        image->Set_Status(0);

        Region2D region;
        region.hi.x = 1.0f;
        region.hi.y = 1.0f;
        region.lo.x = 0.0f;
        region.lo.y = 0.0f;
        image->Set_UV_Region(&region);

        image->Set_Texture_Height(128);
        image->Set_Texture_Width(128);
        g_theMappedImageCollection->Add_Image(image);
    }

    return image;
}

bool MapCache::Add_Map(Utf8String dir, Utf8String name, FileInfo *info, bool build_cache)
{
    if (info == nullptr) {
        return false;
    }

    Utf8String name_lower;
    name_lower = name;
    name_lower.To_Lower();
    auto map_it = find(name_lower);
    MapMetaData data;
    int size = info->file_size_low;

    if (map_it != end()) {
        data = map_it->second;

        if (data.m_fileSize == size && data.m_CRC != 0) {
            if (data.m_lookupTag.Is_Empty()) {
                Utf8String str2;
                str2 = name.Reverse_Find('\\') + 1;
                (*this)[name_lower].m_displayName.Translate(str2);

                if (data.m_numPlayers >= 2) {
                    Utf16String str3;
                    str3.Format(U_CHAR(" (%d)"), data.m_numPlayers);
                    (*this)[name_lower].m_displayName.Concat(str3);
                }
            } else {
                (*this)[name_lower].m_displayName = g_theGameText->Fetch(data.m_lookupTag);

                if (data.m_numPlayers >= 2) {
                    Utf16String str3;
                    str3.Format(U_CHAR(" (%d)"), data.m_numPlayers);
                    (*this)[name_lower].m_displayName.Concat(str3);
                }
            }

            return false;
        }

        captainslog_debug("%s didn't match file in MapCache", name.Str());
        captainslog_debug("size: %d / %d", size, data.m_fileSize);
        captainslog_debug("time1: %d / %d", info->write_time_high, data.m_timestamp.m_highTimeStamp);
        captainslog_debug("time2: %d / %d", info->write_time_low, data.m_timestamp.m_lowTimeStamp);
    }

    captainslog_debug("MapCache::Add_Map(): caching '%s' because '%s' was not found", name.Str(), name_lower.Str());
    Load_Map(name);
    data.m_mapFilename = name_lower;
    data.m_fileSize = size;
    data.m_isOfficial = build_cache;
    data.m_waypoints.Update();
    data.m_numPlayers = data.m_waypoints.m_numStartSpots;
    data.m_isMultiplayer = data.m_waypoints.m_numStartSpots >= 2;
    data.m_timestamp.m_highTimeStamp = info->write_time_high;
    data.m_timestamp.m_lowTimeStamp = info->write_time_low;
    data.m_supplyPositions = s_supplyPositions;
    data.m_techPositions = s_techPositions;
    data.m_CRC = Calc_CRC(dir, name);
    bool exists = false;

    Utf8String map = s_worldDict.Get_AsciiString(g_mapName, &exists);
    data.m_lookupTag = map;

    if (exists && !map.Is_Empty()) {
        Utf8String map_str;
        map_str.Format("%s\\%s", dir.Str(), name.Str());

        for (int i = 0; i < 4; i++) {
            map_str.Remove_Last_Char();
        }

        map_str.Concat("\\map.str");
        g_theGameText->Init_Map_String_File(map_str);
        data.m_displayName = g_theGameText->Fetch(map);

        if (data.m_numPlayers >= 2) {
            Utf16String str6;
            str6.Format(U_CHAR(" (%d)"), data.m_numPlayers);
            data.m_displayName.Concat(str6);
        }

        captainslog_debug("Map name is now '%ls'", data.m_displayName.Str());
        g_theGameText->Reset();
    } else {
        captainslog_debug("Missing TheKey_mapName!");
        Utf8String str7;
        str7 = name.Reverse_Find('\\') + 1;
        data.m_displayName.Translate(str7);

        if (data.m_numPlayers >= 2) {
            Utf16String str8;
            str8.Format(U_CHAR(" (%d)"), data.m_numPlayers);
            data.m_displayName.Concat(str8);
        }

        g_theGameText->Reset();
    }

    Get_Extent(&data.m_extent);
    (*this)[name_lower] = data;
    captainslog_debug("  filesize = %d bytes", data.m_fileSize);
    captainslog_debug("  displayName = %ls", data.m_displayName.Str());
    captainslog_debug("  CRC = %X", data.m_CRC);
    captainslog_debug("  isOfficial = %s", data.m_isOfficial ? "yes" : "no");
    captainslog_debug("  isMultiplayer = %s", data.m_isMultiplayer ? "yes" : "no");
    captainslog_debug("  numPlayers = %d", data.m_numPlayers);
    captainslog_debug("  extent = (%2.2f,%2.2f) -> (%2.2f,%2.2f)",
        data.m_extent.lo.x,
        data.m_extent.lo.y,
        data.m_extent.hi.x,
        data.m_extent.hi.y);

    for (auto waypoint = data.m_waypoints.begin(); waypoint != data.m_waypoints.end(); waypoint++) {
        captainslog_debug("    waypoint %s: (%2.2f,%2.2f)", waypoint->first.Str(), waypoint->second.x, waypoint->second.y);
    }

    Reset_Map();
    return true;
}

MapMetaData *MapCache::Find_Map(Utf8String map_name)
{
    map_name.To_Lower();
    auto it = find(map_name);

    if (it == end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

void Find_Draw_Positions(int start_x, int start_y, int width, int height, Region3D extent, ICoord2D *ul, ICoord2D *lr)
{
    float max_w = extent.Width() / (width * 1.0f);
    float max_h = extent.Height() / (height * 1.0f);

    if (max_w < max_h) {
        float fitted_w = extent.Width() / max_h;
        float fitted_h = extent.Height() / max_h;
        ul->x = (width - fitted_w) / 2.0f;
        ul->y = 0;
        lr->x = width - ul->x;
        lr->y = fitted_h;
    } else {
        float fitted_w = extent.Width() / max_w;
        float fitted_h = extent.Height() / max_w;
        ul->x = 0;
        ul->y = (height - fitted_h) / 2.0f;
        lr->x = fitted_w;
        lr->y = height - ul->y;
    }

    ul->x += start_x;
    ul->y += start_y;
    lr->x += start_x;
    lr->y += start_y;
}

const FieldParse *MapMetaDataReader::Get_Field_Parse()
{
    return MapMetaDataReader::s_mapFieldParseTable;
}

void MapCache::Parse_Map_Cache_Definition(INI *ini)
{
    MapMetaDataReader reader;
    MapMetaData map;
    Utf8String str;
    str.Set(ini->Get_Next_Token(" \n\r\t"));
    str = Quoted_Printable_To_Ascii_String(str);
    map.m_waypoints.clear();
    ini->Init_From_INI(&reader, MapMetaDataReader::Get_Field_Parse());
    map.m_extent = reader.m_extent;
    map.m_isOfficial = reader.m_isOfficial;
    map.m_isMultiplayer = reader.m_isMultiplayer;
    map.m_numPlayers = reader.m_numPlayers;
    map.m_fileSize = reader.m_filesize;
    map.m_CRC = reader.m_CRC;
    map.m_timestamp = reader.m_timestamp;
    map.m_waypoints[g_theNameKeyGenerator->Key_To_Name(g_theInitialCameraPositionKey)] = reader.m_initialCameraPosition;
    map.m_lookupTag = Quoted_Printable_To_Ascii_String(reader.m_nameLookupTag);

    if (map.m_lookupTag.Is_Empty()) {
        Utf8String display_name;
        display_name = str.Reverse_Find('\\') + 1;
        map.m_displayName.Translate(display_name);

        if (map.m_numPlayers >= 2) {
            Utf16String players;
            players.Format(U_CHAR(" (%d)"), map.m_numPlayers);
            map.m_displayName.Concat(players);
        }
    } else {
        map.m_displayName = g_theGameText->Fetch(map.m_lookupTag);

        if (map.m_numPlayers >= 2) {
            Utf16String players;
            players.Format(U_CHAR(" (%d)"), map.m_numPlayers);
            map.m_displayName.Concat(players);
        }
    }

    Utf8String start_pos;

    for (int i = 0; i < map.m_numPlayers; i++) {
        start_pos.Format("Player_%d_Start", i + 1);
        map.m_waypoints[start_pos] = reader.m_waypoints[i];
    }

    for (auto it = reader.m_supplyPositions.begin(); it != reader.m_supplyPositions.end(); it++) {
        map.m_supplyPositions.push_back(*it);
    }

    for (auto it = reader.m_techPositions.begin(); it != reader.m_techPositions.end(); it++) {
        map.m_techPositions.push_back(*it);
    }

    if (g_theMapCache != nullptr && !map.m_displayName.Is_Empty()) {
        Utf8String filename(str);
        filename.To_Lower();
        map.m_mapFilename = filename;
        (*g_theMapCache)[filename] = map;
    }
}
