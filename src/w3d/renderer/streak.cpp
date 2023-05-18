/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Streak Line
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "streak.h"
void StreakLineClass::Prepare_LOD(CameraClass &camera)
{
    m_normalizedScreenArea = Get_Screen_Size(camera);
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();

    if (level >= static_cast<unsigned int>(m_maxSubdivisionLevels)) {
        m_lineRenderer.Set_Current_Subdivision_Level(m_maxSubdivisionLevels);
    } else {
        m_lineRenderer.Set_Current_Subdivision_Level(level);
    }
}