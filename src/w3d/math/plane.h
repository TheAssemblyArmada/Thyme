/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Plane class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "sphere.h"
#include "vector3.h"

class PlaneClass
{
public:
    enum
    {
        FRONT = 0,
        BACK,
        ON
    };

    PlaneClass() : N(0.0f, 0.0f, 1.0f), D(0.0f) {}

    PlaneClass(float nx, float ny, float nz, float dist) { Set(nx, ny, nz, dist); }

    PlaneClass(const Vector3 &normal, float dist) { Set(normal, dist); }

    PlaneClass(const Vector3 &normal, const Vector3 &point) { Set(normal, point); }

    PlaneClass(const Vector3 &point1, const Vector3 &point2, const Vector3 &point3) { Set(point1, point2, point3); }

    inline void Set(float a, float b, float c, float d)
    {
        N.X = a;
        N.Y = b;
        N.Z = c;
        D = d;
    }

    inline void Set(const Vector3 &normal, float dist)
    {
        N = normal;
        D = dist;
    }

    inline void Set(const Vector3 &normal, const Vector3 &point)
    {
        N = normal;
        D = Vector3::Dot_Product(normal, point);
    }

    inline void Set(const Vector3 &point1, const Vector3 &point2, const Vector3 &point3)
    {
        N = Vector3::Cross_Product((point2 - point1), (point3 - point1));

        if (N != Vector3(0.0f, 0.0f, 0.0f)) {
            // Points are not colinear. Normalize N and calculate D.
            N.Normalize();
            D = N * point1;
        } else {
            // They are colinear - return default plane (constructors can't fail).
            N = Vector3(0.0f, 0.0f, 1.0f);
            D = 0.0f;
        }
    }

    bool In_Front(const Vector3 &point) const;
    bool In_Front(const SphereClass &sphere) const;
    bool Compute_Intersection(const Vector3 &p0, const Vector3 &p1, float *set_t) const;
    bool In_Front_Or_Intersecting(const SphereClass &sphere) const;
    void Intersect_Planes(const PlaneClass &a, const PlaneClass &b, Vector3 *line_dir, Vector3 *line_point);

public:
    Vector3 N;
    float D;
};

inline bool PlaneClass::In_Front(const Vector3 &point) const
{
    float dist = Vector3::Dot_Product(point, N);
    return (dist > D);
}

inline bool PlaneClass::In_Front(const SphereClass &sphere) const
{
    float dist = Vector3::Dot_Product(sphere.Center, N);
    return ((dist - D) >= sphere.Radius);
}

inline bool PlaneClass::Compute_Intersection(const Vector3 &p0, const Vector3 &p1, float *set_t) const
{
    float num, den;
    den = Vector3::Dot_Product(N, p1 - p0);

    if (den == 0.0f) {
        return false;
    }

    num = -(Vector3::Dot_Product(N, p0) - D);
    *set_t = num / den;

    if ((*set_t < 0.0f) || (*set_t > 1.0f)) {
        return false;
    }

    return true;
}

inline bool PlaneClass::In_Front_Or_Intersecting(const SphereClass &sphere) const
{
    float dist = Vector3::Dot_Product(sphere.Center, N);
    return ((D - dist) < sphere.Radius);
}

inline void PlaneClass::Intersect_Planes(const PlaneClass &a, const PlaneClass &b, Vector3 *line_dir, Vector3 *line_point)
{
    Vector3::Cross_Product(a.N, b.N, line_dir);
    Vector3 abs_dir = *line_dir;
    abs_dir.Update_Max(-abs_dir);

    if (abs_dir.X > abs_dir.Y) {
        if (abs_dir.X > abs_dir.Z) {
            float ool = 1.0f / line_dir->X;
            line_point->Y = (b.N.Z * a.D - a.N.Z * b.D) * ool;
            line_point->Z = (a.N.Y * b.D - b.N.Y * a.D) * ool;
            line_point->X = 0.0f;
        } else {
            float ool = 1.0f / line_dir->Z;
            line_point->X = (b.N.Y * a.D - a.N.Y * b.D) * ool;
            line_point->Y = (a.N.X * b.D - b.N.X * a.D) * ool;
            line_point->Z = 0.0f;
        }
    } else {
        if (abs_dir.Y > abs_dir.Z) {
            float ool = 1.0f / line_dir->Y;
            line_point->Z = (b.N.X * a.D - a.N.X * b.D) * ool;
            line_point->X = (a.N.Z * b.D - b.N.Z * a.D) * ool;
            line_point->Y = 0.0f;
        } else {
            float ool = 1.0f / line_dir->Z;
            line_point->X = (b.N.Y * a.D - a.N.Y * b.D) * ool;
            line_point->Y = (a.N.X * b.D - b.N.X * a.D) * ool;
            line_point->Z = 0.0f;
        }
    }

    line_dir->Normalize();
}
