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
#pragma once

#ifndef SUBSYSTEMINTERFACE_H
#define SUBSYSTEMINTERFACE_H

#include "asciistring.h"
#include "hooker.h"
#include <vector>

class Xfer;

class SubsystemInterface
{
    public:
        SubsystemInterface() : m_subsystemName() {}

        virtual ~SubsystemInterface() {}
        virtual void Init() = 0;
        virtual void PostProcessLoad() {}
        virtual void Reset() = 0;
        virtual void Update() = 0;
        virtual void Draw() {}

        void Set_Name(AsciiString name);    // Needs confirming.

    private:
        AsciiString m_subsystemName;     // Needs confirming.
};

class SubsystemInterfaceList
{
    public:
        SubsystemInterfaceList() : m_subsystems(), m_unksubsystems() {}

        void Init_Subsystem(SubsystemInterface *sys, const char *default_ini_path, const char *ini_path, const char *dir_path, Xfer *xfer, AsciiString sys_name);
        void Post_Process_Load_All();
        void Reset_All();
        void Shutdown_All();

    private:
        std::vector<SubsystemInterface *> m_subsystems;
        std::vector<SubsystemInterface *> m_unksubsystems;   // Needs confirming.
};

#define g_theSubsystemList (Make_Global<SubsystemInterfaceList*>(0x00A29B84))
//extern SubsystemInterfaceList *g_theSubsystemList;

#endif // _SUBSYSTEMINTERFACEH_