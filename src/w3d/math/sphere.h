/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief Sphere object.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "gamemath.h"
#include "matrix3d.h"
#include "vector3.h"

class SphereClass
{
public:
    SphereClass() {}
    SphereClass(const Vector3 &center, float radius) { Init(center, radius); }
    SphereClass(const Vector3 &center, const SphereClass &s0);
    SphereClass(const Vector3 *Position, const int VertCount);

    void Init(const Vector3 &pos, float radius);
    void Re_Center(const Vector3 &center);
    void Add_Sphere(const SphereClass &s);
    void Transform(const Matrix3D &tm);
    float Volume() const;

    SphereClass &operator+=(const SphereClass &s);
    SphereClass &operator*=(const Matrix3D &m);

public:
    Vector3 Center;
    float Radius;
};

inline SphereClass::SphereClass(const Vector3 &center, const SphereClass &s0)
{
    float dist = (s0.Center - center).Length();
    Center = center;
    Radius = s0.Radius + dist;
}

inline SphereClass::SphereClass(const Vector3 *Position, const int VertCount)
{
    int i;
    float dx, dy, dz;
    Vector3 xmin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 xmax(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 ymin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 ymax(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 zmin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 zmax(Position[0].X, Position[0].Y, Position[0].Z);

    for (i = 1; i < VertCount; i++) {
        if (Position[i].X < xmin.X) {
            xmin.X = Position[i].X;
            xmin.Y = Position[i].Y;
            xmin.Z = Position[i].Z;
        }

        if (Position[i].X > xmax.X) {
            xmax.X = Position[i].X;
            xmax.Y = Position[i].Y;
            xmax.Z = Position[i].Z;
        }

        if (Position[i].Y < ymin.Y) {
            ymin.X = Position[i].X;
            ymin.Y = Position[i].Y;
            ymin.Z = Position[i].Z;
        }

        if (Position[i].Y > ymax.Y) {
            ymax.X = Position[i].X;
            ymax.Y = Position[i].Y;
            ymax.Z = Position[i].Z;
        }

        if (Position[i].Z < zmin.Z) {
            zmin.X = Position[i].X;
            zmin.Y = Position[i].Y;
            zmin.Z = Position[i].Z;
        }

        if (Position[i].Z > zmax.Z) {
            zmax.X = Position[i].X;
            zmax.Y = Position[i].Y;
            zmax.Z = Position[i].Z;
        }
    }

    dx = xmax.X - xmin.X;
    dy = xmax.Y - xmin.Y;
    dz = xmax.Z - xmin.Z;
    float xspan = dx * dx + dy * dy + dz * dz;

    dx = ymax.X - ymin.X;
    dy = ymax.Y - ymin.Y;
    dz = ymax.Z - ymin.Z;
    float yspan = dx * dx + dy * dy + dz * dz;

    dx = zmax.X - zmin.X;
    dy = zmax.Y - zmin.Y;
    dz = zmax.Z - zmin.Z;
    float zspan = dx * dx + dy * dy + dz * dz;

    Vector3 dia1 = xmin;
    Vector3 dia2 = xmax;
    float maxspan = xspan;

    if (yspan > maxspan) {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }

    if (zspan > maxspan) {
        maxspan = zspan;
        dia1 = zmin;
        dia2 = zmax;
    }

    Vector3 center;
    center.X = (dia1.X + dia2.X) / 2.0f;
    center.Y = (dia1.Y + dia2.Y) / 2.0f;
    center.Z = (dia1.Z + dia2.Z) / 2.0f;
    dx = dia2.X - center.X;
    dy = dia2.Y - center.Y;
    dz = dia2.Z - center.Z;
    float radsqr = dx * dx + dy * dy + dz * dz;
    float radius = GameMath::Sqrt(radsqr);

    for (i = 0; i < VertCount; i++) {
        dx = Position[i].X - center.X;
        dy = Position[i].Y - center.Y;
        dz = Position[i].Z - center.Z;
        float testrad2 = dx * dx + dy * dy + dz * dz;

        if (testrad2 > radsqr) {
            float testrad = GameMath::Sqrt(testrad2);
            radius = (radius + testrad) / 2.0f;
            radsqr = radius * radius;
            float oldtonew = testrad - radius;
            center.X = (radius * center.X + oldtonew * Position[i].X) / testrad;
            center.Y = (radius * center.Y + oldtonew * Position[i].Y) / testrad;
            center.Z = (radius * center.Z + oldtonew * Position[i].Z) / testrad;
        }
    }

    Center = center;
    Radius = radius;
}

inline void SphereClass::Init(const Vector3 &pos, float radius)
{
    Center = pos;
    Radius = radius;
}

inline void SphereClass::Re_Center(const Vector3 &center)
{
    float dist = (Center - center).Length();
    Center = center;
    Radius += dist;
}

inline void SphereClass::Add_Sphere(const SphereClass &s)
{
    if (s.Radius == 0.0f) {
        return;
    }

    float dist = (s.Center - Center).Length();

    if (dist == 0.0f) {
        Radius = (Radius > s.Radius) ? Radius : s.Radius;

        return;
    }

    float rnew = (dist + Radius + s.Radius) / 2.0f;

    if (rnew < Radius) {
    } else {
        if (rnew < s.Radius) {
            Init(s.Center, s.Radius);
        } else {
            float lerp = (rnew - Radius) / dist;
            Vector3 center = (s.Center - Center) * lerp + Center;
            Init(center, rnew);
        }
    }
}

inline void SphereClass::Transform(const Matrix3D &tm)
{
    Center = tm * Center;
}

inline float SphereClass::Volume() const
{
    return (4.0f / 3.0f) * GAMEMATH_PI * (Radius * Radius * Radius);
}

inline SphereClass &SphereClass::operator+=(const SphereClass &s)
{
    Add_Sphere(s);

    return *this;
}

inline SphereClass &SphereClass::operator*=(const Matrix3D &m)
{
    Init(m * Center, Radius);

    return *this;
}

inline bool Spheres_Intersect(const SphereClass &s0, const SphereClass &s1)
{
    Vector3 delta = s0.Center - s1.Center;
    float dist2 = delta * delta;

    if (dist2 < (s0.Radius + s1.Radius) * (s0.Radius + s1.Radius)) {
        return true;
    } else {
        return false;
    }
}

inline SphereClass Add_Spheres(const SphereClass &s0, const SphereClass &s1)
{
    if (s0.Radius == 0.0f) {
        return s1;
    } else {
        SphereClass result(s0);
        result.Add_Sphere(s1);

        return result;
    }
}

inline SphereClass operator+(const SphereClass &s0, const SphereClass &s1)
{
    return Add_Spheres(s0, s1);
}

inline SphereClass Transform_Sphere(const Matrix3D &m, const SphereClass &s)
{
    return SphereClass(m * s.Center, s.Radius);
}

inline void Transform_Sphere(const Matrix3D &m, const SphereClass &s, SphereClass &res)
{
    res.Center = m * s.Center;
    res.Radius = s.Radius;
}

inline SphereClass operator*(const Matrix3D &m, const SphereClass &s)
{
    return Transform_Sphere(m, s);
}