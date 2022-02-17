/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief 4D Vector class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vector4.h"

#include "vector3.h"

static_assert(offsetof(Vector4, X) == offsetof(Vector3, X), "Must match if Vector pointers are meant to be compatible");
static_assert(offsetof(Vector4, Y) == offsetof(Vector3, Y), "Must match if Vector pointers are meant to be compatible");
static_assert(offsetof(Vector4, Z) == offsetof(Vector3, Z), "Must match if Vector pointers are meant to be compatible");
static_assert(sizeof(Vector3) == 12, "Must match if Vector pointers are meant to be compatible");
