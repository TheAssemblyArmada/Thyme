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
#pragma once

#include "always.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include <captainslog.h>

enum
{
    TRI_RAYCAST_FLAG_NONE = 0x00,
    TRI_RAYCAST_FLAG_HIT_EDGE = 0x01,
    TRI_RAYCAST_FLAG_START_IN_TRI = 0x02
};

class TriClass
{
public:
    void Compute_Normal()
    {
        captainslog_assert(N != nullptr);
        captainslog_assert(V[0] != nullptr);
        captainslog_assert(V[1] != nullptr);
        captainslog_assert(V[2] != nullptr);

        // #TODO This is a bad const_cast and should be fixed.

        Vector3::Cross_Product(*(V[1]) - *(V[0]), *(V[2]) - *(V[0]), const_cast<Vector3 *>(N));
        const_cast<Vector3 *>(N)->Normalize();
    }

    bool Contains_Point(const Vector3 &ipoint) const;
    void Find_Dominant_Plane(int *axis1, int *axis2) const;

    const Vector3 *N;
    const Vector3 *V[3];
};

inline bool Point_In_Triangle_2D(const Vector3 &tri_point0,
    const Vector3 &tri_point1,
    const Vector3 &tri_point2,
    const Vector3 &test_point,
    int axis_1,
    int axis_2,
    unsigned char &flags)
{
    Vector2 p0p1(tri_point1[axis_1] - tri_point0[axis_1], tri_point1[axis_2] - tri_point0[axis_2]);
    Vector2 p1p2(tri_point2[axis_1] - tri_point1[axis_1], tri_point2[axis_2] - tri_point1[axis_2]);
    Vector2 p2p0(tri_point0[axis_1] - tri_point2[axis_1], tri_point0[axis_2] - tri_point2[axis_2]);
    Vector2 p0p2(tri_point2[axis_1] - tri_point0[axis_1], tri_point2[axis_2] - tri_point0[axis_2]);
    float p0p1p2 = Vector2::Perp_Dot_Product(p0p1, p0p2);

    if (p0p1p2 != 0.0f) {
        float side_factor = p0p1p2 > 0.0f ? 1.0f : -1.0f;
        float factors[3];
        Vector2 p0pT(test_point[axis_1] - tri_point0[axis_1], test_point[axis_2] - tri_point0[axis_2]);
        factors[0] = Vector2::Perp_Dot_Product(p0p1, p0pT);

        if (factors[0] * side_factor < 0.0f) {
            return false;
        }

        Vector2 p1pT(test_point[axis_1] - tri_point1[axis_1], test_point[axis_2] - tri_point1[axis_2]);
        factors[1] = Vector2::Perp_Dot_Product(p1p2, p1pT);

        if (factors[1] * side_factor < 0.0f) {
            return false;
        }

        Vector2 p2pT(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
        factors[2] = Vector2::Perp_Dot_Product(p2p0, p2pT);

        if (factors[2] * side_factor < 0.0f) {
            return false;
        }

        if ((factors[0] == 0.0f) || (factors[1] == 0.0f) || (factors[2] == 0.0f)) {
            flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
        }

        return true;

    } else {
        float p0p1dist2 = p0p1.Length2();
        float p1p2dist2 = p1p2.Length2();
        float p2p0dist2 = p1p2.Length2();
        float max_dist2;
        Vector2 pSpE, pSpT;

        if (p0p1dist2 > p1p2dist2) {
            if (p0p1dist2 > p2p0dist2) {
                pSpE = p0p1;
                pSpT.Set(test_point[axis_1] - tri_point0[axis_1], test_point[axis_2] - tri_point0[axis_2]);
                max_dist2 = p0p1dist2;
            } else {
                pSpE = p2p0;
                pSpT.Set(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
                max_dist2 = p2p0dist2;
            }
        } else {
            if (p1p2dist2 > p2p0dist2) {
                pSpE = p1p2;
                pSpT.Set(test_point[axis_1] - tri_point1[axis_1], test_point[axis_2] - tri_point1[axis_2]);
                max_dist2 = p1p2dist2;
            } else {
                pSpE = p2p0;
                pSpT.Set(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
                max_dist2 = p2p0dist2;
            }
        }

        if (max_dist2 != 0.0f) {
            if (Vector2::Perp_Dot_Product(pSpE, pSpT)) {
                return false;
            } else {
                Vector2 pEpT = pSpT - pSpE;

                if (pSpT.Length2() <= max_dist2 && pEpT.Length2() <= max_dist2) {
                    flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
                    return true;
                } else {
                    return false;
                }
            }
        } else {
            if (pSpT.Length2() == 0.0f) {
                flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
                return true;
            } else {
                return false;
            }
        }
    }
}

inline bool Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(const Vector3 &tri_point0,
    const Vector3 &tri_point1,
    const Vector3 &tri_point2,
    const Vector4 &tri_plane,
    const Vector3 &ray_start,
    int axis_r,
    int axis_1,
    int axis_2,
    int direction,
    unsigned char &flags)
{
    bool retval = false;
    unsigned char flags_2d = TRI_RAYCAST_FLAG_NONE;

    if (Point_In_Triangle_2D(tri_point0, tri_point1, tri_point2, ray_start, axis_1, axis_2, flags_2d)) {
        static const float sign[2] = { -1.0f, 1.0f };
        float result = tri_plane[axis_r] * sign[direction]
            * (tri_plane.X * ray_start.X + tri_plane.Y * ray_start.Y + tri_plane.Z * ray_start.Z + tri_plane.W);

        if (result < 0.0f) {
            flags |= (flags_2d & TRI_RAYCAST_FLAG_HIT_EDGE);
            retval = true;
        } else {
            if (result == 0.0f) {
                if (tri_plane[axis_r]) {
                    flags |= (flags_2d & TRI_RAYCAST_FLAG_HIT_EDGE);
                    flags |= TRI_RAYCAST_FLAG_START_IN_TRI;
                    retval = true;
                } else {
                    TriClass tri;
                    tri.V[0] = &tri_point0;
                    tri.V[1] = &tri_point1;
                    tri.V[2] = &tri_point2;
                    tri.N = Thyme::To_Vector3_Ptr(&tri_plane);

                    if (tri.Contains_Point(ray_start)) {
                        flags |= TRI_RAYCAST_FLAG_START_IN_TRI;
                    }
                }
            }
        }
    }

    return retval;
}
