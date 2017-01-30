////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ARCHIVEFILESYSTEM.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for local file system implementations.
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

#ifndef _ARCHIVEFILESYSTEM_H_
#define _ARCHIVEFILESYSTEM_H_

#include "subsysteminterface.h"
#include "hooker.h"

#define TheArchiveFileSystem (Make_Global<ArchiveFileSystem*>(0x00A2BA00))

class ArchiveFileSystem : public SubsystemInterface 
{
    
};

#endif // _ARCHIVEFILESYSTEM_H_
