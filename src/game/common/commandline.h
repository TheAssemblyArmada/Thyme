/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Command line argument parser.
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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include "always.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

void Parse_Command_Line(int argc, char *argv[]);

#ifndef THYME_STANDALONE
namespace CommandLine
{

inline void Hook_Me()
{
    Hook_Function(0x004F8150, &Parse_Command_Line);
}

} // namespace CommandLine
#endif

#endif // _COMMANDLINE_H
