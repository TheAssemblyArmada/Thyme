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

#include "always.h"
#include "asciistring.h"
#include <vector>

class Xfer;

class SubsystemInterface
{
public:
    SubsystemInterface();

    virtual ~SubsystemInterface();
    virtual void Init() = 0;
    virtual void PostProcessLoad() {}
    virtual void Reset() = 0;
    virtual void Update() = 0;
    virtual void Draw() {}

    void Set_Name(Utf8String name);

private:
#ifdef GAME_DEBUG_STRUCTS
    float m_updateTimeTracker;
    float m_updateTime;
    float m_drawTimeTracker;
    float m_drawTime;
    bool m_logUpdateTime;
    bool m_logDrawTime;
#endif
    Utf8String m_subsystemName;

#ifdef GAME_DLL
    static float &s_totalSubsystemTime;
#else
    static float s_totalSubsystemTime;
#endif
};

class SubsystemInterfaceList
{
public:
    SubsystemInterfaceList() {}
    ~SubsystemInterfaceList();
    void Init_Subsystem(SubsystemInterface *sys,
        const char *default_ini_path,
        const char *ini_path,
        const char *dir_path,
        Xfer *xfer,
        Utf8String sys_name);
    void Post_Process_Load_All();
    void Reset_All();
    void Shutdown_All();
    void Add_Subsystem(SubsystemInterface *sys);
    void Remove_Subsystem(SubsystemInterface *sys);

private:
    std::vector<SubsystemInterface *> m_subsystems;
    std::vector<SubsystemInterface *> m_profiledSubsystems;
};

#ifdef GAME_DLL
extern SubsystemInterfaceList *&g_theSubsystemList;
#else
extern SubsystemInterfaceList *g_theSubsystemList;
#endif

template<typename T>
void Init_Subsystem(T *&instance,
    Utf8String name,
    T *sys,
    Xfer *xfer = nullptr,
    const char *default_ini_path = nullptr,
    const char *ini_path = nullptr,
    const char *dir_path = nullptr)
{
    instance = sys;
    g_theSubsystemList->Init_Subsystem(sys, default_ini_path, ini_path, dir_path, xfer, name);
}
