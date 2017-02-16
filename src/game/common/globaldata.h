////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GLOBALDATA.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class for handling various global variables
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

#ifndef _GLOBALDATA_H_
#define _GLOBALDATA_H_

#include "subsysteminterface.h"

class INI;

class GlobalData : public SubsystemInterface
{
    public:
        GlobalData();
        virtual ~GlobalData();

        // SubsystemInterface implementation
        virtual void Init() {}
        virtual void Reset();
        virtual void Update() {}

        static void Parse_Game_Data_Definitions(INI *ini);
    
    // Looks like members are likely public or there would have been a lot of
    // getters/setters.
    public:
        
};

#endif
