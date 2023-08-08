/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Resource Gathering Manager
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
#include "mempoolobj.h"
#include "object.h"
#include "snapshot.h"

class ResourceGatheringManager : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ResourceGatheringManager)

public:
    ResourceGatheringManager() {}
    ~ResourceGatheringManager() override;

    void CRC_Snapshot(Xfer *xfer) override {}
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override {}

private:
    std::list<ObjectID> m_supplyWarehouses;
    std::list<ObjectID> m_supplyCenters;
};
