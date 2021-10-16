/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Pivot
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
#include "matrix3d.h"

struct PivotClass
{
    char name[16];
    PivotClass *parent;
    Matrix3D base_transform;
    Matrix3D transform;
    Matrix3D cap_transform;
    int index;
    bool is_visible;
    bool world_space_translation;
    bool is_captured;
    bool unknown;
    PivotClass(PivotClass &pivot);
    PivotClass();
    ~PivotClass();
    void Capture_Update();
    PivotClass &operator=(PivotClass &pivot);
};
