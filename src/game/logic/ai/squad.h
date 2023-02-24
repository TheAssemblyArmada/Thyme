/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Squad
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

class Squad : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Squad)

public:
    virtual ~Squad() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

private:
    std::vector<ObjectID> m_objectIDs;
    std::vector<Object *> m_objectsCached;
};
