/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Defines an "axis-aligned" plane.
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
#include "vector3.h"

class AAPlaneClass
{
public:
    enum AxisEnum
    {
        XNORMAL = 0,
        YNORMAL = 1,
        ZNORMAL = 2
    };

    AAPlaneClass() {}
    AAPlaneClass(AxisEnum normal, float dist) : m_normal(normal), m_dist(dist) {}

    void Set(AxisEnum normal, float dist)
    {
        m_normal = normal;
        m_dist = dist;
    }
    void Get_Normal(Vector3 *normal) const
    {
        normal->Set(0, 0, 0);
        (*normal)[m_normal] = 1.0f;
    }

public:
    AxisEnum m_normal;
    float m_dist;
};
