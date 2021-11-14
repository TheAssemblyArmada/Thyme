/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions involved in file transfers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "filetransfer.h"
#include <algorithm>
#include <captainslog.h>
#include <cstddef>

using std::ptrdiff_t;

Utf8String Get_Base_Path_From_Path(Utf8String path)
{
    const char *path_str = path.Str();

    // Find the last path seperator for either win or unix style.
    const char *last_sep = std::max(strrchr(path_str, '/'), strrchr(path_str, '\\'));

    captainslog_trace("Getting base path from '%s'.", path_str);

    if (last_sep != nullptr) {
        ptrdiff_t size = last_sep - path_str;
        Utf8String ret;
        memcpy(ret.Get_Buffer_For_Read(size + 1), path_str, size);

        return ret;
    }

    return Utf8String::s_emptyString;
}

Utf8String Get_File_From_Path(Utf8String path)
{
    const char *path_str = path.Str();

    // Find the last path seperator for either win or unix style.
    const char *last_sep = std::max(strrchr(path_str, '/'), strrchr(path_str, '\\'));

    captainslog_trace("Getting file from '%s'.", path_str);

    if (last_sep != nullptr) {
        return Utf8String(last_sep + 1);
    }

    return path;
}

Utf8String Get_Base_File_From_File(Utf8String path)
{
    const char *path_str = path.Str();

    // Find the last . seperator for file extension.
    const char *last_sep = strrchr(path_str, '.');

    captainslog_trace("Getting base filename from '%s'.", path_str);

    // If we have a file extension, return the part before it.
    if (last_sep != nullptr) {
        ptrdiff_t size = last_sep - path_str;
        Utf8String ret;
        memcpy(ret.Get_Buffer_For_Read(size + 1), path_str, size);

        return ret;
    }

    return Utf8String::s_emptyString;
}

Utf8String Get_Preview_From_Map(Utf8String path)
{
    Utf8String preview;

    preview.Format(
        "%s/%s.tga", Get_Base_Path_From_Path(path).Str(), Get_Base_File_From_File(Get_File_From_Path(path)).Str());

    return preview;
}

Utf8String Get_INI_From_Map(Utf8String path)
{
    Utf8String ini;

    ini.Format("%s/map.ini", Get_Base_Path_From_Path(path).Str());

    return ini;
}

Utf8String Get_Str_File_From_Map(Utf8String path)
{
    Utf8String str;

    str.Format("%s/map.str", Get_Base_Path_From_Path(path).Str());

    return str;
}

Utf8String Get_Solo_INI_From_Map(Utf8String path)
{
    Utf8String ini;

    ini.Format("%s/solo.ini", Get_Base_Path_From_Path(path).Str());

    return ini;
}

Utf8String Get_Asset_Usage_From_Map(Utf8String path)
{
    Utf8String use;

    use.Format("%s/assetusage.txt", Get_Base_Path_From_Path(path).Str());

    return use;
}

Utf8String Get_Readme_From_Map(Utf8String path)
{
    Utf8String readme;

    readme.Format("%s/readme.txt", Get_Base_Path_From_Path(path).Str());

    return readme;
}

bool Do_Any_File_Transfers(GameInfo *gameinfo)
{
    // TODO, needs GameInfo
    return false;
}

bool Do_File_Transfer(Utf8String filename, MapTransferLoadScreen *screen, int unkbool)
{
    // TODO needs MapTransferLoadScreen
    return false;
}
