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
#include "resourcegatheringmanager.h"

ResourceGatheringManager::~ResourceGatheringManager()
{
    m_supplyWarehouses.erase(m_supplyWarehouses.begin(), m_supplyWarehouses.end());
    m_supplyCenters.erase(m_supplyCenters.begin(), m_supplyCenters.end());
}

void ResourceGatheringManager::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferSTLObjectIDList(&m_supplyWarehouses);
    xfer->xferSTLObjectIDList(&m_supplyCenters);
}
