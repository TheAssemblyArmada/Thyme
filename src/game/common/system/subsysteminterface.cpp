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
#include "ini.h"
#include "xfer.h"

//SubsystemInterfaceList *g_theSubsystemList = nullptr;

////////////
// Interface
////////////
void SubsystemInterface::Set_Name(AsciiString name)
{
    m_subsystemName = name;
}

/////////////////
// Interface List
/////////////////
void SubsystemInterfaceList::Init_Subsystem(SubsystemInterface *sys, const char *default_ini_path, const char *ini_path, const char *dir_path, Xfer *xfer, AsciiString sys_name)
{
    INI ini;

    sys->Set_Name(sys_name);
    sys->Init();

    if ( default_ini_path != nullptr ) {
        ini.Load(default_ini_path, INI_LOAD_OVERWRITE, xfer);
    }

    if ( ini_path != nullptr ) {
        ini.Load(ini_path, INI_LOAD_OVERWRITE, xfer);
    }

    if ( dir_path != nullptr ) {
        ini.Load_Directory(dir_path, true, INI_LOAD_OVERWRITE, xfer);
    }

    m_subsystems.push_back(sys);
}

void SubsystemInterfaceList::Post_Process_Load_All()
{
    for ( auto it = m_subsystems.begin(); it != m_subsystems.end(); ++it ) {
        (*it)->PostProcessLoad();
    }
}

void SubsystemInterfaceList::Reset_All()
{
    for ( auto it = m_subsystems.begin(); it != m_subsystems.end(); ++it ) {
        (*it)->Reset();
    }
}

void SubsystemInterfaceList::Shutdown_All()
{
    for ( auto it = m_subsystems.end(); it != m_subsystems.begin(); --it ) {
        (*it)->~SubsystemInterface();
    }

    m_subsystems.erase(m_subsystems.begin(), m_subsystems.end());
}
