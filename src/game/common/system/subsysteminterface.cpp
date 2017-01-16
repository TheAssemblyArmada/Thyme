////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SUBSYSTEMINTERFACE.CPP
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

#include "subsysteminterface.h"
////////////
// Interface
////////////
void SubsystemInterface::Set_Name(AsciiString name)
{
    SubsystemName = name;
}

/////////////////
// Interface List
/////////////////
void SubsystemInterfaceList::Init_Subsystem(SubsystemInterface *sys, char const *path1, char const *path2, char const *dirpath, Xfer *xfer, AsciiString sys_name)
{
    // TODO, requires INI
}

void SubsystemInterfaceList::Post_Process_Load_All()
{
    for (
        std::vector<SubsystemInterface*>::iterator it = Subsystems.begin();
        it != Subsystems.end();
        ++it
    ) {
        (*it)->PostProcessLoad();
    }
}

void SubsystemInterfaceList::Reset_All()
{
    for (
        std::vector<SubsystemInterface*>::iterator it = Subsystems.begin();
        it != Subsystems.end();
        ++it
    ) {
        (*it)->Reset();
    }
}

void SubsystemInterfaceList::Shutdown_All()
{
    for (
        std::vector<SubsystemInterface*>::iterator it = Subsystems.end();
        it != Subsystems.begin();
        --it
    ) {
        (*it)->~SubsystemInterface();
    }

    Subsystems.erase(Subsystems.begin(), Subsystems.end());
}
