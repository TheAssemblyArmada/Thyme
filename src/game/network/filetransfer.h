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
#pragma once

#include "always.h"
#include "asciistring.h"

class GameInfo;
class MapTransferLoadScreen;

Utf8String Get_Base_Path_From_Path(Utf8String path);
Utf8String Get_File_From_Path(Utf8String path);
Utf8String Get_Base_File_From_File(Utf8String path);
Utf8String Get_Preview_From_Map(Utf8String path);
Utf8String Get_INI_From_Map(Utf8String path);
Utf8String Get_Str_File_From_Map(Utf8String path);
Utf8String Get_Solo_INI_From_Map(Utf8String path);
Utf8String Get_Asset_Usage_From_Map(Utf8String path);
Utf8String Get_Readme_From_Map(Utf8String path);
bool Do_Any_File_Transfers(GameInfo *gameinfo);
bool Do_File_Transfer(Utf8String filename, MapTransferLoadScreen *screen, int unkbool);
