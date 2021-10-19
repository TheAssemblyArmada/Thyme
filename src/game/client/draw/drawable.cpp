/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "drawable.h"
#include "object.h"

const Matrix3D *Drawable::Get_Transform_Matrix() const
{
    const Object *object = Get_Object();

    if (object != nullptr) {
        return object->Get_Transform_Matrix();
    }

    return Thing::Get_Transform_Matrix();
}
