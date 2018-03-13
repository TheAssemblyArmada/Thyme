/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for "things".
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thing.h"

Thing::Thing(const ThingTemplate *thing) : m_template(nullptr)
{
    if (thing != nullptr) {
        m_transform.Make_Identity();
        m_cachedPos = {0.0f, 0.0f, 0.0f};
        m_cachedAngle = 0.0f;
        m_cachedDirVector = { 0.0f, 0.0f, 0.0f };
        m_cachedAltitudeAboveTerrain = 0.0f;
        m_cachedAltitudeAboveTerrainOrWater = 0.0f;
        m_cacheFlags = 0;
    }
}
