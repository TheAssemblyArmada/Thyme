////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32GAMEENGINE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for the game engine implementation.
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

#ifndef _WIN32GAMEENGINE_H_
#define _WIN32GAMEENGINE_H_

#include "gameengine.h"
#include "win32localfilesystem.h"

class Win32GameEngine : public GameEngine
{
    public:
        //TODO

        static LocalFileSystem *Create_Local_Filesystem();
};

#endif // _WIN32GAMEENGINE_H_