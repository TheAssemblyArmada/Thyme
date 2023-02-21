/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief CD Manager
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
#include "subsysteminterface.h"

class CDDriveInterface;

class CDManagerInterface : public SubsystemInterface
{
public:
    CDManagerInterface() {}
    virtual ~CDManagerInterface() override {}
    virtual int Drive_Count() = 0;
    virtual CDDriveInterface *Get_Drive() = 0;
    virtual CDDriveInterface *New_Drive(const char *drive) = 0;
    virtual void Refresh_Drives() = 0;
    virtual void Destroy_All_Drives() = 0;
};

CDManagerInterface *Create_CD_Manager();

#ifdef GAME_DLL
extern CDManagerInterface *&g_theCDManager;
#else
extern CDManagerInterface *g_theCDManager;
#endif
