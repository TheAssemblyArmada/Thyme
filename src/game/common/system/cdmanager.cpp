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
#include "cdmanager.h"
#include "captainslog.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
CDManagerInterface *g_theCDManager;
#endif

class CDManager : public CDManagerInterface
{
public:
    CDManager() {}
    ~CDManager() override {}
    void Init() override {}
    void Reset() override {}
    void Update() override {}

    int Drive_Count() override
    {
        captainslog_relassert(false, 0, "CDManager error");
        return 0;
    }

    CDDriveInterface *Get_Drive() override
    {
        captainslog_relassert(false, 0, "CDManager error");
        return nullptr;
    }

    CDDriveInterface *New_Drive(const char *drive) override
    {
        captainslog_relassert(false, 0, "CDManager error");
        return nullptr;
    }

    void Refresh_Drives() override { captainslog_relassert(false, 0, "CDManager error"); }
    void Destroy_All_Drives() override { captainslog_relassert(false, 0, "CDManager error"); }
};

CDManagerInterface *Create_CD_Manager()
{
    return new CDManager;
}
