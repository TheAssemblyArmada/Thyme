/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Collision types
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
enum
{
    COLLISION_TYPE_ALL = 0x01,
    COLLISION_TYPE_0 = 0x02,
    COLLISION_TYPE_1 = 0x04,
    COLLISION_TYPE_2 = 0x08,
    COLLISION_TYPE_3 = 0x10,
    COLLISION_TYPE_4 = 0x20,
    COLLISION_TYPE_5 = 0x40,
    COLLISION_TYPE_6 = 0x80,
    COLLISION_TYPE_PHYSICAL = COLLISION_TYPE_0,
    COLLISION_TYPE_PROJECTILE = COLLISION_TYPE_1,
    COLLISION_TYPE_VIS = COLLISION_TYPE_2,
    COLLISION_TYPE_CAMERA = COLLISION_TYPE_3,
    COLLISION_TYPE_VEHICLE = COLLISION_TYPE_4,
};
