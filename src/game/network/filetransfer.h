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
#pragma once

#include "always.h"
#include "asciistring.h"

class GameInfo;
class MapTransferLoadScreen;

AsciiString Get_Base_Path_From_Path(AsciiString path);
AsciiString Get_File_From_Path(AsciiString path);
AsciiString Get_Base_File_From_File(AsciiString path);
AsciiString Get_Preview_From_Map(AsciiString path);
AsciiString Get_INI_From_Map(AsciiString path);
AsciiString Get_Str_File_From_Map(AsciiString path);
AsciiString Get_Solo_INI_From_Map(AsciiString path);
AsciiString Get_Asset_Usage_From_Map(AsciiString path);
AsciiString Get_Readme_From_Map(AsciiString path);
bool Do_Any_File_Transfers(GameInfo *gameinfo);
bool Do_File_Transfer(AsciiString filename, MapTransferLoadScreen *screen, int unkbool);

#ifndef THYME_STANDALONE
#include "hooker.h"

namespace FileTransfer
{
inline void Hook_Me()
{
    Hook_Function(0x006F6200, &Get_Base_Path_From_Path);
    Hook_Function(0x006F6390, &Get_File_From_Path);
    Hook_Function(0x006F6490, &Get_Base_File_From_File);
    Hook_Function(0x006F6620, &Get_Preview_From_Map);
}
}
#endif