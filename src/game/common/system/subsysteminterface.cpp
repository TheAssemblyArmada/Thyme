/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base subsystem class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "subsysteminterface.h"
#include "ini.h"
#include "xfer.h"

#ifndef GAME_DLL
SubsystemInterfaceList *g_theSubsystemList = nullptr;
float SubsystemInterface::s_totalSubsystemTime;
#endif

SubsystemInterface::SubsystemInterface() : m_subsystemName()
{
    if (g_theSubsystemList != nullptr) {
        g_theSubsystemList->Add_Subsystem(this);
    }
}

SubsystemInterface::~SubsystemInterface()
{
    if (g_theSubsystemList != nullptr) {
        g_theSubsystemList->Remove_Subsystem(this);
    }
}

void SubsystemInterface::Set_Name(Utf8String name)
{
    m_subsystemName = name;
}

SubsystemInterfaceList::~SubsystemInterfaceList()
{
    captainslog_dbgassert(m_subsystems.empty(), "not empty");
    Shutdown_All();
}

void SubsystemInterfaceList::Init_Subsystem(SubsystemInterface *sys,
    const char *default_ini_path,
    const char *ini_path,
    const char *dir_path,
    Xfer *xfer,
    Utf8String sys_name)
{
    INI ini;

    sys->Set_Name(sys_name);
    sys->Init();

    if (default_ini_path != nullptr) {
        ini.Load(default_ini_path, INI_LOAD_OVERWRITE, xfer);
    }

    if (ini_path != nullptr) {
        ini.Load(ini_path, INI_LOAD_OVERWRITE, xfer);
    }

    if (dir_path != nullptr) {
        ini.Load_Directory(dir_path, true, INI_LOAD_OVERWRITE, xfer);
    }

    m_subsystems.push_back(sys);
}

void SubsystemInterfaceList::Post_Process_Load_All()
{
    for (auto it = m_subsystems.begin(); it != m_subsystems.end(); ++it) {
        (*it)->PostProcessLoad();
    }
}

void SubsystemInterfaceList::Reset_All()
{
    for (auto it = m_subsystems.rbegin(); it != m_subsystems.rend(); ++it) {
        (*it)->Reset();
    }
}

void SubsystemInterfaceList::Shutdown_All()
{
    for (auto it = m_subsystems.rbegin(); it != m_subsystems.rend(); ++it) {
        (*it)->~SubsystemInterface();
    }

    m_subsystems.clear();
}

void SubsystemInterfaceList::Add_Subsystem(SubsystemInterface *sys)
{
    // Empty in release, adds subsystem to m_profiledSubsystems from the look of it in debug.
    m_profiledSubsystems.push_back(sys);
}

void SubsystemInterfaceList::Remove_Subsystem(SubsystemInterface *sys)
{
    for (auto it = m_profiledSubsystems.begin(); it != m_profiledSubsystems.end(); ++it) {
        if (*it == sys) {
            m_profiledSubsystems.erase(it);
            break;
        }
    }
}
