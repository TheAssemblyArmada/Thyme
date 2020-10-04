/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Axis-Aligned bounding box
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aabox.h"

void AABoxClass::Init_Random(float min_center, float max_center, float min_extent, float max_extent)
{
    float center = max_center - min_center;
    m_center.X = GameMath::Random_Float() * center + min_center;
    m_center.Y = GameMath::Random_Float() * center + min_center;
    m_center.Z = GameMath::Random_Float() * center + min_center;

    float extent = max_extent - min_extent;
    m_extent.X = GameMath::Random_Float() * extent + min_extent;
    m_extent.Y = GameMath::Random_Float() * extent + min_extent;
    m_extent.Z = GameMath::Random_Float() * extent + min_extent;
}

void AABoxClass::Transform(const Matrix3D &tm, const AABoxClass &in, AABoxClass *out)
{
    tm.Transform_Center_Extent_AABox(in.m_center, in.m_extent, &out->m_center, &out->m_extent);
}

void MinMaxAABoxClass::Init_Empty()
{
    m_minCorner.X = GAMEMATH_FLOAT_MAX;
    m_minCorner.Y = GAMEMATH_FLOAT_MAX;
    m_minCorner.Z = GAMEMATH_FLOAT_MAX;
    m_maxCorner.X = -GAMEMATH_FLOAT_MAX;
    m_maxCorner.Y = -GAMEMATH_FLOAT_MAX;
    m_maxCorner.Z = -GAMEMATH_FLOAT_MAX;
}
