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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#include "always.h"
#include "hooker.h"

void Parse_Command_Line(int argc, char const **argv);

namespace CommandLine {

inline void Hook_Me()
{
    Hook_Function((Make_Function_Ptr<void, int, char const**>(0x004F8150)), &Parse_Command_Line);
}

} // namespace CommandLine

#endif // _COMMANDLINE_H_
