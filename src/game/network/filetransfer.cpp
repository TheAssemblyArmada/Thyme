/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Functions involved in file transfers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "filetransfer.h"
#include "gamedebug.h"
#include "minmax.h"

AsciiString Get_Base_Path_From_Path(AsciiString path)
{
    const char *path_str = path.Str();

    // Find the last path seperator for either win or unix style.
    const char *last_sep = MAX(strrchr(path_str, '/'), strrchr(path_str, '\\'));

    DEBUG_LOG("Getting base path from '%s'.\n", path_str);

    if (last_sep != nullptr) {
        ptrdiff_t size = last_sep - path_str;
        AsciiString ret;
        memcpy(ret.Get_Buffer_For_Read(size + 1), path_str, size);

        return ret;
    }

    return AsciiString::s_emptyString;
}

AsciiString Get_File_From_Path(AsciiString path)
{
    const char *path_str = path.Str();

    // Find the last path seperator for either win or unix style.
    const char *last_sep = MAX(strrchr(path_str, '/'), strrchr(path_str, '\\'));

    DEBUG_LOG("Getting file from '%s'.\n", path_str);

    if (last_sep != nullptr) {
        return AsciiString(last_sep + 1);
    }

    return path;
}

AsciiString Get_Base_File_From_File(AsciiString path)
{
    const char *path_str = path.Str();

    // Find the last . seperator for file extension.
    const char *last_sep = strrchr(path_str, '.');

    DEBUG_LOG("Getting base filename from '%s'.\n", path_str);

    // If we have a file extension, return the part before it.
    if (last_sep != nullptr) {
        ptrdiff_t size = last_sep - path_str;
        AsciiString ret;
        memcpy(ret.Get_Buffer_For_Read(size + 1), path_str, size);

        return ret;
    }

    return AsciiString::s_emptyString;
}

AsciiString Get_Preview_From_Map(AsciiString path)
{
    AsciiString preview;

    preview.Format(
        "%s/%s.tga", Get_Base_Path_From_Path(path).Str(), Get_Base_File_From_File(Get_File_From_Path(path)).Str());

    return preview;
}

AsciiString Get_INI_From_Map(AsciiString path)
{
    AsciiString ini;

    ini.Format("%s/map.ini", Get_Base_Path_From_Path(path).Str());

    return ini;
}

AsciiString Get_Str_File_From_Map(AsciiString path)
{
    AsciiString str;

    str.Format("%s/map.str", Get_Base_Path_From_Path(path).Str());

    return str;
}

AsciiString Get_Solo_INI_From_Map(AsciiString path)
{
    AsciiString ini;

    ini.Format("%s/solo.ini", Get_Base_Path_From_Path(path).Str());

    return ini;
}

AsciiString Get_Asset_Usage_From_Map(AsciiString path)
{
    AsciiString use;

    use.Format("%s/assetusage.txt", Get_Base_Path_From_Path(path).Str());

    return use;
}

AsciiString Get_Readme_From_Map(AsciiString path)
{
    AsciiString readme;

    readme.Format("%s/readme.txt", Get_Base_Path_From_Path(path).Str());

    return readme;
}

bool Do_Any_File_Transfers(GameInfo *gameinfo)
{
    // TODO, needs GameInfo
    return false;
}

bool Do_File_Transfer(AsciiString filename, MapTransferLoadScreen *screen, int unkbool)
{
    // TODO needs MapTransferLoadScreen
    return false;
}
