/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Returns the results of a ray cast?
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
#include "w3d_file.h"

struct CastResultStruct
{
    CastResultStruct() { Reset(); }

    void Reset()
    {
        start_bad = false;
        fraction = 1.0f;
        normal.Set(0.0f, 0.0f, 0.0f);
        surface_type = SURFACE_TYPE_LIGHT_METAL;
        compute_contact_point = false;
        contact_point.Set(0.0f, 0.0f, 0.0f);
    }

    bool start_bad;
    float fraction;
    Vector3 normal;
    uint32_t surface_type;
    bool compute_contact_point;
    Vector3 contact_point;
};
