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
#include "squad.h"

void Squad::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short object_id_count = static_cast<unsigned short>(m_objectIDs.size());
    xfer->xferUnsignedShort(&object_id_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_objectIDs.begin(); i != m_objectIDs.end(); i++) {
            ObjectID id = *i;
            xfer->xferObjectID(&id);
        }
    } else {
        captainslog_relassert(m_objectsCached.size() == 0, 6, "Squad::xfer - m_objectsCached should be emtpy, but is not");
        for (int i = 0; i < object_id_count; i++) {
            ObjectID id;
            xfer->xferObjectID(&id);
            m_objectIDs.push_back(id);
        }
    }
}

bool Squad::Is_On_Squad(const Object *obj) const
{
    ObjectID id = obj->Get_ID();

    for (auto i = m_objectIDs.begin(); i != m_objectIDs.end(); i++) {
        if (id == (*i)) {
            return true;
        }
    }

    return false;
}
