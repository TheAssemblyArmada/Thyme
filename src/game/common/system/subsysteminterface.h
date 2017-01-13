////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SUBSYSTEMINTERFACE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base subsystem class.
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

#ifndef _SUBSYSTEMINTERFACE_H_
#define _SUBSYSTEMINTERFACE_H_

#include "asciistring.h"
#include <vector>

class Xfer;

class SubsystemInterface
{
    public:
        SubsystemInterface() : SubsystemName() {}

        virtual ~SubsystemInterface() {}
        virtual void Init() {}
        virtual void PostProcessLoad() {}
        virtual void Reset() {}
        virtual void Update() {}
        virtual void Draw() {}

        void Set_Name(AsciiString name);    // Needs confirming.

    private:
        AsciiString SubsystemName;     // Needs confirming.
};

class SubsystemInterfaceList
{
    public:
        SubsystemInterfaceList();

        void Init_Subsystem(SubsystemInterface *sys, char const *path1, char const *path2, char const *dirpath, Xfer *xfer, AsciiString sys_name);
        void Post_Process_Load_All();
        void Reset_All();
        void Shutdown_All();

    private:
        std::vector<SubsystemInterface *> Subsystems;
        std::vector<SubsystemInterface *> MoreSubsystems;   // Needs confirming.
};

#endif // _SUBSYSTEMINTERFACEH_