/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Triangle class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "tri.h"

void TriClass::Find_Dominant_Plane(int *axis1, int *axis2) const
{
    int side = 0;
    float x = GameMath::Fabs(N->X);
    float y = GameMath::Fabs(N->Y);
    float z = GameMath::Fabs(N->Z);

    if (y > x) {
        side = 1;
        x = y;
    }

    if (z > x) {
        side = 2;
    }

    if (side == 0) {
        *axis1 = 1;
        *axis2 = 2;
    } else if (side == 1) {
        *axis1 = 0;
        *axis2 = 2;
    } else if (side == 2) {
        *axis1 = 0;
        *axis2 = 1;
    }
}

bool TriClass::Contains_Point(const Vector3 &ipoint) const
{
    bool side[3];
    int axis1;
    int axis2;
    Find_Dominant_Plane(&axis1, &axis2);
    Vector2 v1;
    Vector2 v2;

    for (int i = 0; i < 3; i++) {
        int i1 = (i + 1) % 3;
        v1.Set((*V[i1])[axis1] - (*V[i])[axis1], (*V[i1])[axis2] - (*V[i])[axis2]);
        v2.Set(ipoint[axis1] - (*V[i])[axis1], ipoint[axis2] - (*V[i])[axis2]);
        side[i] = (v1.X * v2.Y - v1.Y * v2.X) >= 0.0f;
    }
    return side[0] == side[1] && side[1] == side[2];
}
