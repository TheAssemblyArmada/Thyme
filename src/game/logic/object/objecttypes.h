/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief List of objects.
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
#include "mempoolobj.h"
#include "snapshot.h"
#include <vector>

class ObjectTypes : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ObjectTypes);

public:
    ObjectTypes() {}
    ObjectTypes(const Utf8String &list_name);
    virtual ~ObjectTypes() override {}

    // Snapshot virtuals
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() override {}

private:
    Utf8String m_listName;
    std::vector<Utf8String> m_objectTypes;
};
