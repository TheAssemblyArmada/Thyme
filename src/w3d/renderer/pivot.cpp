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
#include "pivot.h"

PivotClass::PivotClass() :
    parent(nullptr),
    base_transform(true),
    transform(true),
    cap_transform(true),
    index(0),
    is_visible(true),
    world_space_translation(false),
    is_captured(false),
    unknown(false)
{
    name[0] = 0;
}

PivotClass::PivotClass(PivotClass &that) :
    parent(that.parent),
    base_transform(that.base_transform),
    transform(that.transform),
    cap_transform(that.cap_transform),
    index(that.index),
    is_visible(that.is_visible),
    world_space_translation(that.world_space_translation),
    is_captured(that.is_captured),
    unknown(that.unknown)
{
    strlcpy(name, that.name, sizeof(name));
}

PivotClass::~PivotClass() {}

void PivotClass::Capture_Update()
{
    if (world_space_translation) {
        Matrix3D CapOrientation = cap_transform;
        CapOrientation.Set_Translation(Vector3(0, 0, 0));
        Matrix3D::Multiply(transform, CapOrientation, &transform);
        transform.Adjust_Translation(cap_transform.Get_Translation());
    } else {
        transform.Post_Mul(cap_transform);
    }
}

PivotClass &PivotClass::operator=(PivotClass &that)
{
    if (this != &that) {
        strlcpy(name, that.name, sizeof(name));
        parent = that.parent;
        base_transform = that.base_transform;
        transform = that.transform;
        cap_transform = that.cap_transform;
        index = that.index;
        is_visible = that.is_visible;
        world_space_translation = that.world_space_translation;
        is_captured = that.is_captured;
        unknown = that.unknown;
    }
    return *this;
}
