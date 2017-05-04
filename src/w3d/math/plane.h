////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: PLANE.H
//
//        Author:: Tiberian Technologies
//
//  Contributors:: OmniBlade
//
//   Description:: Plane class
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef PLANE_H
#define PLANE_H

#include "vector3.h"
#include "sphere.h"

class PlaneClass
{
public:
    PlaneClass() : N(0.0f, 0.0f, 1.0f), D(0.0f) {}

    PlaneClass(float nx, float ny, float nz, float dist)
    {
        Set(nx, ny, nz, dist);
    }

    PlaneClass(const Vector3 &normal, float dist)
    {
        Set(normal, dist);
    }

    PlaneClass(const Vector3 &normal, const Vector3 &point)
    {
        Set(normal, point);
    }

    PlaneClass(const Vector3 &point1, const Vector3 &point2, const Vector3 &point3)
    {
        Set(point1, point2, point3);
    }

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

        if ( N != Vector3(0.0f, 0.0f, 0.0f) ) {
            // Points are not colinear. Normalize N and calculate D.
            N.Normalize();
            D = N * point1;
        } else {
            // They are colinear - return default plane (constructors can't fail).
            N = Vector3(0.0f, 0.0f, 1.0f);
            D = 0.0f;
        }
    }

    inline void Normalize()
    {
        float oolength = 1.0f / N.Length();
        N *= oolength;
        D *= oolength;
    }

    inline PlaneClass operator-()
    {
        return PlaneClass(-N, D);
    }

    bool In_Front(const Vector3 &point) const;
    bool In_Front(const SphereClass &sphere) const;
    bool Compute_Intersection(const Vector3 &p0, const Vector3 &p1, float *set_t) const;

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

    if ( den == 0.0f ) {
        return false;
    }

    num = -(Vector3::Dot_Product(N, p0) - D);
    *set_t = num / den;
    
    if ( (*set_t < 0.0f) || (*set_t > 1.0f) ) {
        return false;
    }

    return true;
}

#endif
