////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: PLATFORMFILE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class for plafrom specific file IO.
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

#ifndef _PLATFORMFILE_H_
#define _PLATFORMFILE_H_

#include "localfile.h"

class PlatformFile : public LocalFile
{
    IMPLEMENT_POOL(PlatformFile);
};

#endif // _PLATFORMFILE_H_
