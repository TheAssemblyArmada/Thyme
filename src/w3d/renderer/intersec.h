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
#pragma once

#include "always.h"
#include "coltype.h"
#include "matrix3d.h"
#include "sphere.h"
#include "vector3.h"
class RenderObjClass;

typedef unsigned short POLYGONINDEX;
class IntersectionResultClass
{
public:
    RenderObjClass *m_intersectedRenderObject;
    POLYGONINDEX m_intersectedPolygon;
    Matrix3D m_modelMatrix;
    Vector3 m_modelLocation;
    Vector3 m_intersection;
    float m_range;
    float m_alpha;
    float m_beta;
    bool m_intersects;
    int m_CollisionType;

    enum INTERSECTION_TYPE
    {
        NONE = 0,
        GENERIC,
        POLYGON
    } m_intersectionType;
};

class IntersectionClass
{
public:
    enum
    {
        MAX_POLY_INTERSECTION_COUNT = 1000,
        MAX_HIERARCHY_NODE_COUNT = 256
    };

    Vector3 *m_rayLocation;
    Vector3 *m_rayDirection;
    Vector3 *m_intersectionNormal;
    float m_screenX;
    float m_screenY;
    bool m_interpolateNormal;
    bool m_convexTest;
    float m_maxDistance;
    IntersectionResultClass m_result;

    IntersectionClass() :
        // bugfix, init all members
        m_screenX(0),
        m_screenY(0),
        m_interpolateNormal(false),
        m_convexTest(false),
        m_maxDistance(GAMEMATH_FLOAT_MAX),
        m_result()
    {
        m_rayLocation = &s_rayLocation;
        m_rayDirection = &s_rayDirection;
        m_intersectionNormal = &s_intersectionNormal;

        m_result.m_CollisionType = COLLISION_TYPE_ALL;
    }

    virtual ~IntersectionClass() {}

    bool Intersect_Sphere_Quick(SphereClass &Sphere, IntersectionResultClass *FinalResult)
    {
        Vector3 sphere_vector(Sphere.Center - *m_rayLocation);
        FinalResult->m_alpha = Vector3::Dot_Product(sphere_vector, *m_rayDirection);
        FinalResult->m_beta = Sphere.Radius * Sphere.Radius
            - (Vector3::Dot_Product(sphere_vector, sphere_vector) - FinalResult->m_alpha * FinalResult->m_alpha);

        if (FinalResult->m_beta < 0.0f) {
            FinalResult->m_intersects = false;
        }

        FinalResult->m_intersects = true;
        return FinalResult->m_intersects;
    }

    bool Intersect_Sphere(SphereClass &Sphere, IntersectionResultClass *FinalResult)
    {
        if (!Intersect_Sphere_Quick(Sphere, FinalResult)) {
            return false;
        }

        float d = GameMath::Sqrt(FinalResult->m_beta);
        FinalResult->m_range = FinalResult->m_alpha - d;

        if (FinalResult->m_range > m_maxDistance) {
            return false;
        }

        FinalResult->m_intersection = *m_rayLocation + FinalResult->m_range * (*m_rayDirection);

        if (m_intersectionNormal != nullptr) {
            (*m_intersectionNormal) = FinalResult->m_intersection - Sphere.Center;
        }

        return true;
    }

    static Vector3 s_rayLocation;
    static Vector3 s_rayDirection;
    static Vector3 s_intersectionNormal;
};
