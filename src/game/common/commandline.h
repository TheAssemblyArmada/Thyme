////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: COMMANDLINE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Command line argument parser.
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
#pragma once

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include "always.h"
#include "hooker.h"

void Parse_Command_Line(int argc, char *argv[]);

namespace CommandLine {

inline void Hook_Me()
{
    Hook_Function((Make_Function_Ptr<void, int, char **>(0x004F8150)), &Parse_Command_Line);
}

} // namespace CommandLine

#endif // _COMMANDLINE_H
