/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Parser for SAGE engine configuration files.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "drawgroupinfo.h"
#include "ini.h"

// Was originally INI::parseDrawGroupNumberDefinition
void INI::Parse_Draw_Group_Info(INI *ini)
{
    captainslog_relassert(g_theDrawGroupInfo != nullptr, 0xDEAD0006, "g_theDrawGroupInfo is null when attempting parse.");
    ini->Init_From_INI(g_theDrawGroupInfo, DrawGroupInfo::Get_Parse_Table());
}
