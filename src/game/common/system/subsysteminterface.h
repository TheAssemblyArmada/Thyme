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
#pragma once

#include "asciistring.h"
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

    void Set_Name(Utf8String name); // Needs confirming.

private:
#ifdef GAME_DEBUG_STRUCTS
    int32_t m_unkInt1;
    int32_t m_unkInt2;
    int32_t m_unkInt3;
    int32_t m_unkInt4;
    bool m_unkBool1;
    bool m_unkBool2;
#endif
    Utf8String m_subsystemName; // Needs confirming.
};

class SubsystemInterfaceList
{
public:
    SubsystemInterfaceList() : m_subsystems(), m_unksubsystems() {}

    void Init_Subsystem(SubsystemInterface *sys, const char *default_ini_path, const char *ini_path, const char *dir_path,
        Xfer *xfer, Utf8String sys_name);
    void Post_Process_Load_All();
    void Reset_All();
    void Shutdown_All();

private:
    std::vector<SubsystemInterface *> m_subsystems;
    std::vector<SubsystemInterface *> m_unksubsystems; // Needs confirming.
};

#ifdef GAME_DLL
extern SubsystemInterfaceList *&g_theSubsystemList;
#else
extern SubsystemInterfaceList *g_theSubsystemList;
#endif