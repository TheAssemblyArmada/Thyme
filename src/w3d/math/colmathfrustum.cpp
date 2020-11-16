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
#include "always.h"
#include "aabox.h"
#include "colmath.h"
#include "frustum.h"
#include "lineseg.h"
#include "vector3.h"

CollisionMath::OverlapType CollisionMath::Overlap_Test(const FrustumClass &frustum, const Vector3 &point)
{
    int flags = 0;

    for (int i = 0; i < ARRAY_SIZE(frustum.m_planes); ++i) {
        OverlapType res = Overlap_Test(frustum.m_planes[i], point);

        if (res == OUTSIDE) {
            return OUTSIDE;
        }

        flags |= res;
    }

    if (flags == INSIDE) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const FrustumClass &frustum, const TriClass &tri)
{
    int flags = 0;

    for (int i = 0; i < ARRAY_SIZE(frustum.m_planes); ++i) {
        OverlapType res = Overlap_Test(frustum.m_planes[i], tri);

        if (res == OUTSIDE) {
            return OUTSIDE;
        }

        flags |= res;
    }

    if (flags == INSIDE) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const FrustumClass &frustum, const SphereClass &sphere)
{
    int flags = 0;

    for (int i = 0; i < ARRAY_SIZE(frustum.m_planes); ++i) {
        OverlapType res = Overlap_Test(frustum.m_planes[i], sphere);

        if (res == OUTSIDE) {
            return OUTSIDE;
        }

        flags |= res;
    }

    if (flags == INSIDE) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const FrustumClass &frustum, const AABoxClass &box)
{
    int flags = 0;

    for (int i = 0; i < ARRAY_SIZE(frustum.m_planes); ++i) {
        OverlapType res = Overlap_Test(frustum.m_planes[i], box);

        if (res == OUTSIDE) {
            return OUTSIDE;
        }

        flags |= res;
    }

    if (flags == INSIDE) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(
    const FrustumClass &frustum, const AABoxClass &box, int &planes_passed)
{
    int flags = 0;

    for (int i = 0; i < 6; i++) {
        int plane_bit = (1 << i);

        if ((planes_passed & plane_bit) == 0) {
            OverlapType res = CollisionMath::Overlap_Test(frustum.m_planes[i], box);

            if (res == OUTSIDE) {
                return OUTSIDE;
            } else if (res == INSIDE) {
                planes_passed |= plane_bit;
            }

            flags |= res;
        } else {
            flags |= INSIDE;
        }
    }

    if (flags == INSIDE) {
        return INSIDE;
    }

    return OVERLAPPED;
}
