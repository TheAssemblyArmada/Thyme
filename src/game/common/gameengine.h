////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEENGINE.H
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

#ifndef _GAMEENGINE_H_
#define _GAMEENGINE_H_

#include "subsysteminterface.h"

class GameEngine : public SubsystemInterface
{
    public:
        
    private:
        int MaxFPS;
        bool Quitting;
        bool Active;
};

#endif // _GAMEENGINE_H_
