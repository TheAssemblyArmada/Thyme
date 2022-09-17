/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief intersection class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "intersec.h"

Vector3 IntersectionClass::s_rayLocation(0, 0, 0);
Vector3 IntersectionClass::s_rayDirection(0, 0, 0);
Vector3 IntersectionClass::s_intersectionNormal(0, 0, 0);
